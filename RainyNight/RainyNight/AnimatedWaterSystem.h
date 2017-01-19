#include <glm/glm.hpp>
#include <learnopengl/shader.h>

#pragma once
class AnimatedWaterSystem
{
public:
	AnimatedWaterSystem();
	void AnimatedWaterSystem::drawWater(Shader shader, glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
	~AnimatedWaterSystem();

private:
	void generateSubPlanesPositions();
	void bindData();
	int planeCoordToPlaneArrayNr(int x, int z);
	int planeCoordToArrayCoordY(int x, int z);
};

