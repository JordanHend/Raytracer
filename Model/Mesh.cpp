#include "Mesh.h"





Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	//this->indices.resize(indices.size() / 4);


	setupMesh();
}

void Mesh::reBuffer()
{

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);


	// Create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);


	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// Setting vertex attribute pointer.


	// Positions

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	// Normals

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	glEnableVertexAttribArray(1);
	// Texture coords


	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	glEnableVertexAttribArray(2);
	// Tangent

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	glEnableVertexAttribArray(3);
	// Bitangent


	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
	glEnableVertexAttribArray(4);
	// Bone ID


	glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)(offsetof(Vertex, boneID)));
	glEnableVertexAttribArray(5);

	// Bone Weight

	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, weight)));
	glEnableVertexAttribArray(6);




	glBindVertexArray(0);
}

void Mesh::setupMesh()
{




	// Create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);


	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// Setting vertex attribute pointer.


	// Positions

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	// Normals

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	glEnableVertexAttribArray(1);
	// Texture coords


	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	glEnableVertexAttribArray(2);
	// Tangent

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	glEnableVertexAttribArray(3);
	// Bitangent


	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
	glEnableVertexAttribArray(4);
	// Bone ID


	glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)(offsetof(Vertex, boneID)));
	glEnableVertexAttribArray(5);

	// Bone Weight

	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, weight)));
	glEnableVertexAttribArray(6);




	glBindVertexArray(0);

}

void Mesh::Draw(Shader shader)
{

	

	//Loop through textures, and bind them where appropriate.
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);

		std::string number;
		std::string name = "Texture_diffuse";
									
		glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
		// Then bind the texture.
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}

	//Bind VAO and draw
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		std::cout << "GL ERROR! " << err << std::endl;
	}
	// Set back to default
	glActiveTexture(GL_TEXTURE0);

}

void Mesh::DrawShadow(Shader shader)
{
	//Bind VAO and draw
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Mesh::Serialize(std::ofstream * stream)
{
	unsigned int vsize = vertices.size(), isize = indices.size(), tsize = textureData.size();
	if (stream)
	{

		stream->write((char*)&vsize, sizeof(unsigned int));
		stream->write((char*)&isize, sizeof(unsigned int));
		stream->write((char*)&tsize, sizeof(unsigned int));
		stream->write((char*)vertices.data(), sizeof(Vertex) * vsize);
		stream->write((char*)indices.data(), sizeof(unsigned int) * isize);
		for (unsigned int i = 0; i < textureData.size(); i++)
		{
			textureData[i].Serialize(stream);
		}
	}
}

void Mesh::FromSerialize(std::ifstream * stream)
{
	unsigned int vsize = 0, isize = 0, tsize = 0;
	if (stream)
	{

		vertices.clear();
		indices.clear();
		textures.clear();
		textureData.clear();
		stream->read((char*)&vsize, sizeof(unsigned int));
		stream->read((char*)&isize, sizeof(unsigned int));
		stream->read((char*)&tsize, sizeof(unsigned int));
		vertices.resize(vsize);
		indices.resize(isize);
		textureData.resize(tsize);
		textures.resize(tsize);
	
		stream->read((char*)&vertices[0], sizeof(Vertex) * vsize);
		stream->read((char*)&indices[0], sizeof(unsigned int) * isize);
		for (unsigned int i = 0; i < tsize; i++)
		{
			textureData[i].FromSerialize(stream);
			textureData[i].genTexture();
			textures[i].id = textureData[i].getTextureID();
		}
	
			reBuffer();
	
	}
}
