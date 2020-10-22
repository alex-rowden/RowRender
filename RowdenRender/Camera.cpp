#include "Camera.h"

Camera::Camera(glm::vec3 _position, glm::vec3 target, float _fov, float _aspect, glm::vec3 _up) {
	up = _up;
	position = _position;
	direction = glm::normalize(target - position);
	right = glm::normalize(glm::cross(up, direction));
	pitch = asin(-direction.y);
	yaw = atan2(direction.x, direction.z);
	aspect = _aspect;
	fov = _fov;
}

void Camera::setDirection(glm::vec3 dir) {
	moved = true;
	direction = glm::normalize(dir);
}

glm::mat4 Camera::getView() {
	return glm::lookAt(position, position + direction, up);
	//return glm::mat4(1);
}

glm::mat4 Camera::getProjection() {
	return glm::perspective(glm::radians(fov), aspect, .1f, 1000.0f);
}

glm::mat4 Camera::getProjection(float zNear) {
	return glm::perspective(glm::radians(fov), aspect, zNear, 1000.0f);
}glm::mat4 Camera::getProjection(float zNear, float zFar) {
	return glm::perspective(glm::radians(fov), aspect, zNear, zFar);
}

glm::vec3 Camera::getPosition() {
	return position;
}

void Camera::moveForward(float amount) {
	moved = true;
	position += direction * -amount;
}

void Camera::moveUp(float amount) {
	moved = true;
	position += up * -amount;
}

void Camera::moveRight(float amount) {
	moved = true;
	right = glm::normalize(glm::cross(direction, up));
	position += right * amount;
}
