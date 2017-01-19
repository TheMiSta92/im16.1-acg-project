#include "AnimatedWaterSystem.h"

// System
#include <vector>

// GL
#include <learnopengl/shader.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM Mathemtics
#define M_PI 3.14159265358979323846f 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// PARAMETERS
const GLfloat CENTER_X = 0.0f;
const GLfloat CENTER_Y = 0.0f;
const GLfloat CENTER_Z = 0.0f;
const GLfloat SIZE_SINGLE = 0.2f;
const GLint AMOUNT_EDGE = 21;	// should be odd
static const GLfloat GEOMETRY_SINGLE_DATA[] = {
	-SIZE_SINGLE / 2.0f, 0.0f, -SIZE_SINGLE / 2.0f,
	SIZE_SINGLE / 2.0f, 0.0f, -SIZE_SINGLE / 2.0f,
	-SIZE_SINGLE / 2.0f, 0.0f, SIZE_SINGLE / 2.0f,
	SIZE_SINGLE / 2.0f, 0.0f, SIZE_SINGLE / 2.0f
};

// Data
std::vector<GLfloat> AW_positions;
GLuint AW_geometry_buffer_id;
GLuint AW_positions_buffer_id;

AnimatedWaterSystem::AnimatedWaterSystem() {
	generateSubPlanesPositions();
	bindData();
}

AnimatedWaterSystem::~AnimatedWaterSystem() {
	for (int i = 0; i < AW_positions.size(); i++) {
		AW_positions[i] = 0;
	}
	AW_positions.clear();
}

void AnimatedWaterSystem::generateSubPlanesPositions() {
	for (GLint x = -AMOUNT_EDGE / 2; x <= AMOUNT_EDGE / 2; x++) {
		for (GLint z = -AMOUNT_EDGE / 2; z <= AMOUNT_EDGE / 2; z++) {
			AW_positions.push_back(CENTER_X + x * SIZE_SINGLE);	// x
			AW_positions.push_back(CENTER_Y);					// y
			AW_positions.push_back(CENTER_Z + z * SIZE_SINGLE);	// z
		}
	}
}

void AnimatedWaterSystem::bindData() {
	// Vertices buffer
	glGenBuffers(1, &AW_geometry_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, AW_geometry_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GEOMETRY_SINGLE_DATA), GEOMETRY_SINGLE_DATA, GL_STATIC_DRAW);

	// Positions buffer
	glGenBuffers(1, &AW_positions_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, AW_positions_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, AMOUNT_EDGE * AMOUNT_EDGE * 3 * sizeof(GLfloat), AW_positions.data(), GL_STREAM_DRAW);
}