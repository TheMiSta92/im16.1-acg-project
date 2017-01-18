#include <glm/glm.hpp>
#include <learnopengl/shader.h>

#pragma once
class RainSystem
{
public:
	RainSystem();
	void updateParticles(float deltaTime);
	void drawParticles(Shader shader, glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
	~RainSystem();

private:
	void bindData();
	void renewParticle(int particleIndex);
	GLfloat randomBetween0And1();
	int getArrayIdxOfParticleIdxForX(int particleIdx);
	int getArrayIdxOfParticleIdxForY(int particleIdx);
	int getArrayIdxOfParticleIdxForZ(int particleIdx);
	GLfloat generateRandomXZInRange();
	GLfloat generateRandomYInRange();
};

