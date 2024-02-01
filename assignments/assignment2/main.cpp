#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>

#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>

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

struct
{
	GLuint vao;
	GLuint vbo;
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

char* effects[] = { "Normal", "Inverted", };

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);

	//Shader and Models
	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader postProcessShader = ew::Shader("assets/post.vert", "assets/post.frag");
	ew::Shader Inverted = ew::Shader("assets/inverted.vert", "assets/inverted.frag");
	ew::Shader Grayscale = ew::Shader("assets/grayscale.vert", "assets/grayscale.frag");
	ew::Shader Box = ew::Shader("assets/box.vert", "assets/box.frag");
	ew::Shader Edge = ew::Shader("assets/edge.vert", "assets/edge.frag");

	ew::Shader shadow_shader = ew::Shader("assets/shadow.vert", "assets/shadow.frag");
	ew::Shader debug_shader = ew::Shader("assets/debug.vert", "assets/debug.frag");

	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	create_debug_pass();

	//Model Tranform
	ew::Transform monkeyTransform;

	//Camera and its Controller
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	//DEPTH BUFFER
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//framebuffer
	glGenFramebuffers(1, &shadow.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow.fbo);

	glGenTextures(1, &shadow.map);
	glBindTexture(GL_TEXTURE_2D, shadow.map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glCheckFramebufferStatus(GL_FRAMEBUFFER);


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete \n");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//State machine
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing

	//Screen Quad
	glGenVertexArrays(1, &screenVAO);
	glGenBuffers(1, &screenVBO);

	glBindVertexArray(screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	
	//RBO
	//glGenRenderbuffers(1, &rbo);
	//glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//Light variables
		auto light_pos = glm::vec3(0.0f, 10.0f, 0.0f);
		auto light_view = glm::lookAt(light_pos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		auto light_proj = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.0f, 100.0f);

		//Reset the model after a change in rotation
		if (reset)
		{
			monkeyTransform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			reset = false;
		}

		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime * speed, rotation);

		cameraController.move(window, &camera, deltaTime);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		//RENDER FROM LIGHTS POV
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);

		shadow_shader.use();
		shadow_shader.setMat4("_Model", monkeyTransform.modelMatrix());
		shadow_shader.setMat4("_ViewProjection", light_view * light_proj);
		monkeyModel.draw();
		////////////////////////////

		//glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT);

		//RENDER THE SCENE NORMALLY
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, brickTexture);
		glBindTextureUnit(0, brickTexture);

		shader.use();
		shader.setInt("_MainTex", 0);
		shader.setVec3("_EyePos", camera.position);

		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());

		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);

		monkeyModel.draw();
		//////////////////////////

		// RENDER THE DEBUG QUAD
		glBindVertexArray(debug.vao);
		glViewport(0, 0, screenWidth, screenHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		debug_shader.use();

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
		////////////////////////////





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

