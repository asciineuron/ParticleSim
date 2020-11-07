#include <iostream>
#include "DataTypes.h"
#include "GLDisplay.h"
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

const char* vertexShaderSource = "#version 460 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection*view*model*vec4(aPos, 1.0);\n"
"}\0";

const char* fragmentShaderSource = "#version 460 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

GLFWwindow* init_window()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "ParticleSim", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}

	glViewport(0, 0, 800, 600);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	/*while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();*/

	// only needs be set once:
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	return window;
}

int update_window(GLFWwindow* window, const GLData* data, const std::vector<Particle>& particles)
{
	// not sure best way to make sure it works...
	if (window)
	{
		// so not stuck in while loop
		if (glfwWindowShouldClose(window))
		{
			glfwTerminate();
			return 1;
		}

		processInput(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (data)
		{
			if (data->shader)
				glUseProgram(data->shader);
			if (data->VAO)
				glBindVertexArray(data->VAO);
			// need number of points: using just vertices for now since from .obj file
			//glDrawArrays(GL_TRIANGLES, 0, data->num_vertices);
			draw_particles(data, particles);
			//glDrawElements(GL_TRIANGLES, data->num_indices, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
		return 0;
	}
	return 1;
}

unsigned int gen_shader()
{
	// create/compile vertex shader:
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check for success
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// create/compile fragment shader:
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check success again
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// create/link shader program:
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check success again
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	glUseProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return shaderProgram;
}

GLData* init_gl(float* vertices, int num_vertices, unsigned int* indices, int num_indices)
{
	// for now only works for single set of vertices/indices...
	// generate VAO:
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	// generate VBO:
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// load into memory: (will have to re-call when color updated)
	// set to DYNAMIC vs STATIC since will be altering
	glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(float), vertices, GL_DYNAMIC_DRAW);
	// generate EBO:
	unsigned int EBO = 0;
	if (indices)
	{
		// only generate if passed indices
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(unsigned int), indices, GL_DYNAMIC_DRAW);
	}
	unsigned int shader = gen_shader();

	// how to interpret vertex data:
	// position:
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color: but not changing color this time
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
	//glEnableVertexAttribArray(1);
	// unbind VAO for safety:
	glBindVertexArray(0);

	// won't change projection or view matrix so can set here:
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -15.0f));
	glm::mat4 projection = glm::mat4(1.0f);;
	projection = glm::perspective(glm::radians(75.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	int viewLoc = glGetUniformLocation(shader, "view");
	int projLoc = glGetUniformLocation(shader, "projection");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	GLData* out = new GLData();
	*out = { vertices, num_vertices, indices, num_indices, VAO, VBO, EBO, shader };
	return out;
}

void draw_particles(const GLData* data, const std::vector<Particle>& particles)
{
	for (const Particle& p : particles)
	{
		// generate matrix from position to offset the point particle mesh model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, p.pos);
		int modelLoc = glGetUniformLocation(data->shader, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		//glUseProgram(data->shader); ~ already do in update_window, this is just drawing the vertices
		//glBindVertexArray(data->VAO);

		glDrawArrays(GL_POINTS, 0, data->num_vertices);
		//glBindVertexArray(0);
	}
}

void update_gl_vertices(GLData* data)
{
	// need to rewrite this to take into account each particle, reapplying the matrices then drawing
	glBufferData(GL_ARRAY_BUFFER, data->num_vertices * sizeof(float), data->vertices, GL_DYNAMIC_DRAW);
}

void delete_gldata(GLData* data)
{
	glDeleteVertexArrays(1, &(data->VAO));
	glDeleteBuffers(1, &(data->VBO));
	glDeleteBuffers(1, &(data->EBO));
	glDeleteProgram(data->shader);
}