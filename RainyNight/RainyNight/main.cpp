// Std. Includes
#include <string>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

// GLM Mathemtics
#define M_PI 3.14159265358979323846f 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Effects
#include "RainSystem.h"
#include "AnimatedWaterSystem.h"

// Other Libs
#include <SOIL.h>
#include <learnopengl/filesystem.h>

// Properties
GLuint screenWidth = 1280, screenHeight = 720;

#pragma region Function Declarations
void keyHandler(GLFWwindow* window, int key, int scancode, int action, int mode);
#pragma region Camera
void doScriptedCameraMovement(GLfloat currentFrame);
void moveCameraTo(glm::vec3 position);
void lookCameraHorizontal(float angle);
void lookCameraVertical(float angle);
void cameraRide(float currentTime, float startTime, glm::vec3 startPosition, float startRotationHorizontal, float startRotationVertical, float endTime, glm::vec3 endPosition, float endRotationHorizontal, float endRotationVertical);
void updateCamera();
#pragma endregion
#pragma region Shader
Shader loadShader(string filename);
#pragma endregion
#pragma region Models
Model loadModel(string path);
void drawModel(Model model, Shader shader, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
#pragma endregion
#pragma endregion

// Camera
Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
glm::mat4 projectionMatrix = glm::mat4();
glm::mat4 viewMatrix = glm::mat4();

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;


int main()
{
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Rainy Night", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);

	// Set callback functions
	glfwSetKeyCallback(window, keyHandler);

	// Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define viewport
	glViewport(0, 0, screenWidth, screenHeight);

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);

	// Setup and compile shaders
	Shader shaderSimpleGreen = loadShader("simple_green");
	Shader shaderSimpleBlue = loadShader("simple_blue");
	Shader shaderTexture = loadShader("texture");
	Shader shaderRain = loadShader("rain");

	// Load models
	Model modelHouse = loadModel("resources/objects/house/Farmhouse.obj");
	Model modelRock = loadModel("resources/objects/rock/rock.obj");
	Model modelTerrain = loadModel("resources/terrain/terrain.fbx");

	// Initialize RainSystem
	RainSystem *rainSystem = new RainSystem();

	// Initialize AnimatedWater
	AnimatedWaterSystem *animatedWaterSystem = new AnimatedWaterSystem();

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// FPS
		float fps = 1.0f / deltaTime;
		//cout << fps << " FPS" << endl;

		// Check and call events
		glfwPollEvents();

		// Clear the colorbuffer
		glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Camera
		doScriptedCameraMovement(currentFrame);
		projectionMatrix = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		viewMatrix = camera.GetViewMatrix();
	
		// Draw Models
		shaderSimpleGreen.Use();
		drawModel(modelTerrain, shaderSimpleGreen, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(20.0f, 20.0f, 20.0f));
		shaderTexture.Use();
		drawModel(modelHouse, shaderTexture, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.05f, 0.05f, 0.05f));
		drawModel(modelRock, shaderTexture, glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.05f, 0.05f, 0.05f));
		drawModel(modelRock, shaderTexture, glm::vec3(1.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.06f, 0.06f, 0.06f));

		// AnimatedWaterSystem
		shaderSimpleBlue.Use();
		animatedWaterSystem->drawWater(shaderSimpleBlue, projectionMatrix, viewMatrix);

		// RainSystem
		shaderRain.Use();
		rainSystem->updateParticles(deltaTime);
		rainSystem->drawParticles(shaderSimpleBlue, projectionMatrix, viewMatrix);

		// Swap the buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

#pragma region Camera Management
// Scripted Camera Movement
void doScriptedCameraMovement(GLfloat currentFrame) {
	//cameraRide(currentFrame, 5.0f, glm::vec3(0.0f, 0.0f, 5.0f), 0.0f, 0.0f, 10.0f, glm::vec3(5.0f, 2.0f, 4.0f), -20.0f, -20.0f);
	//cameraRide(currentFrame, 12.0f, glm::vec3(5.0f, 2.0f, 4.0f), -20.0f, -20.0f, 20.0f, glm::vec3(0.0f, 0.0f, 5.0f), 0.0f, 0.0f);
	cameraRide(currentFrame, 5.0f, glm::vec3(0.0f, 2.0f, 10.0f), 0.0f, 0.0f, 6.0f, glm::vec3(0.0f, 2.0f, 10.0f), 0.0f, 90.0f);
	cameraRide(currentFrame, 6.0f, glm::vec3(0.0f, 2.0f, 10.0f), 0.0f, 90.0f, 8.0f, glm::vec3(0.0f, 2.0f, 10.0f), 0.0f, -90.0f);
	cameraRide(currentFrame, 8.0f, glm::vec3(0.0f, 2.0f, 10.0f), 0.0f, -90.0f, 9.0f, glm::vec3(0.0f, 2.0f, 10.0f), 0.0f, 0.0f);
	cameraRide(currentFrame, 9.0f, glm::vec3(0.0f, 2.0f, 10.0f), 0.0f, 0.0f, 13.0f, glm::vec3(0.0f, 2.0f, 10.0f), 360.0f, 0.0f);
}

// Moves the camera to a certain point in the scene
void moveCameraTo(glm::vec3 position) {
	camera.Position = position;
}

// Sets the horizontal rotation of the camera (left-angle < 0 < right-angle)
void lookCameraHorizontal(float angle) {
	camera.Yaw = -90.0f + angle;
}

// Sets the vertical rotation of the camera (down-angle < 0 < up-angle)
void lookCameraVertical(float angle) {
	camera.Pitch = angle;
}

// Performs a camera ride
void cameraRide(float currentTime, float startTime, glm::vec3 startPosition, float startRotationHorizontal, float startRotationVertical, float endTime, glm::vec3 endPosition, float endRotationHorizontal, float endRotationVertical) {
	if (currentTime >= startTime && currentTime <= endTime) {
		float currentAnimationTime = currentTime - startTime;

		// position
		glm::vec3 animationPosition = glm::vec3();
		animationPosition.x = (endPosition.x - startPosition.x) / (endTime - startTime) * currentAnimationTime + startPosition.x;
		animationPosition.y = (endPosition.y - startPosition.y) / (endTime - startTime) * currentAnimationTime + startPosition.y;
		animationPosition.z = (endPosition.z - startPosition.z) / (endTime - startTime) * currentAnimationTime + startPosition.z;
		moveCameraTo(animationPosition);

		// rotation horizontal
		float animationRotationHorizontal = (endRotationHorizontal - startRotationHorizontal) / (endTime - startTime) * currentAnimationTime + startRotationHorizontal;
		lookCameraHorizontal(animationRotationHorizontal);

		// rotation vertical
		float animationRotationVertical = (endRotationVertical - startRotationVertical) / (endTime - startTime) * currentAnimationTime + startRotationVertical;
		lookCameraVertical(animationRotationVertical);

		// update
		updateCamera();
	}
}

// Updates the camera with the new set parameters
void updateCamera() {
	camera.updateCameraVectors();
}
#pragma endregion

#pragma region Shader Management
// Loads Shader
Shader loadShader(string filename) {
	Shader shader(FileSystem::getPath("resources/shader/" + filename + ".vs").c_str(), FileSystem::getPath("resources/shader/" + filename + ".frag").c_str());
	return shader;
}
#pragma endregion

#pragma region Model Management
// Loads model
Model loadModel(string path) {
	Model model(FileSystem::getPath(path).c_str());
	return model;
}

// Draws model
void drawModel(Model model, Shader shader, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glm::mat4 location = glm::mat4();
	location = glm::translate(location, position);
	location = glm::scale(location, scale);
	location = glm::rotate(location, rotation.x * M_PI / 180.0f, glm::vec3(1, 0, 0));
	location = glm::rotate(location, rotation.y * M_PI / 180.0f, glm::vec3(0, 1, 0));
	location = glm::rotate(location, rotation.z * M_PI / 180.0f, glm::vec3(0, 0, 1));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(location));
	model.Draw(shader);
}
#pragma endregion

#pragma region User input
// Is called whenever a key is pressed/released via GLFW
void keyHandler(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}
#pragma endregion