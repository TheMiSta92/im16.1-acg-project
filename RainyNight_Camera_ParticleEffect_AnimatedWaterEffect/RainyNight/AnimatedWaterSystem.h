#include <glm/glm.hpp>
#include <learnopengl/shader.h>

#pragma once
class AnimatedWaterSystem
{
public:
	AnimatedWaterSystem();
	void startSwinging(int planeX, int planeZ);
	void updateSwinging(GLfloat deltaTime);
	void drawWater(Shader shader, glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
	~AnimatedWaterSystem();

private:
	void generateSubPlanesPositions();
	void bindData();
	int planeCoordToPlaneArrayNr(int x, int z);
	int planeArrayNrToArrayCoordY(int planeArrayNr);
	int planeCoordToArrayCoordY(int x, int z);
};

