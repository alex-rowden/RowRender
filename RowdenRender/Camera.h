#pragma once
#include "RowRender.h"
class Camera
{
public:
	Camera(glm::vec3 position, glm::vec3 target, float fov, float aspect, glm::vec3 up = glm::vec3(0,1,0));
	glm::mat4 getView();
	glm::vec3 getPosition();
	void moveForward(float amount);
	void moveUp(float amount);
	void moveRight(float amount);
	void setDirection(glm::vec3 dir);
	float pitch, yaw, fov, aspect;
	
private:
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 right;
	glm::vec3 up = glm::vec3(0, 1, 0);
};

