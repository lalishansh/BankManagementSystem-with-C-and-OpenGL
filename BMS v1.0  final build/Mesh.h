#pragma once

#include <GL\glew.h>
#include <GLM\glm.hpp>
#include <vector>

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};
class Mesh
{
public:
	Mesh();

	void CreateMesh(std::vector<Vertex> vertices, unsigned int* indices, unsigned int numOfIndices);
	void CreateMesh(GLfloat *vertices, unsigned int *indices, unsigned int numOfVertices, unsigned int numOfIndices);
	void RenderMesh();
	void ClearMesh();

	~Mesh();

private:
	GLuint VAO, VBO, IBO;
	GLsizei indexCount;
};

