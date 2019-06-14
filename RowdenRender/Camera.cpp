#include "Camera.h"

Camera::Camera(glm::vec3 _position, glm::vec3 target, glm::vec3 _up) {
	up = _up;
	position = _position;
	direction = glm::normalize(position - target);
	right = glm::normalize(glm::cross(up, direction));
	pitch = asin(-direction.y);
	yaw = atan2(direction.x, direction.z);
}

void Camera::setDirection(glm::vec3 dir) {
	direction = dir;
}

glm::mat4 Camera::getView() {
	return glm::lookAt(position, position - direction, up);
}

glm::vec3 Camera::getPosition() {
	return position;
}

void Camera::moveForward(float amount) {
	position += direction * amount;
}

void Camera::moveUp(float amount) {
	position += up * amount;
}

void Camera::moveRight(float amount) {
	position += right * amount;
}
