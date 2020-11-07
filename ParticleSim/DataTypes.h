#pragma once
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

struct array_len
{
	float* arr;
	int len;
};

struct Particle
{
	// will make use of glm to more easily integrate into OpenGL...
	glm::vec3 pos;
	glm::vec3 vel;
	float charge; // its attraction to other particles, could add more later (mass etc.)
	float mass; // for acceleration, ignore gravity for now
};
