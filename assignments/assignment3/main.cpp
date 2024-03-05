#include <stdio.h>
#include <math.h>
#include <iostream>

#include <ew/external/glad.h>

#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>
#include <ew/procGen.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();
void resetCamera();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
bool reset = false;

ew::Camera camera;
ew::CameraController cameraController;

float prevFrameTime;
float deltaTime;
int speed = 1;

glm::vec3 rotation = glm::vec3(0.0, 1.0, 0.0);

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

struct {
	GLuint fbo;
	GLuint world_position;
	GLuint world_normal;
	GLuint albedo;
	GLuint depth;
} deferred;

struct {
	GLuint vao;
	GLuint vbo;

}display;

struct PointLight {
	glm::vec3 position;
	float radius;
	glm::vec3 color;
};

const int MAX_LIGHT_POINTS = 64;
PointLight pointLights[MAX_LIGHT_POINTS];


GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

static void create_deferred_pass(void)
{
	glGenFramebuffers(1, &deferred.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, deferred.fbo);

	//World Position
	glGenTextures(1, &deferred.world_position);
	glBindTexture(GL_TEXTURE_2D, deferred.world_position);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//World Normal
	glGenTextures(1, &deferred.world_normal);
	glBindTexture(GL_TEXTURE_2D, deferred.world_normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//World Albedo
	glGenTextures(1, &deferred.albedo);
	glBindTexture(GL_TEXTURE_2D, deferred.albedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//World Depth
	glGenTextures(1, &deferred.depth);
	glBindTexture(GL_TEXTURE_2D, deferred.depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, deferred.world_position, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, deferred.world_normal, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, deferred.albedo, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, deferred.depth, 0);

	GLenum buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, buffers);

	glCheckError();

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete \n");
	}

	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void create_display_pass()
{
	float quad[] = {

		//Vertices				UV Coords

		//Triangle 1
		-1.0f, +1.0f, 0.0f,		0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
		+1.0f, +1.0f, 0.0f,		1.0f, 1.0f,

		//triangle 2
		+1.0f, -1.0f, 0.0f,		1.0f, 0.0f,
		+1.0f, +1.0f, 0.0f,		1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
	};

	glGenVertexArrays(1, &display.vao);
	glGenBuffers(1, &display.vbo);

	glBindVertexArray(display.vao);
	glBindBuffer(GL_ARRAY_BUFFER, display.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);
}

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);

	//Shader and Models
	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader gpShader = ew::Shader("assets/geometryPass.vert", "assets/geometryPass.frag");
	ew::Shader lightingPassShader = ew::Shader("assets/deferredLit.vert", "assets/deferredLit.frag");
	ew::Shader lightOrbShader = ew::Shader("assets/orb.vert", "assets/orb.frag");

	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	ew::Mesh planeMesh = ew::Mesh(ew::createPlane(100, 100, 100));
	ew::Mesh sphereMesh = ew::Mesh(ew::createSphere(1.0f, 8));

	//Model Tranform
	ew::Transform monkeyTransform[8][8];
	ew::Transform planeTransform;

	planeTransform.position = glm::vec3(6.0f, -1.5f, 10.0f);

	for (int row = 0; row < 8; row++)
	{
		for (int col = 0; col < 8; col++)
		{
			monkeyTransform[row][col].position = glm::vec3(row * 8, 0, col * 8);
		}
	}

	create_deferred_pass();
	create_display_pass();

	//Camera and its Controller
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	//point lights
	srand((unsigned)time(NULL));

	//for (int i = 0; i < MAX_LIGHT_POINTS; i++)
	//{
	//	int x = -10 + (rand() % 20);
	//	int y = (rand() % 20);
	//	int z = -10 + (rand() % 20);
	//	pointLights[i].position = glm::vec3(x, y, z);
	//	pointLights[i].radius = 1 + (rand() % 100);
	//	pointLights[i].color = glm::vec3(rand() % 1, rand() % 1, rand() % 1);

	srand(time(0));

		for (int row = 0; row < 8; row++)
		{
			for (int col = 0; col < 8; col++)
			{
				auto i = (row * 8 + col);

				float r = (float)(rand() % 100) / 100;
				float g = (float)(rand() % 100) / 100;
				float b = (float)(rand() % 100) / 100;

				pointLights[i].position = glm::vec3(rand() % 50, rand() % 3 , rand() % 50);
				pointLights[i].radius = rand() % 10 + 2;
				pointLights[i].color = glm::vec3(r,g,b);
			}
		}
	//}


	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;


		cameraController.move(window, &camera, deltaTime);

		//RENDER
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, deferred.fbo);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, brickTexture);
		glBindTextureUnit(0, brickTexture);

		//geometry pass >>>
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		gpShader.use();
		gpShader.setInt("_MainTex", 0);
		gpShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());

		for (int row = 0; row < 8; row++)
		{
			for (int col = 0; col < 8; col++)
			{
				gpShader.setMat4("_Model", monkeyTransform[row][col].modelMatrix());
				monkeyModel.draw();
			}
		}
		
		gpShader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//geometry pass <<<

		//lighting pass >>>
		lightingPassShader.use();
		glBindVertexArray(display.vao);
		glDisable(GL_DEPTH_TEST);
		glBindTextureUnit(0, deferred.world_position);
		glBindTextureUnit(1, deferred.world_normal);
		glBindTextureUnit(2, deferred.albedo);
		glBindTextureUnit(3, deferred.depth);

		lightingPassShader.setFloat("_Material.Ka", material.Ka);
		lightingPassShader.setFloat("_Material.Kd", material.Kd);
		lightingPassShader.setFloat("_Material.Ks", material.Ks);
		lightingPassShader.setFloat("_Material.Shininess", material.Shininess);
		lightingPassShader.setVec3("_EyePos", camera.position);

		for (int i = 0; i < MAX_LIGHT_POINTS; i++)
		{
			std::string prefix = "_PointLights[" + std::to_string(i) + "].";
			lightingPassShader.setVec3(prefix + "position", pointLights[i].position);
			lightingPassShader.setFloat(prefix + "radius", pointLights[i].radius);
			lightingPassShader.setVec3(prefix + "color", pointLights[i].color);
		}

		lightingPassShader.setInt("gPosition", 0);
		lightingPassShader.setInt("gNormal", 1);
		lightingPassShader.setInt("gAlbedo", 2);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);
		//light pass <<<


		//drawing orbs to screen >>>
		glBindFramebuffer(GL_READ_FRAMEBUFFER, deferred.fbo); //Read from gBuffer 
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, deferred.fbo); //Write to current fbo
		glBlitFramebuffer(
			0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST
		);

		//Draw all light orbs
		lightOrbShader.use();
		lightOrbShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		for (int i = 0; i < MAX_LIGHT_POINTS; i++)
		{
			glm::mat4 m = glm::mat4(1.0f);
			m = glm::translate(m, pointLights[i].position);
			m = glm::scale(m, glm::vec3(0.2f)); //Whatever radius you want

			lightOrbShader.setMat4("_Model", m);
			lightOrbShader.setVec3("_Color", pointLights[i].color);
			sphereMesh.draw();
		}

		//drawing orbs to screen <<<

		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void resetCamera()
{
	camera.position = glm::vec3(0, 0, 5.0f);
	camera.target = glm::vec3(0);
	cameraController.yaw = cameraController.pitch = 0;
}

void resetRotation()
{
	speed = 1;
	rotation = glm::vec3(0.0, 1.0, 0.0);
	reset = true;
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	if (ImGui::Button("Reset Camera"))
	{
		resetCamera();
	}

	if(ImGui::CollapsingHeader("Material"))
	{
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);

		if (ImGui::Button("Reset Material"))
		{
			material.Ka = 1;
			material.Kd = 0.5;
			material.Ks = 0.5;
			material.Shininess = 128;
		}
	}
	ImGui::End();

	//GBUFFER
	ImGui::Begin("World Position");
	//Using a Child allow to fill all the space of the window.
	ImGui::BeginChild("World Position");
	//Stretch image to be window size
	ImVec2 windowSize = ImGui::GetWindowSize();
	//Invert 0-1 V to flip vertically for ImGui display
	//shadowMap is the texture2D handle
	ImGui::Image((ImTextureID)deferred.world_position, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::EndChild();
	ImGui::End();
	
	ImGui::Begin("World Normal");
	//Using a Child allow to fill all the space of the window.
	ImGui::BeginChild("World Normal");
	//Stretch image to be window size
	ImVec2 windowSize1 = ImGui::GetWindowSize();
	//Invert 0-1 V to flip vertically for ImGui display
	//shadowMap is the texture2D handle
	ImGui::Image((ImTextureID)deferred.world_normal, windowSize1, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::EndChild();
	ImGui::End();

	ImGui::Begin("Albedo");
	//Using a Child allow to fill all the space of the window.
	ImGui::BeginChild("Albedo");
	//Stretch image to be window size
	ImVec2 windowSize2 = ImGui::GetWindowSize();
	//Invert 0-1 V to flip vertically for ImGui display
	//shadowMap is the texture2D handle
	ImGui::Image((ImTextureID)deferred.albedo, windowSize2, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::EndChild();
	ImGui::End();

	ImGui::Begin("Depth");
	//Using a Child allow to fill all the space of the window.
	ImGui::BeginChild("Depth");
	//Stretch image to be window size
	ImVec2 windowSize3 = ImGui::GetWindowSize();
	//Invert 0-1 V to flip vertically for ImGui display
	//shadowMap is the texture2D handle
	ImGui::Image((ImTextureID)deferred.depth, windowSize3, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::EndChild();
	ImGui::End();
	

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

