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
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
bool keys[1024];
glm::mat4 projectionMatrix = glm::mat4();
glm::mat4 viewMatrix = glm::mat4();

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Testmode
enum Testmode {
	Models,
	Effects,
	Both
};
Testmode testmode;


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
	Shader shaderWater = loadShader("water");
	Shader shaderTexture = loadShader("texture");
	Shader shaderRain = loadShader("rain");

	// Load models
	Model modelHouse = loadModel("resources/objects/house/Farmhouse.obj");
	Model modelRock = loadModel("resources/objects/rock/rock.obj");
	Model modelTerrain = loadModel("resources/objects/terrain/terrain.fbx");

	// Initialize RainSystem
	RainSystem *rainSystem = new RainSystem();

	// Initialize AnimatedWater
	AnimatedWaterSystem *animatedWaterSystem = new AnimatedWaterSystem();
	GLfloat lastWaterSwing = 0.0f;

	//******** Select Testmode **********
	testmode = Testmode::Effects;
	//testmode = Testmode::Models;
	//testmode = Testmode::Both;
	//***********************************

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
		glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Camera
		doScriptedCameraMovement(currentFrame);
		projectionMatrix = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		viewMatrix = camera.GetViewMatrix();

		if (testmode == Testmode::Effects || testmode == Testmode::Both) {
			// AnimatedWaterSystem
			if (currentFrame >= lastWaterSwing + 6.0f) {
				animatedWaterSystem->startSwinging(0, 0);
				lastWaterSwing = currentFrame;
			}
			animatedWaterSystem->updateSwinging(deltaTime);
			glUseProgram(shaderWater.Program);
			animatedWaterSystem->drawWater(shaderWater, projectionMatrix, viewMatrix);

			// RainSystem	
			rainSystem->updateParticles(deltaTime);
			glUseProgram(shaderRain.Program);
			rainSystem->drawParticles(shaderRain, projectionMatrix, viewMatrix);
		}

		if (testmode == Testmode::Models || testmode == Testmode::Both) {
			// Draw Terrain
			glUseProgram(shaderSimpleGreen.Program);
			drawModel(modelTerrain, shaderSimpleGreen, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(20.0f, 20.0f, 20.0f));

			// Draw House
			glUseProgram(shaderTexture.Program);
			drawModel(modelHouse, shaderTexture, glm::vec3(0.0f, -0.02f, 0.0f), glm::vec3(0.0f, 200.0f, 0.0f), glm::vec3(0.05f, 0.05f, 0.05f));

			// Draw Rocks
			drawModel(modelRock, shaderTexture, glm::vec3(0.0f, -0.03f, 2.0f), glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.05f, 0.05f, 0.05f));
			drawModel(modelRock, shaderTexture, glm::vec3(1.0f, -0.03f, 2.5f), glm::vec3(0.0f, 45.0f, 0.0f), glm::vec3(0.06f, 0.06f, 0.06f));

			// Draw Pond
			drawModel(modelRock, shaderTexture, glm::vec3(-2.45f, -0.05f, -1.1f), glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.08f, 0.08f, 0.08f));
			drawModel(modelRock, shaderTexture, glm::vec3(-2.25f, -0.045f, -1.2f), glm::vec3(0.0f, 130.0f, 5.0f), glm::vec3(0.09f, 0.09f, 0.09f));
			drawModel(modelRock, shaderTexture, glm::vec3(-2.05f, -0.03f, -1.15f), glm::vec3(30.0f, 170.0f, 0.0f), glm::vec3(0.06f, 0.06f, 0.06f));
			drawModel(modelRock, shaderTexture, glm::vec3(-1.88f, -0.03f, -1.05f), glm::vec3(10.0f, 70.0f, 0.0f), glm::vec3(0.06f, 0.06f, 0.06f));
			drawModel(modelRock, shaderTexture, glm::vec3(-1.82f, -0.03f, -0.9f), glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.06f, 0.06f, 0.06f));
			drawModel(modelRock, shaderTexture, glm::vec3(-1.80f, -0.037f, -0.72f), glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.05f, 0.05f, 0.05f));
			drawModel(modelRock, shaderTexture, glm::vec3(-1.80f, -0.04f, -0.50f), glm::vec3(0.0f, 80.0f, 0.0f), glm::vec3(0.07f, 0.07f, 0.07f));
			drawModel(modelRock, shaderTexture, glm::vec3(-1.85f, -0.045f, -0.30f), glm::vec3(0.0f, 210.0f, 0.0f), glm::vec3(0.065f, 0.065f, 0.065f));
			drawModel(modelRock, shaderTexture, glm::vec3(-2.05f, -0.035f, -0.28f), glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.08f, 0.08f, 0.09f));
			drawModel(modelRock, shaderTexture, glm::vec3(-2.15f, -0.03f, -0.32f), glm::vec3(0.0f, 90.0f, 0.0f), glm::vec3(0.07f, 0.07f, 0.07f));
			drawModel(modelRock, shaderTexture, glm::vec3(-2.3f, -0.025f, -0.5f), glm::vec3(0.0f, 60.0f, 0.0f), glm::vec3(0.07f, 0.07f, 0.07f));
			drawModel(modelRock, shaderTexture, glm::vec3(-2.45f, -0.03f, -0.7f), glm::vec3(0.0f, 150.0f, 0.0f), glm::vec3(0.08f, 0.08f, 0.07f));
			drawModel(modelRock, shaderTexture, glm::vec3(-2.52f, -0.01f, -0.95f), glm::vec3(0.0f, 250.0f, 0.0f), glm::vec3(0.06f, 0.06f, 0.06f));
		}

		// Swap the buffers
		glfwSwapBuffers(window);

	}

	glfwTerminate();
	return 0;
}

#pragma region Camera Management
// Scripted Camera Movement
void doScriptedCameraMovement(GLfloat currentFrame) {
	cameraRide(currentFrame, 0.0f, glm::vec3(0.0f, 0.3f, 5.0f), 10.0f, 0.0f, 5.0f, glm::vec3(0.0f, 0.3f, 5.0f), 10.0f, 0.0f);				// init look
	cameraRide(currentFrame, 5.0f, glm::vec3(0.0f, 0.3f, 5.0f), 10.0f, 0.0f, 10.0f, glm::vec3(1.5f, 0.3f, 3.0f), -40.0f, -5.0f);			// go to the right of the rock
	cameraRide(currentFrame, 10.0f, glm::vec3(1.5f, 0.3f, 3.0f), -40.0f, -5.0f, 11.0f, glm::vec3(1.5f, 0.3f, 2.5f), -30.0f, -2.5f);			// house side
	cameraRide(currentFrame, 11.0f, glm::vec3(1.5f, 0.3f, 2.5f), -30.0f, -2.5f, 12.0f, glm::vec3(1.5f, 0.3f, 2.0f), -20.0f, 0.0f);			// house side
	cameraRide(currentFrame, 12.0f, glm::vec3(1.5f, 0.3f, 2.0f), -20.0f, 0.0f, 14.0f, glm::vec3(1.5f, 0.3f, 1.0f), -20.0f, 0.0f);			// house side
	cameraRide(currentFrame, 14.0f, glm::vec3(1.5f, 0.3f, 1.0f), -20.0f, 0.0f, 16.0f, glm::vec3(1.5f, 0.3f, 0.0f), -40.0f, 0.0f);			// house side
	cameraRide(currentFrame, 16.0f, glm::vec3(1.5f, 0.3f, 0.0f), -40.0f, 0.0f, 21.0f, glm::vec3(0.5f, 0.3f, -2.5f), -140.0f, 0.0f);			// house back corner
	cameraRide(currentFrame, 21.0f, glm::vec3(0.5f, 0.3f, -2.5f), -140.0f, 0.0f, 26.0f, glm::vec3(-1.5f, 0.3f, -2.0f), -210.0f, 0.0f);		// house other back corner
	cameraRide(currentFrame, 26.0f, glm::vec3(-1.5f, 0.3f, -2.0f), -210.0, 0.0f, 28.0f, glm::vec3(-2.0f, 0.3f, -1.8f), -160.0f, -20.0f);	// to the pond
	cameraRide(currentFrame, 28.0f, glm::vec3(-2.0f, 0.3f, -1.8f), -160.0, -20.0f, 30.0f, glm::vec3(-2.0f, 0.3f, -1.8f), -180.0f, -10.0f);	// pond-look around
	cameraRide(currentFrame, 30.0f, glm::vec3(-2.0f, 0.3f, -1.8f), -180.0, -10.0f, 32.0f, glm::vec3(-2.0f, 0.3f, -1.8f), -170.0f, -15.0f);	// pond-look around
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