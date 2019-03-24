#include <iostream>
#include <thread>

// Window/OpenGL Functionality
// GLAD - https://github.com/Dav1dde/glad
#include "glad/glad.h"
// GLFW3 - https://github.com/glfw/glfw
#include <GLFW/glfw3.h>

// GUI - https://github.com/ocornut/imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "model.h"
#include "shader.h"

// Prototypes
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, float deltaTime);
void cursor_callback(GLFWwindow* window, double xpos, double ypos);
void physics(Model &ball, Model floor, float timestep);

// Misc Variables
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

struct Camera
{
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 orientation = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
};

// Camera variables
Camera camera;
float last_x = SCR_WIDTH / 2;
float last_y = SCR_HEIGHT / 2;
float yaw = -90.0f;
float pitch = 0.0f;

// Simulation Variables
bool isFocused = true;
bool isRunning = true;

float set_pos[3];
float set_vel[3];
glm::vec3 gravity(0, -0.0098, 0 );
float restitution = 1.0;

int fps = 60;
int physics_tick = 60;


int main(int argc, char *argv[])
{
	// Create window with an OpenGL context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assessment 2", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Window Initialistion Failed." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	// Setup Cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, cursor_callback);

	// Load OpenGl Functions
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// OpenGL Settings
	glEnable(GL_DEPTH_TEST);


	// Load Ball
	Model ball("Models/ball.obj");
	Shader shader("Shaders/VertexShader", "Shaders/BasicFragShader");

	// Move ball away from camera a bit
	ball.move(glm::vec3(0, 0, -4));

	// Load Floor
	Model floor("Models/floor.obj");

	// Move floor down and away
	floor.move(glm::vec3(0, -4, -4));

	// Setup matrices
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view;
	view = glm::lookAt(camera.position, camera.position + camera.front, camera.orientation);

	// Initialise GUI Lib
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Load GLFW functionality
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");
	ImGui::StyleColorsDark();

	float delta_time = 0.0f;
	float last_frame = 0.0f;
	float current_frame;

	float frame_time = 1 / (float)fps;
	float physics_time = 1 / (float)physics_tick;
	float time_buffer = 0.0;

	// Render Loop
	while (!glfwWindowShouldClose(window))
	{
		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		
		// FPS Control
		frame_time = 1 / (float)fps;
		if (delta_time < frame_time)
		{
			float new_time = frame_time - delta_time;
			delta_time = frame_time;
			Sleep(new_time * 1000);
		}

		time_buffer += delta_time;
		while(time_buffer >= physics_time) // Phyiscs updates at own rate
		{
			physics(ball, floor, physics_time);
			time_buffer -= physics_time;
		}
		
		processInput(window, delta_time);


		// Update view position
		view = glm::lookAt(camera.position, camera.position + camera.front, camera.orientation);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);

		// Pass uniform
		shader.setVec3("colour", glm::vec3(1.0, 0.0, 0.0));
		shader.setVec3("lightPosition", camera.position);

		// Draw Ball
		shader.setMat4("model", ball.position);
		glBindVertexArray(ball.vao);
		glDrawArrays(GL_TRIANGLES, 0, ball.vertex.size());
		glBindVertexArray(0);

		// Draw Floor
		shader.setMat4("model", floor.position);
		shader.setVec3("colour", glm::vec3(0.0, 1.0, 0.0));
		glBindVertexArray(floor.vao);
		glDrawArrays(GL_TRIANGLES, 0, floor.vertex.size());
		glBindVertexArray(0);

		// Draw GUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Window
		ImGui::Begin("Simulation Controls");
		ImGui::SetNextWindowSize(ImVec2(100, 100));

		// Widgets
		ImGui::InputFloat3("Position", set_pos);
		ImGui::InputFloat3("Velocity", set_vel);
		if (ImGui::Button("Set"))
			ball.setState(set_pos, set_vel);

		ImGui::SliderFloat("Restitution", &restitution, 0.0, 1.0);
		ImGui::SliderFloat("Gravity", &gravity.y, 0.0, -0.01);
		ImGui::SliderInt("FPS", &fps, 1, 59);

		if (isRunning)
		{
			if (ImGui::Button("Pause"))
				isRunning = false;
		}
		else
		{
			if (ImGui::Button("Play"))
				isRunning = true;
		}

		if(ImGui::Button("Close"))
			glfwSetWindowShouldClose(window, true);

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap framebuffer, poll inputs
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	// Destroy GUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return 0;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void physics(Model &ball, Model floor, float timestep)
{
	if (isRunning)
	{
		// Gravity
		glm::vec3 g(gravity * timestep);
		ball.velocity = ball.velocity + (gravity * timestep); // Add gravity to balls velocity
		ball.move(ball.velocity); // Apply velocity to ball

		std::cout << "\n\t== Gravity ==\n";
		std::cout << "FPS: " << fps << "    |    Timestep: " << timestep << "    |    Change in y: " << g.y  << ")\n\t==============";

		// Collision
		glm::vec3 dif = ball.pos - floor.pos; // Distance between ball centre and floor

		//Rebound
		if(dif.y < ball.rad & ball.velocity.y < 0) // If distance to floor less than radius and object is movign towards it
		{
			ball.velocity.y = -ball.velocity.y * restitution;

			if (ball.velocity.y > 0.001) // Debug info
			{
				std::cout << "\n\tCollision" << std::endl;
				std::cout << "Radius: " << ball.rad << "    |    Dist to Floor: " << dif.y << std::endl;
				std::cout << "Restitution: " << restitution << "   |   Velocity: " << ball.velocity.y << std::endl;
			}
		}
	}
}

void processInput(GLFWwindow *window, float deltaTime)
{
	// Use time to find how fast mouse was moved
	float speed = 2.5f * deltaTime;

	// Camera
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.position += speed * camera.front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.position -= speed * camera.front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.position -= glm::normalize(glm::cross(camera.front, camera.orientation)) * speed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.position += glm::normalize(glm::cross(camera.front, camera.orientation)) * speed;

	// Other
	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) // Focus window
	{
		if(isFocused)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		isFocused = !isFocused;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) // Toggle simulation
		isRunning = !isRunning;
}

// Called when mouse (cursor) moves, used to move camera
void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (!isFocused)
		return;

	float xoffset = xpos - last_x;
	float yoffset = last_y - ypos;

	// Set last position to current position
	last_x = xpos;
	last_y = ypos;

	// Adjust based on sensitivity
	float sensitivity = 0.05;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// Bind camera to front 180 angle on y axis (pitch)
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

	camera.front = glm::normalize(front);
}
