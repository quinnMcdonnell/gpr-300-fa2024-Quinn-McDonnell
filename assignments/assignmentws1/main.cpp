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
	GLuint albedo;
	GLuint metallic;
	GLuint roughness;
	GLuint occulusion;
	GLuint specular;
}PBRMaterial;

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);

	//Shader and Models
	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader PBRshader = ew::Shader("assets/pbr.vert", "assets/pbr.frag");

	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");

	ew::Model shellModel = ew::Model("assets/togezoshell.obj");

	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");
	
	PBRMaterial.albedo = ew::loadTexture("assets/togezoshell_col.png");
	PBRMaterial.metallic = ew::loadTexture("assets/togezoshell_mtl.png");
	PBRMaterial.roughness = ew::loadTexture("assets/togezoshell_rgh.png");
	PBRMaterial.occulusion = ew::loadTexture("assets/togezoshell_ao.png");
	PBRMaterial.specular = ew::loadTexture("assets/togezoshell_spc.png");


	//Model Tranform
	ew::Transform monkeyTransform;
	ew::Transform shellTransform;

	//Camera and its Controller
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		glClearColor(0.6f,0.8f,0.92f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cameraController.move(window, &camera, deltaTime);

		PBRshader.use();
		PBRshader.setMat4("_Model", shellTransform.modelMatrix());
		PBRshader.setMat4("_ViewProjection", camera.projectionMatrix());

		PBRshader.setVec3("cameraPos", camera.position);
		PBRshader.setVec3("lightPos", glm::vec3(0, 10, 0));
		PBRshader.setVec3("ambientLight", glm::vec3(0, 10, 0));
		PBRshader.setInt("material", PBRMaterial.albedo);
	
		shellModel.draw();

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

