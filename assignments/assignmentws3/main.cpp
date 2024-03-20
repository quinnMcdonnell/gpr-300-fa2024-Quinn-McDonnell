#include <stdio.h>
#include <math.h>

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

float cutoff = 0.0f;

ew::Camera camera;
ew::CameraController cameraController;

struct {
	GLuint vao;
	GLuint vbo;

}display;

void create_transition_pass()
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

GLuint g1;
GLuint g2;
GLuint g3;
GLuint current;

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);

	//Shader and Models
	ew::Shader transition = ew::Shader("assets/transition.vert", "assets/transition.frag");
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");

	g1 = ew::loadTexture("assets/transitions/gradient1.png");
	g2 = ew::loadTexture("assets/transitions/gradient2.png");
	g3 = ew::loadTexture("assets/transitions/gradient3.png");
	current = g1;


	ew::Transform planeTransform;
	planeTransform.position = glm::vec3(0.0f, -2.0f, 0.0f);

	//Model Tranform
	ew::Transform monkeyTransform;

	//Create Quad
	create_transition_pass();


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

		//RENDER
		glClearColor(0.6f,0.8f,0.92f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, current);
		glBindTextureUnit(0, current);
		
		transition.use();
		transition.setFloat("cutoff", cutoff);
		transition.setVec3("color", 0.94f, 0.76f, 0.05f);
		transition.setInt("gradient", 0);

		glBindVertexArray(display.vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}


void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	
	ImGui::SliderFloat("Cutoff", &cutoff, 0.0f, 1.0f, "%.2f");

	if(ImGui::Button("gradient1"))
	{
		current = g1;
	}

	if (ImGui::Button("gradient2"))
	{
		current = g2;
	}

	if (ImGui::Button("gradient3"))
	{
		current = g3;
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

