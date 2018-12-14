#ifndef ETEXTURE_H
#define ETEXTURE_H

#include <string>
#include <vector>
#include <glm\glm.hpp>
#include <iostream>
#include <fstream>
#include <ostream>
#include "stb_image.h"
#include "../GL/gl3w.h"
#include "../GL/glcorearb.h"
class ETexture
{
public:
	ETexture()
	{
		data = NULL;
	};
	ETexture(std::string dir)
	{
		init(dir);
	};
	~ETexture();
	void init(std::string dir);
	void Serialize(std::ofstream * stream);
	void FromSerialize(std::ifstream * stream);
	void fromTexture(ETexture &t);
	glm::ivec2 getDimensions();
	void freeData();
	void freeAll();
	unsigned char * getData();
	int getBytesPerPixel();
	unsigned int getTextureID();
	void genTexture();
	bool readyToRender();
private:
	int width = 0, height = 0;
	unsigned char * data = NULL;
	int bytesPerPixel = 0;
	unsigned int textureID = 0;
};



#endif