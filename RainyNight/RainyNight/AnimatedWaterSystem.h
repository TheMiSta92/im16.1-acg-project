#include <glm/glm.hpp>
#include <learnopengl/shader.h>

#pragma once
class AnimatedWaterSystem
{
public:
	AnimatedWaterSystem();
	~AnimatedWaterSystem();

private:
	void generateSubPlanesPositions();
	void bindData();
};

