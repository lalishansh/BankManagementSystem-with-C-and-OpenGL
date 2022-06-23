#pragma once

#include <GL\glew.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <GLFW\glfw3.h>
#include<iostream>
#include "CommonDefs.h"
class Camera
{
public:
	Camera();
	Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed);

	void keyControl(bool* keys, GLfloat deltaTime);
	void joyStickControl(const unsigned char* buttons, const float* axes, GLfloat deltaTime);
	void mouseControl(GLfloat xChange, GLfloat yChange);

	void Orbit(glm::vec3 orbitOrigin);
	void Orbit(bool status);

	void setCameraPosition(glm::vec3 newPosn) { position = newPosn; };
	glm::vec3 getCameraPosition();
	glm::vec3 getCameraDirection();

	glm::mat4 calculateViewMatrix();

	~Camera();

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	glm::vec3 orbitAround;
	bool orbit;

	GLfloat yaw;
	GLfloat pitch;

	GLfloat moveSpeed;
	GLfloat turnSpeed;

	void update();
	void retainPitchYaw();
};

