#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "DataTypes.h"
#include "GLDisplay.h"
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

// note have to use float everywhere since glm uses float
constexpr float dt = 0.1f;

// could add an external field as a possibility?

array_len import_vertices(std::string file_name)
{
	std::ifstream vertices;
	vertices.open(file_name);
	std::string data_line;
	int num_vertices = 0;
	while (std::getline(vertices, data_line))
	{
		if (data_line[0] == 'v')
		{
			// vertices only start with 'v'
			// first find out number of v's to set array size
			num_vertices++;
		}
	}
	// now restart and actually read data
	vertices.clear();
	vertices.seekg(0);
	// have to change if using color eg (for now 3 for x,y,z) but could have another function to
	// alter array later:
	int num_elements = 3 * num_vertices;
	float* vertex_data = new float[num_elements];

	// use vertices >> v >> x >> y >> z
	// instead here since reading elements not lines
	int pos = 0;
	while (std::getline(vertices, data_line))
	{
		if (data_line[0] == 'o') // declaring object, followed by vertices
		{
			break;
		}
	}
	char v;
	float x, y, z;
	while (vertices >> v >> x >> y >> z)
	{
		if (v == 'v')
		{
			// vertices only start with 'v'
			// then read in x,y,z floats
			vertex_data[3 * pos + 0] = x;
			vertex_data[3 * pos + 1] = y;
			vertex_data[3 * pos + 2] = z;
			pos++;
		}
		else
		{
			break;
		}
	}
	return { vertex_data, num_elements };
}

inline void update_position(Particle& p)
{
	p.pos += p.vel * dt;
}

void apply_force(Particle& p, glm::vec3 f)
{
	p.vel += (1.0f / p.mass) * f * dt; // euler step
}

glm::vec3 compute_force(const Particle& a, const Particle& b)
{
	// kqq/r^2 rhat = kqqr/r^3
	glm::vec3 r = a.pos - b.pos;
	// ignore k constant
	return a.charge * b.charge * r / pow(glm::length(r), 3);
}

void update_particles(std::vector<Particle>& particles)
{
	// strategy: for each particle, calculate forces from others
	// then after finding all forces, finally update the particles
	// # forces to save = # particles since summing constituent forces
	// how to represent a force? a vector, and then do euler step F = m*dv
	std::vector<glm::vec3> forces;
	for (int i = 0; i < particles.size(); i++)
	{
		Particle& a = particles[i];
		forces.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
		// for each particle apply the affects of every other particle
		for (int j = 0; j < particles.size(); j++)
		{
			if (j != i) // i.e. unique particle
			{
				Particle& b = particles[j];
				// add to current force the new force (i.e. forces[i] is total force on particles[i])
				forces[i] += compute_force(a, b);
			}
		}
	}
	// now have all forces, so apply:
	for (int i = 0; i < particles.size(); i++)
	{
		apply_force(particles[i], forces[i]);
	}
	// that was for forces, now we need to update the positions from the velocities that changed as a result:
	for (int i = 0; i < particles.size(); i++)
	{
		update_position(particles[i]);
	}
}

int main()
{
	// glfw caps at 60fps
	array_len sphere_vertices = import_vertices("sphere.obj");

	Particle a = { glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(-0.00f, 0.0f, -0.1f), 0.5f, 5.0f };
	Particle b = { glm::vec3(-5.0f, 0.0f, 0.0f), glm::vec3(0.00f, 0.0f, 0.1f), 0.5f, 5.0f };
	Particle c = { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.001f, 0.0f), -0.5f, 100.0f };
	//Particle a = { glm::vec3(-10.0f, 3.0f, 0.0f), glm::vec3(0.02f, 0.0f, 0.0f), 0.3f, 10.0f };
	//Particle b = { glm::vec3(2.0f, -5.0f, 0.0f), glm::vec3(-0.02f, 0.0f, 0.0f), -0.3f, 10.0f };
	std::vector<Particle> particles;
	particles.push_back(a);
	particles.push_back(b);
	particles.push_back(c);

	GLFWwindow* glwindow = init_window();
	GLData* gldata = init_gl(sphere_vertices.arr, sphere_vertices.len, NULL, 0);


	while (!update_window(glwindow, gldata, particles))
	{
		update_particles(particles);
	}
}