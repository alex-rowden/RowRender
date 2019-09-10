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
	direction = glm::normalize(dir);
}

glm::mat4 Camera::getView() {
	return glm::lookAt(position, position + direction, up);
}

glm::mat4 Camera::getProjection() {
	return glm::perspective(fov, aspect, .1f, 1000.0f);
}

glm::vec3 Camera::getPosition() {
	return position;
}

void Camera::moveForward(float amount) {
	float y = position.z;
	
	position += direction * -amount;
	if (lock_axis)
		position.z = y;
}

void Camera::moveUp(float amount) {
	float y = position.z;
	position += up * -amount;
	if (lock_axis)
		position.z = y;
}

void Camera::moveRight(float amount) {
	right = glm::normalize(glm::cross(direction, up));
	float y = position.z;
	position += right * amount;
	if (lock_axis)
		position.z = y;
}
