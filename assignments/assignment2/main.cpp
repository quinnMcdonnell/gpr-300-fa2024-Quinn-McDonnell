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

glm::vec3 pos = glm::vec3(0.0f, 50.0f, 0.0f);
glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 up = glm::vec3(0.0f, 0.0f, -1.0f);

struct
{
	GLuint fbo;
	GLuint map;
	
} shadow;

struct
{
	GLuint vao;
	GLuint vbo;
} debug;

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

//Frame Buffer 
unsigned int fbo;
unsigned int rbo;
unsigned int colorBuffer;

auto ambient_color = glm::vec3(0.6f, 0.8f, 0.92f);

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

static void create_debug_pass(void)
{
	float dbg_vertices[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };
	
	glGenVertexArrays(1, &debug.vao);
	glGenBuffers(1, &debug.vbo);

	glBindVertexArray(debug.vao);
	glBindBuffer(GL_ARRAY_BUFFER, debug.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(dbg_vertices), &dbg_vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
}

static void create_shadow_pass(void)
{
	glGenFramebuffers(1, &shadow.fbo);

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	glGenTextures(1, &shadow.map);
	glBindTexture(GL_TEXTURE_2D, shadow.map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, shadow.fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow.map, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glCheckError();

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete \n");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float quad[] = {

	//Vertices				UV Coords

	//Triangle 1
	-1.0f, +1.0f, 0.0f,		0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
	+1.0f, +1.0f, 0.0f,		1.0f, 1.0f,

	//trinagle 2
	+1.0f, -1.0f, 0.0f,		1.0f, 0.0f,
	+1.0f, +1.0f, 0.0f,		1.0f, 1.0f,
	-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
};

unsigned int screenVBO;
unsigned int screenVAO;

bool edge = false, inverted = false, box = false, grayscale = false;
float blurring = 9.0;

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);

	//Shader and Models
	ew::Shader postProcessShader = ew::Shader("assets/post.vert", "assets/post.frag");
	ew::Shader Inverted = ew::Shader("assets/inverted.vert", "assets/inverted.frag");
	ew::Shader Grayscale = ew::Shader("assets/grayscale.vert", "assets/grayscale.frag");
	ew::Shader Box = ew::Shader("assets/box.vert", "assets/box.frag");
	ew::Shader Edge = ew::Shader("assets/edge.vert", "assets/edge.frag");

	//THESE ARE THE THREE FOR ASSIGNMENT
	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");

	ew::Shader model_shader = ew::Shader("assets/model.vert", "assets/model.frag");
	ew::Shader shadow_shader = ew::Shader("assets/shadow.vert", "assets/shadow.frag");
	ew::Shader debug_shader = ew::Shader("assets/debug.vert", "assets/debug.frag");

	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");
	ew::Mesh planeMesh = ew::Mesh(ew::createPlane(10, 10, 5));
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	create_debug_pass();
	create_shadow_pass();

	//Model Tranform
	ew::Transform monkeyTransform;
	ew::Transform planeTransform;

	planeTransform.position = glm::vec3(0.0f, -2.0f, 0.0f);

	//Camera and its Controller
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	//State machine
	//glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST); //Depth testing
	//glCullFace(GL_BACK); //Back face culling
	//glViewport(0, 0, screenWidth, screenHeight);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//Light variables
		float near_plane = 0.0f, far_plane = 100.0f;
		glm::mat4 lightProjection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(
			pos,
			target,
			up
		);
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		cameraController.move(window, &camera, deltaTime);

		// >>> RENDER FROM LIGHTS POV
		glBindFramebuffer(GL_FRAMEBUFFER, shadow.fbo);
		
		// pass
		glViewport(0, 0, 1024, 1024);
		glClear(GL_DEPTH_BUFFER_BIT);

		// pipeline
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST); //Depth testing
		glCullFace(GL_BACK); //Back face culling
		//glViewport(0, 0, 1024, 1024);
		

		// bind
		shadow_shader.use();

		shadow_shader.setMat4("_ViewProjection", lightSpaceMatrix);

		shadow_shader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();

		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime * speed, rotation);
		shadow_shader.setMat4("_Model", monkeyTransform.modelMatrix());
		
		monkeyModel.draw();
		

		glBindFramebuffer(GL_FRAMEBUFFER, 0); //HOW TO UNBIND
		// <<< RENDER FROM LIGHTS POV

		// >>> RENDER THE SCENE NORMALLY
 
		//pass
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//pipeline
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST); //Depth testing
		glCullFace(GL_BACK); //Back face culling
		glViewport(0, 0, screenWidth, screenHeight);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, brickTexture);
		glBindTextureUnit(0, brickTexture);
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shadow.map);
		glBindTextureUnit(1, shadow.map);

		shader.use();
		
		shader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();

		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		shader.setMat4("_LightViewProj", lightView * lightProjection);
		
		

		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);

		

		shader.setVec3("lightPos", pos);
		shader.setVec3("viewPos", camera.position);		
		
		

		shader.setInt("_MainTex", 0);
		shader.setInt("_ShadowMap", 1);


		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		monkeyModel.draw();

		
		
		// <<< RENDER THE SCENE NORMALLY

		// >>>  RENDER THE DEBUG QUAD
		glViewport(screenWidth - 150, 0, 150, 150);
		glBindVertexArray(debug.vao);
		debug_shader.use();
		debug_shader.setInt("debug_image", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shadow.map);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		// <<<  RENDER THE DEBUG QUAD





		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT);
		

		//if (edge) 
		//{
		//	Edge.use();
		//}else if (box)
		//{
		//	Box.use();
		//	Box.setFloat("DivideValue", blurring);
		//} else if (inverted)
		//{
		//	Inverted.use();
		//} else if (grayscale)
		//{
		//	Grayscale.use();
		//}
		//else {
		//	postProcessShader.use();
		//}

		//glBindVertexArray(screenVAO);
		//glDisable(GL_DEPTH_TEST);
		//glBindTexture(GL_TEXTURE_2D, colorBuffer);
		//
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		//
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

	if (ImGui::CollapsingHeader("Light/Shadow Controller"))
	{
		ImGui::SliderFloat3("Position", &pos.x, -50.0f, 50.0f);
		ImGui::SliderFloat3("Target", &target.x, -50.0f, 50.0f);
		ImGui::SliderFloat3("Up", &up.x, -50.0f, 50.0f);

		if (ImGui::Button("Reset Light"))
		{
			pos = glm::vec3(0.0f, 50.0f, 0.0f);
			target = glm::vec3(0.0f, 0.0f, 0.0f);
			up = glm::vec3(0.0f, 0.0f, -1.0f);
		}
	}

	//Change Direction Stuff
	if (ImGui::CollapsingHeader("Rotation"))
	{
		ImGui::SliderFloat3("Rotation Direction", &rotation.x,-1.0f,1.0f);
		ImGui::SliderInt("Rotation Speed", &speed,1.0f,20.0f);
		if (ImGui::Button("Reset Rotation"))
		{
			resetRotation();
		}
	}

	if (ImGui::CollapsingHeader("Post Processing"))
	{
		ImGui::Checkbox("Inverted", &inverted);
		ImGui::Checkbox("Grayscale", &grayscale);
		ImGui::Checkbox("Box Blur", &box);
		ImGui::SliderFloat("Box Blur Brightness", &blurring, 1, 18);
		ImGui::Checkbox("Edge Detect", &edge);
	}

	ImGui::End();

	ImGui::Begin("Shadow Map");
	//Using a Child allow to fill all the space of the window.
	ImGui::BeginChild("Shadow Map");
	//Stretch image to be window size
	ImVec2 windowSize = ImGui::GetWindowSize();
	//Invert 0-1 V to flip vertically for ImGui display
	//shadowMap is the texture2D handle
	ImGui::Image((ImTextureID)shadow.map, windowSize, ImVec2(0, 1), ImVec2(1, 0));
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

