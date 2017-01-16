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
const GLfloat START_Y = 3.0f;
const GLfloat END_Y = -1.0f;
const GLfloat VELOCITY = 2.0f;

// Particles
GLuint max_particles = 5000;
std::vector<Particle> particles;	//Array

RainSystem::RainSystem() {
	// Fill array with empty/dead/default particles for faster check of free particle array spaces
	for (GLuint i = 0; i < max_particles; i++) {
		particles.push_back(Particle());
	}
}

// Replaces dead particles and emitts new ones
void RainSystem::emittingParticles() {
	GLuint maxAmountNewParticles = 10;
	// Adding new particles
	for (GLuint i = 0; i < maxAmountNewParticles; i++) {
		int deadParticleIndex = getFirstDeadParticle();
		// Checks if there is a dead particle, then respawn it
		if (deadParticleIndex != -1) {
			renewParticle(particles[deadParticleIndex]);
		}
		else {
			return;
		}
	}
}

// Updating all particles in array every frame
void RainSystem::updateParticles(float deltaTime) {
	for (GLuint i = 0; i < max_particles; i++) {
		// Pointer Nachverfolgung
		Particle &p = particles[i];
		if (p.Position.y > END_Y) {
			// Particle haven't reached the end position yet
			p.Position.y -= p.Velocity * deltaTime;
		}
	}
	// 8-)
	emittingParticles();
}

// Returns first dead particle from array
// If no dead particle found -> returns -1
int RainSystem::getFirstDeadParticle() {
	for (GLuint i = 0; i < max_particles; i++) {
		if (particles[i].Position.y <= END_Y) {
			return i;
		}
	}
	return -1;
}

// Reactivates dead particle
// "Emitter"
void RainSystem::renewParticle(Particle &particle) {
	particle.Position.x = (rand() % 1000) / 1000.0f * 50.0f - 25.0f;
	particle.Position.y = START_Y + (rand() % 50000) / 10000.0f;
	particle.Position.z = (rand() % 1000) / 1000.0f * 50.0f - 25.0f;
	particle.Velocity = VELOCITY;
}

// Draw the particles
void RainSystem::drawParticles(Shader shader, glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {
	for (Particle particle : particles) {
		if (particle.Position.y > END_Y) {
			//std::cout << "Regen da" << std::endl;
			GLint vertexPositionLocation = glGetUniformLocation(shader.Program, "position");
			glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
			glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
			// TODO: Bind particle.Position to location = 0 | GET HELP
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)offsetof(Particle, Position));
			glEnableVertexAttribArray(0);
		}
		else {
			//std::cout << "Regen weg" << std::endl;
		}
	}
}

RainSystem::~RainSystem()
{
}
