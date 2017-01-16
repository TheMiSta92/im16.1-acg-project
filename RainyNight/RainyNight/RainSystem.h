#include <glm/glm.hpp>
#include <learnopengl/shader.h>

// Structure particles
struct Particle {
	glm::vec3 Position;
	float Velocity;

	Particle() { 
		Position.x = (rand() % 1000) / 1000.0f * 50.0f - 25.0f;
		Position.y = 3.0f + (rand() % 50000) / 10000.0f;
		Position.z = (rand() % 1000) / 1000.0f * 50.0f - 25.0f;
		Velocity = 2.0f;
	}
};

#pragma once
class RainSystem
{
public:
	RainSystem();
	void updateParticles(float deltaTime);
	void drawParticles(Shader shader, glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
	~RainSystem();

private:
	void emittingParticles();
	int getFirstDeadParticle();
	void renewParticle(Particle &particle);
};

