#pragma once

#include<cstring>
#include <GL/glew.h>
#include "stb_image.h"

class Texture
{
public:
	Texture();
	Texture(const char* fileLoc);
	~Texture();

	bool LoadTexture();
	bool LoadTextureA();
	void UseTexture();
	void UseTexture(unsigned int texUnit);
	void ClearTexture();

	void ClearTexture(bool deleteTex);

	GLuint GetTextureID();
private:
	GLuint textureId;
	int width, height, bitDepth;
	const char* fileLocation;
};

