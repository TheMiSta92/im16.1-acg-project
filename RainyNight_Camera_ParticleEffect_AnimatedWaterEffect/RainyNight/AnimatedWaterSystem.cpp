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
const GLfloat CENTER_X = -4.0f;
const GLfloat CENTER_Y = 0.1f;
const GLfloat CENTER_Z = -2.0f;
const GLfloat SIZE_SINGLE = 0.01f;
const GLint AMOUNT_EDGE = 101;	// should be odd
static const GLfloat GEOMETRY_SINGLE_DATA[] = {
	-SIZE_SINGLE / 2.0f, 0.0f, -SIZE_SINGLE / 2.0f,
	SIZE_SINGLE / 2.0f, 0.0f, -SIZE_SINGLE / 2.0f,
	-SIZE_SINGLE / 2.0f, 0.0f, SIZE_SINGLE / 2.0f,
	SIZE_SINGLE / 2.0f, 0.0f, SIZE_SINGLE / 2.0f
};

// SWINGING PARAMETERS
const GLfloat SWINGING_AMPLITUDE = 0.1f;
const GLfloat SWINGING_SPEED = 2.0f;
const GLfloat SWINGING_FADE = 5.0f;
const GLint SWINGING_SPREAD = 20;
// "swinging" is the vertical water movement of a point
// if "swinging" is activated a point of water will raise up to a [SWINGING_AMPLITUDE], then back up, down, etc. until it stops swinging
// SWINGING_AMPLITUDE defines the maximal amplitude at start
// SWINGING_SPEED defines the length of an sine-cycle in seconds
// SWINGING_FADE defines how long it takes to swing out in seconds
// SWINGING_SPREAD defines how many neighbour-rows/-cols are effected by a swing

// Data
std::vector<GLfloat> AW_positions;
std::vector<GLfloat> AW_swinging_time;	// used for animating swinging of vertices
std::vector<GLfloat> AW_swinging_amp;	// used for animating swinging of vertices (neighbours get less amplitude than main-swinging-tile)
GLuint AW_geometry_buffer_id;
GLuint AW_positions_buffer_id;
GLuint AW_vao;

AnimatedWaterSystem::AnimatedWaterSystem() {
	generateSubPlanesPositions();
	bindData();
}

void AnimatedWaterSystem::startSwinging(int planeX, int planeZ) {
	// main-tile-swinging
	AW_swinging_amp[planeCoordToPlaneArrayNr(planeX, planeZ)] = SWINGING_AMPLITUDE;		// sets the amplitude
	AW_positions[planeCoordToArrayCoordY(planeX, planeZ)] = CENTER_Y;					// set it to start-point (neutral)
	AW_swinging_time[planeCoordToPlaneArrayNr(planeX, planeZ)] = 0.0f;					// makes it ready for updateSwinging (shows that it's at start)

	// neighbour-swinging
	for (int neighbourX = -SWINGING_SPREAD; neighbourX <= SWINGING_SPREAD; neighbourX++) {
		for (int neighbourZ = -SWINGING_SPREAD; neighbourZ <= SWINGING_SPREAD; neighbourZ++) {
			if (planeX + neighbourX <= AMOUNT_EDGE / 2 && planeX + neighbourX >= -AMOUNT_EDGE / 2 && planeZ + neighbourX <= AMOUNT_EDGE / 2 && planeZ + neighbourX >= -AMOUNT_EDGE / 2) {
				// in bounds
				if (neighbourX != 0 || neighbourZ != 0) {
					// not the main-tile
					float distance = std::pow(neighbourX * neighbourX + neighbourZ * neighbourZ, 1.0 / 2.0);
					float amplitude = SWINGING_AMPLITUDE * (1.0f - distance / (SWINGING_SPREAD + 1.0f));
					if (amplitude > 0.0f) {
						AW_swinging_amp[planeCoordToPlaneArrayNr(planeX + neighbourX, planeZ + neighbourZ)] = amplitude;	// sets the amplitude
						AW_positions[planeCoordToArrayCoordY(planeX + neighbourX, planeZ + neighbourZ)] = CENTER_Y;			// set it to start-point (neutral)
						AW_swinging_time[planeCoordToPlaneArrayNr(planeX + neighbourX, planeZ + neighbourZ)] = 0.0f;		// makes it ready for updateSwinging (shows that it's at start)
					}
				}
			}
		}
	}
}

void AnimatedWaterSystem::updateSwinging(GLfloat deltaTime) {
	for (unsigned int planeArrayNr = 0; planeArrayNr < AW_swinging_time.size(); planeArrayNr++) {
		if (AW_swinging_time[planeArrayNr] != -1.0f) {
			// this plane is swinging
			AW_swinging_time[planeArrayNr] += deltaTime;	// NICE: does not count up #tableflip
			if (AW_swinging_time[planeArrayNr] >= SWINGING_FADE) {
				// faded out
				AW_swinging_time[planeArrayNr] = -1.0f;		// end swing
			}
			else {
				// should still swing (would swing if I could increase AW_swinging_time in line 62 ^^)
				GLfloat currentAmplitudeFactor = AW_swinging_amp[planeArrayNr] - (AW_swinging_amp[planeArrayNr] / SWINGING_FADE * AW_swinging_time[planeArrayNr]);
				GLfloat currentHeightDeltaToNeutral = std::sin(2 * M_PI / SWINGING_SPEED * AW_swinging_time[planeArrayNr]) * currentAmplitudeFactor;
				AW_positions[planeArrayNrToArrayCoordY(planeArrayNr)] = CENTER_Y + currentHeightDeltaToNeutral;
				//std::cout << "Height of water in the center of the pond: " << CENTER_Y + currentHeightDeltaToNeutral << std::endl;
			}
		}
	}
}

// Draw the planes
void AnimatedWaterSystem::drawWater(Shader shader, glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {
	// Uniforms
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Positions buffer
	glGenBuffers(1, &AW_positions_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, AW_positions_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, AMOUNT_EDGE * AMOUNT_EDGE * 3 * sizeof(GLfloat), AW_positions.data(), GL_STREAM_DRAW);

	// 0th buffer - vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, AW_geometry_buffer_id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// 1st buffer - position of subplane
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, AW_positions_buffer_id);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Vertex Divisor
	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 1);

	// Draw
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, AMOUNT_EDGE * AMOUNT_EDGE);

	// Free-ing
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

AnimatedWaterSystem::~AnimatedWaterSystem() {
	for (unsigned int i = 0; i < AW_positions.size(); i++) {
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
			AW_swinging_time.push_back(-1.0f);					// set swinging time to -1: not swinging
			AW_swinging_amp.push_back(0.0f);					// initializing swinging_amp array
		}
	}
}

void AnimatedWaterSystem::bindData() {
	// VAO
	glGenVertexArrays(1, &AW_vao);
	glBindVertexArray(AW_vao);

	// Vertices buffer
	glGenBuffers(1, &AW_geometry_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, AW_geometry_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GEOMETRY_SINGLE_DATA), GEOMETRY_SINGLE_DATA, GL_STATIC_DRAW);

	// Positions buffer
	glGenBuffers(1, &AW_positions_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, AW_positions_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, AMOUNT_EDGE * AMOUNT_EDGE * 3 * sizeof(GLfloat), AW_positions.data(), GL_STREAM_DRAW);
}

int AnimatedWaterSystem::planeCoordToPlaneArrayNr(int x, int z) {
	// if AMOUNT_EDGE = 7
	// -3 | -3 = plane #0
	// -3 | -2 = plane #1
	// ...
	// -2 | -3 = plane #7
	// ...

	int arrayX = x + AMOUNT_EDGE / 2;
	int arrayZ = z + AMOUNT_EDGE / 2;
	return arrayX * AMOUNT_EDGE + arrayZ;
}

int AnimatedWaterSystem::planeArrayNrToArrayCoordY(int planeArrayNr) {
	return planeArrayNr * 3 + 1;
}

int AnimatedWaterSystem::planeCoordToArrayCoordY(int x, int z) {
	return planeArrayNrToArrayCoordY(planeCoordToPlaneArrayNr(x, z));
}