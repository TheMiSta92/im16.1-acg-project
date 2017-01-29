#include "RainSystem.h"

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

// Parameters
const GLfloat START_XZ_MIN = -10.0f;
const GLfloat START_XZ_DIFF_MAX = 20.0f;
const GLfloat START_Y_MIN = 3.0f;
const GLfloat START_Y_DIFF_MAX = 2.0f;
const GLfloat END_Y = -1.0f;
const GLfloat VELOCITY = 10.0f;
const GLuint PRECISION = 10000;
const GLuint PARTICLES_AMOUNT = 3000;

// Particles
std::vector<GLfloat> RS_positions;
GLuint RS_vertex_buffer_id;
GLuint RS_positions_buffer_id;
GLuint RS_vao;

RainSystem::RainSystem() {
	// Fill array with particles
	for (GLuint particleIdx = 0; particleIdx < PARTICLES_AMOUNT; particleIdx++) {
		RS_positions.push_back(generateRandomXZInRange());	// x
		RS_positions.push_back(generateRandomYInRange());	// y
		RS_positions.push_back(generateRandomXZInRange());	// z
	}
	bindData();
}

// Updating all particles in array every frame
void RainSystem::updateParticles(float deltaTime) {
	for (GLuint particleIdx = 0; particleIdx < PARTICLES_AMOUNT; particleIdx++) {
		if (RS_positions[getArrayIdxOfParticleIdxForY(particleIdx)] > END_Y) {
			// Particle haven't reached the end position yet
			RS_positions[getArrayIdxOfParticleIdxForY(particleIdx)] -= VELOCITY * deltaTime;	// v * t = distance

			// showing that it's working
			if (particleIdx == 0) {
				//std::cout << "Rain-Drop #0: y = " << RS_positions[getArrayIdxOfParticleIdxForY(0)] << std::endl;
			}
		}
		else {
			// Particle is below minimum height
			// Respawn
			renewParticle(particleIdx);
		}
	}
	//emittingParticles();
}

// Draw the particles
void RainSystem::drawParticles(Shader shader, glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {
	// Uniforms
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Positions buffer
	glGenBuffers(1, &RS_positions_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, RS_positions_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, PARTICLES_AMOUNT * 3 * sizeof(GLfloat), RS_positions.data(), GL_STREAM_DRAW);

	// 0th buffer - vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, RS_vertex_buffer_id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// 1st buffer - position of particle
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, RS_positions_buffer_id);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Vertex Divisor
	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 1);
	
	// Draw
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, PARTICLES_AMOUNT);

	// Free-ing
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// Destroy RainSystem
RainSystem::~RainSystem() {
	for (int particleIdx = 0; particleIdx < PARTICLES_AMOUNT; particleIdx++) {
		RS_positions[getArrayIdxOfParticleIdxForX(particleIdx)] = 0;
		RS_positions[getArrayIdxOfParticleIdxForY(particleIdx)] = 0;
		RS_positions[getArrayIdxOfParticleIdxForZ(particleIdx)] = 0;
	}
	RS_positions.clear();
}

// Replaces dead particles and emitts new ones
//void RainSystem::emittingParticles() {
//	GLuint maxAmountNewParticles = 10;
	// Adding new particles
//	for (GLuint i = 0; i < maxAmountNewParticles; i++) {
//		int deadParticleIndex = getFirstDeadParticle();
		// Checks if there is a dead particle, then respawn it
//		if (deadParticleIndex != -1) {
//			renewParticle(deadParticleIndex);
//		}
//		else {
//			return;
//		}
//	}
//}

// Returns first dead particle from array
// If no dead particle found -> returns -1
//int RainSystem::getFirstDeadParticle() {
//	for (GLuint particleIdx = 0; particleIdx < max_particles; particleIdx++) {
//		if (getArrayIdxOfParticleIdxForY(particleIdx) <= END_Y) {
//			return particleIdx;
//		}
//	}
//	return -1;
//}

// Binds data
void RainSystem::bindData() {
	// Vertices of rain-drops
	static const GLfloat vertex_buffer_data[] = {
		-0.004f, -0.02f, 0.0f,
		0.004f, -0.02f, 0.0f,
		-0.004f, 0.02f, 0.0f,
		0.004f, 0.02f, 0.0f
	};

	// VAO
	glGenVertexArrays(1, &RS_vao);
	glBindVertexArray(RS_vao);

	// Vertices buffer
	glGenBuffers(1, &RS_vertex_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, RS_vertex_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

	// Positions buffer
	glGenBuffers(1, &RS_positions_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, RS_positions_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, PARTICLES_AMOUNT * 3 * sizeof(GLfloat), RS_positions.data(), GL_STREAM_DRAW);
}

// Reactivates dead particle
// "Emitter"
void RainSystem::renewParticle(int particleIndex) {
	RS_positions[getArrayIdxOfParticleIdxForX(particleIndex)] = generateRandomXZInRange();
	RS_positions[getArrayIdxOfParticleIdxForY(particleIndex)] = generateRandomYInRange();
	RS_positions[getArrayIdxOfParticleIdxForZ(particleIndex)] = generateRandomXZInRange();
}

// Returns a random number between 0 and 1
GLfloat RainSystem::randomBetween0And1() {
	return (rand() % PRECISION) / (PRECISION * 1.0f);
}

// Returns to array-index for the x-position of a certain particle
int RainSystem::getArrayIdxOfParticleIdxForX(int particleIdx) {
	return particleIdx * 3;
}

// Returns to array-index for the y-position of a certain particle
int RainSystem::getArrayIdxOfParticleIdxForY(int particleIdx) {
	return particleIdx * 3 + 1;
}

// Returns to array-index for the z-position of a certain particle
int RainSystem::getArrayIdxOfParticleIdxForZ(int particleIdx) {
	return particleIdx * 3 + 2;
}

// Generates a random position which is suitable for X- and Z-position
GLfloat RainSystem::generateRandomXZInRange() {
	return START_XZ_MIN + randomBetween0And1() * START_XZ_DIFF_MAX;
}

// Generates a random position which is suitable for Y-position
GLfloat RainSystem::generateRandomYInRange() {
	return START_Y_MIN + randomBetween0And1() * START_Y_DIFF_MAX;
}