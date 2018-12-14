#include "ETexture.h"
ETexture::~ETexture()
{
	freeAll();
}
void  ETexture::init(std::string dir)
{
	int width, height, nrComponents;
	data = stbi_load(dir.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		this->width = width;
		this->height = height;
		this->bytesPerPixel = nrComponents;
	}
	else
		std::cout << "Failed loading texture";
}

void  ETexture::Serialize(std::ofstream * stream)
{

	if (*stream)
	{
		int numPixels = width * height * bytesPerPixel;
		stream->write((char*)&width, sizeof(int));
		stream->write((char*)&height, sizeof(int));
		stream->write((char*)&bytesPerPixel, sizeof(int));
		for (unsigned int i = 0; i < numPixels; i++)
			stream->write((char*)&data[i], sizeof(unsigned char));
	}
	else
	{
		std::cout << "Error saving Texture!" << std::endl;
	}
}

void  ETexture::FromSerialize(std::ifstream * stream)
{
	if (*stream)
	{
		freeAll();
		stream->read((char*)&width, sizeof(int));
		stream->read((char*)&height, sizeof(int));
		stream->read((char*)&bytesPerPixel, sizeof(int));

		int numPixels = width * height * bytesPerPixel;


		data = (unsigned char*)malloc(numPixels + 1);
		
			stream->read((char*)&data[0],  numPixels );
	
	}
	else
	{
		std::cout << "Error loading AABB!" << std::endl;
	}
}

void ETexture::fromTexture(ETexture &t)
{
	freeAll();
	unsigned char * pdata = t.getData();

	if (pdata)
	{
		this->bytesPerPixel = t.getBytesPerPixel();
		glm::ivec2 dim = t.getDimensions();
		this->width = dim.x;
		this->height = dim.y;

		int numPixels = width * height * bytesPerPixel;
		data = (unsigned char*)malloc(numPixels);
	data = (unsigned char*)memcpy(data, pdata, numPixels);
	std::cout << "LMAO" << std::endl;
	}
	else
	{
		std::cout << "Failed to load texture!!!" << std::endl;
	}
}

glm::ivec2  ETexture::getDimensions()
{
	return glm::ivec2(width, height);
}

void ETexture::freeData()
{
	if (data)
	{
		free(data);
		data = NULL;
	}
}

void ETexture::freeAll()
{
	freeData();
	if (textureID != 0)
		glDeleteTextures(1, &textureID);
}

unsigned char *  ETexture::getData()
{
	return data;
}

int ETexture::getBytesPerPixel()
{
	return bytesPerPixel;
}

unsigned int ETexture::getTextureID()
{
	assert(textureID != 0);
	return textureID;
}

void ETexture::genTexture()
{

	glGenTextures(1, &textureID);

	int nrComponents = this->bytesPerPixel;
	GLenum format;
	if (nrComponents == 1)
		format = GL_RED;
	else if (nrComponents == 3)
		format = GL_RGB;
	else if (nrComponents == 4)
		format = GL_RGBA;
	


	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

bool ETexture::readyToRender()
{
	return (textureID != 0);
}
