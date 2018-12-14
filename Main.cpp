#include "GL\gl3w.h"
#include "GL\glcorearb.h"
#include <GLFW\glfw3.h>
#include <iostream>
#include "imgui\imgui.h"
#include "Helpers/imgui_impl_glfw_gl3.h"
#include "Shader/Shader.h"
#include "Helpers/GLCamera.h"
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Helpers/OpenFile.h"
#include "Model/Model.h"

struct Camera {
	glm::vec3	pos, dir, yAxis, xAxis;
	float	tanFovY, tanFovX;
};
struct Light
{
	glm::vec4 pos_dir;
	glm::vec4 color;
	glm::vec4 attenuation;
};

//Due to padding on compute shaders, all of these need to be vec4's
struct Triangle
{
	glm::vec4 vA, vB, vC;
	glm::vec4 nA, nB, nC;
	glm::vec4 tA, tB, tC;
};
typedef Triangle Sphere;
struct Material
{
	glm::vec4 diffuse;
	glm::vec4 specularity;
	glm::vec4 emission;
	float shininess;
};


//Shaders used for render
Shader compute;		//Compute for raytracing
Shader quadshader;	//Quad for rendering framebuffer created from compute
Shader modelshader;	 //Shader for rasterization.


//Uniform members for compute shader.
unsigned int numTriangles = 0;
unsigned int numTriangles2 = 0;
unsigned int numMeshes = 0;

//Framebuffer
unsigned int ftexture = 0;
//Lights to put on compute shader
std::vector<Light> lights;

//Reflection Depth for raytracing
unsigned int reflectionDepth;

//Materials for model & sphere.
Material meshMat, sphereMat;




float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	 // positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	1.0f, -1.0f,  1.0f, 0.0f,
	1.0f,  1.0f,  1.0f, 1.0f
};


bool shouldtrace = false;

//Window given to us by GLFW
GLFWwindow *  window;

//Position of the camera. 
glm::vec3 camerapos = { 0.f, 10.f, 20.0f };

//Models possible to be loaded.
Model model;

//Members used for mouse input.
double lastX = 0, lastY = 0;
bool firstMouse = true;

//Shader Storage Buffer Objects for mesh data & sphere
unsigned int ssbo;
unsigned int ssbo2;

//For Error Handling.
GLenum err;

//for rasterization and changing screen size.
glm::mat4 projection;

//Model Data processed for sending to compute shader.
std::vector<Triangle> mesh1;

//Sphere to render.
Sphere sphere;

//VAO and VBO for quad which is used to render to screen.
unsigned int quadVAO, quadVBO;

//Screen Dimentions
int SCREEN_WIDTH = 1024;
int SCREEN_HEIGHT = 720;

//Work group for compute shader
unsigned int workGroupSizeX, workGroupSizeY;

//Camera 
GLCamera camera;

//Deltatime for camera movement. 
float deltatime;

//Camera zooming 
float fov = 56;

void load_scene()
{
	std::ifstream file("test.scene");
	if (file)
	{
		std::string line;
		while (std::getline(file, line))
		{
			std::istringstream iss(line);

			std::string result;
			if (std::getline(iss, result, ' '))
			{
				if (result == "camera")
				{
					glm::vec3 pos;
					glm::vec3 dir;
					std::string token;
					std::getline(iss, token, ' ');
					pos.x = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					pos.y = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					pos.z = std::atof(token.c_str());

					std::getline(iss, token, ' ');

					std::getline(iss, token, ' ');
					dir.x = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					dir.y = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					dir.z = std::atof(token.c_str());
					
					camera.Position = pos;
					camera.Front = dir;
						
				}
				if (result == "light")
				{
					glm::vec3 pos, ambient, diff, spec;
					std::string token;

					std::getline(iss, token, ' ');
					pos.x = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					pos.y = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					pos.z = std::atof(token.c_str());

					std::getline(iss, token, ' ');

					std::getline(iss, token, ' ');
					ambient.x = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					ambient.y = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					ambient.z = std::atof(token.c_str());

					std::getline(iss, token, ' ');

					std::getline(iss, token, ' ');
					diff.x = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					diff.y = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					diff.z = std::atof(token.c_str());

					std::getline(iss, token, ' ');

					std::getline(iss, token, ' ');
					spec.x = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					spec.y = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					spec.z = std::atof(token.c_str());

					Light l;
					l.pos_dir = glm::vec4(pos, 1);
					l.color = glm::vec4(diff, 1);
					l.attenuation = glm::vec4(0.9, 0.8, 0.7, 1);
					lights.push_back(l);

				}

				if (result == "obj")
				{

					std::string directory;
					glm::vec3 pos, scale, rotation;
					glm::vec3 ambient, diff, spec;
					float shine;

					Material m;
					std::string token;

					std::getline(iss, token, ' ');
					directory = token;

					std::getline(iss, token, ' ');
					pos.x = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					pos.y = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					pos.z = std::atof(token.c_str());

					std::getline(iss, token, ' ');

					std::getline(iss, token, ' ');
					ambient.x = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					ambient.y = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					ambient.z = std::atof(token.c_str());

					std::getline(iss, token, ' ');

					std::getline(iss, token, ' ');
					diff.x = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					diff.y = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					diff.z = std::atof(token.c_str());

					std::getline(iss, token, ' ');

					std::getline(iss, token, ' ');
					spec.x = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					spec.y = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					spec.z = std::atof(token.c_str());

					std::getline(iss, token, ' ');

					std::getline(iss, token, ' ');
					shine = std::atof(token.c_str());
			
					model.Init(directory.c_str());
					model.position = pos;
					m.diffuse = glm::vec4(diff, 1);
					m.emission = glm::vec4(ambient, 1);
					m.specularity = glm::vec4(spec, 1);
					m.shininess = shine;

					meshMat = m;
				}
				if (result == "image")
				{
					std::string token;

					std::getline(iss, token, ' ');
					SCREEN_WIDTH = std::atoi(token.c_str());

					std::getline(iss, token, ' ');
					SCREEN_HEIGHT = std::atoi(token.c_str());

					glfwSetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
					glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
				}
				if (result == "sphere")
				{
				
					glm::vec3 pos, scale, rotation;
					float radius;
					glm::vec3 ambient, diff, spec;
					float shine;

					Material m;
					std::string token;


					std::getline(iss, token, ' ');
					pos.x = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					pos.y = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					pos.z = std::atof(token.c_str());

					std::getline(iss, token, ' ');

					std::getline(iss, token, ' ');
					radius = std::atof(token.c_str());

					std::getline(iss, token, ' ');

					std::getline(iss, token, ' ');
					ambient.x = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					ambient.y = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					ambient.z = std::atof(token.c_str());

					std::getline(iss, token, ' ');

					std::getline(iss, token, ' ');
					diff.x = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					diff.y = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					diff.z = std::atof(token.c_str());

					std::getline(iss, token, ' ');

					std::getline(iss, token, ' ');
					spec.x = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					spec.y = std::atof(token.c_str());

					std::getline(iss, token, ' ');
					spec.z = std::atof(token.c_str());

					std::getline(iss, token, ' ');

					std::getline(iss, token, ' ');
					shine = std::atof(token.c_str());

					sphere.vA = glm::vec4(pos, radius);
					m.diffuse = glm::vec4(diff, 1);
					m.emission = glm::vec4(ambient, 1);
					m.specularity = glm::vec4(spec, 1);
					m.shininess = shine;

					sphereMat = m;
				}
			}

		}
	}

	std::cout << " Done loading scene! " << std::endl;

}
void init_first_ssbo()
{
	//Init ssbo
	glGenBuffers(1, &ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
	Mesh * t = model.meshes[0];
	compute.use();
	numTriangles = t->indices.size() / 3;
	mesh1.resize(numTriangles);
	for (unsigned int i = 0; i < t->indices.size(); i += 3)
	{
		if (i + 3 <= t->indices.size())
		{
			unsigned int id = i / 3;
			unsigned int v0 = t->indices.at(i);
			unsigned int v1 = t->indices.at(i + 1);
			unsigned int v2 = t->indices.at(i + 2);

			mesh1[id].vA = glm::vec4(t->vertices.at(v0).Position + model.position, 1);
			mesh1[id].vB = glm::vec4(t->vertices.at(v1).Position + model.position, 1);
			mesh1[id].vC = glm::vec4(t->vertices.at(v2).Position + model.position, 1);


			mesh1[id].nA = glm::vec4(t->vertices.at(v0).Normal, 1);
			mesh1[id].nB = glm::vec4(t->vertices.at(v1).Normal, 1);
			mesh1[id].nC = glm::vec4(t->vertices.at(v2).Normal, 1);


			mesh1[id].tA = glm::vec4(t->vertices.at(v0).TexCoords, 1, 1);
			mesh1[id].tB = glm::vec4(t->vertices.at(v1).TexCoords, 1, 1);
			mesh1[id].tC = glm::vec4(t->vertices.at(v2).TexCoords, 1, 1);
		}
	};
	glBufferData(GL_SHADER_STORAGE_BUFFER, (numTriangles) * sizeof(Triangle), &mesh1[0], GL_STATIC_DRAW);
}

void init_second_ssbo()
{
	//Init ssbo
	glGenBuffers(1, &ssbo2);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo2);
	compute.use();
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Sphere), &sphere, GL_STATIC_DRAW);
}

void initFrameBuffer()
{
	//Generate framebuffer texture. 
	glGenTextures(1, &ftexture);
	glBindTexture(GL_TEXTURE_2D, ftexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	float* black = (float *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 32);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, black);
	glBindTexture(GL_TEXTURE_2D, 0);

}

void initQuadShader()
{
	quadshader.use();
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

}

void setLights(Shader shader)
{

	shader.setInt("numLights", lights.size());
	for (unsigned int i = 0; i < lights.size(); i++)
	{
		std::string t = "lights[" + std::to_string(i) + "]";
		shader.setVec4(t + ".dir", lights[i].pos_dir);
		shader.setVec4(t + ".color", lights[i].color);
		shader.setVec4(t + ".attenuation", lights[i].attenuation);
	}

}

void setMaterials(Shader shader)
{

	//Set material information for mesh and triangles.
	compute.setVec4("materials[" + std::to_string(0) +"].diffuse", meshMat.diffuse);
	compute.setVec4("materials[" + std::to_string(0) + "].specularity", meshMat.specularity);
	compute.setVec4("materials[" + std::to_string(0) + "].emission", meshMat.emission);
	compute.setFloat("materials[" + std::to_string(0) + "].shininess", meshMat.shininess);

	compute.setVec4("materials[" + std::to_string(1) + "].diffuse", sphereMat.diffuse);
	compute.setVec4("materials[" + std::to_string(1) + "].speculare", sphereMat.specularity);
	compute.setVec4("materials[" + std::to_string(1) + "].emission", sphereMat.emission);
	compute.setFloat("materials[" + std::to_string(1) + "].shininess", sphereMat.shininess);

}

Camera getComputeCamera(GLCamera cam)
{
	Camera c;
	c.pos = cam.Position;
	c.dir = glm::normalize(cam.Front);
	c.xAxis = -glm::normalize(glm::cross(camera.Up, c.dir));
	c.yAxis = -glm::cross(c.dir, c.xAxis);

	float realfov = glm::radians(fov);
	c.tanFovY = tanf((realfov) / 2.0f);
	c.tanFovX = (static_cast<float>(1080)*c.tanFovY) / static_cast<float>(720);
	return c;
}

void handleComputeUniforms()
{
	//Set Camera uniform
	Camera c = getComputeCamera(camera);
	compute.setVec3("camera.pos", c.pos);
	compute.setVec3("camera.dir", c.dir);
	compute.setVec3("camera.yAxis", -c.yAxis);
	compute.setVec3("camera.xAxis", c.xAxis);
	compute.setFloat("camera.tanFovY", c.tanFovY);
	compute.setFloat("camera.tanFovX", c.tanFovX);

	setLights(compute);
	setMaterials(compute);
	//Number of triangles in mesh.
	compute.setInt("mNumTriangles", numTriangles);

	//Reflection Depth of raycast
	compute.setUInt("reflectionDepth", 3);
	std::vector<Light> lights;
	unsigned int reflectionDepth;


}

//Callbacks
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow * window, int width, int height);
//Initializes OpenGL & GLFW Systems.
int initOpenGL();

int initOpenGL()
{
	//Init glfw
	glfwInit();
	//Set version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	//Creating window
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create window";
		return -1;
	}

	//Set context to current window
	glfwMakeContextCurrent(window);



	//Set event callback functions
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSwapInterval(0);
	

	//initialize GL3W function pointers.
	if (gl3wInit() != GL3W_OK) {

		std::cout << "Failed to initialize GL3W" << std::endl;
		return -1;
	}

	return 1;

}

void handleCamera()
{

	//Camera info.


	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltatime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltatime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltatime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltatime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		float velocity = camera.MovementSpeed * deltatime;
		camera.Position += glm::vec3(0.0f, 1.0f, 0.0f) * velocity;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		float velocity = camera.MovementSpeed * deltatime;
		camera.Position += glm::vec3(0.0f, -1.0f, 0.0f) * velocity;
	}
};

int nextPowerOfTwo(int x) {
	x--;
	x |= x >> 1; // handle 2 bit numbers
	x |= x >> 2; // handle 4 bit numbers
	x |= x >> 4; // handle 8 bit numbers
	x |= x >> 8; // handle 16 bit numbers
	x |= x >> 16; // handle 32 bit numbers
	x++;
	return x;
}

 void trace() {


	 compute.use();
	 handleComputeUniforms();
	/* Bind level 0 of framebuffer texture as writable image in the shader. */
	glBindImageTexture(0, ftexture, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		std::cout << "GL ERROR! " << err << std::endl;
	}

	/* Compute appropriate invocation dimension. */
	int worksizeX = nextPowerOfTwo(SCREEN_WIDTH);
	int worksizeY = nextPowerOfTwo(SCREEN_HEIGHT);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo2);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo2);
	glDispatchCompute(worksizeX / workGroupSizeX, worksizeY / workGroupSizeY, 1);



	while ((err = glGetError()) != GL_NO_ERROR)
	{
		std::cout << "GL ERROR! " << err << std::endl;
	}
	/* Reset image binding. */
	glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(0);
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		std::cout << "GL ERROR! " << err << std::endl;
	
	}
	/*
	* Draw the rendered image on the screen using textured full-screen
	* quad.
	*/

	quadshader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ftexture);
	glBindVertexArray(quadVAO);
	quadshader.setInt("screenTexture", 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		std::cout << "GL ERROR! " << err << std::endl;
	}
}

int main()
{


	
	//Initialize OpenGL
	initOpenGL();
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	load_scene();
	//Initialize shaders.
	compute.init("compute.shader");
	modelshader.init("animated.vsh", "animated.fsh");
	quadshader.init("quadrender.vshdr", "quadrender.fshdr");

	compute.use();

	//Get Work size from compute shader.
	GLint * size = new GLint[3];
	glGetProgramiv(compute.ID, GL_COMPUTE_WORK_GROUP_SIZE, size);
	workGroupSizeX = size[0];
	workGroupSizeY = size[2];

	init_first_ssbo();
	init_second_ssbo();

	initFrameBuffer();
	
	initQuadShader();


	//Enable Depth Testing. 
	glEnable(GL_DEPTH_TEST);











	

	deltatime = 0;
	Timer timer;
	float ticks = 0;
	timer.start();
	timer.unpause();


	while (!glfwWindowShouldClose(window))
	{
		float t = timer.getTicks();
		deltatime = (timer.getTicks() - ticks) / 1000.0f;
		ticks = timer.getTicks();

		handleCamera();

		//Clear buffer bits...
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//Build the model matrix using the position and euler angles.
		if (shouldtrace)
		{
			
			trace();
		}
		else
		{
			
			modelshader.use();
			projection = glm::perspective(glm::radians(fov), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 5000.0f);
			glm::mat4 modelmatrix(1);
			modelshader.setMat4("projection", projection);
			modelshader.setMat4("model", modelmatrix);
			modelshader.setMat4("view", camera.GetViewMatrix());
			model.Draw(modelshader);
		}


		//Swap Buffers.
		glfwSwapBuffers(window);
		glfwPollEvents();

	}



	//And cleanup.
	glfwTerminate();
	return 0;
}

 void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	 if (firstMouse)
	 {
		 lastX = xpos;
		 lastY = ypos;
		 firstMouse = false;
	 }

	 float xoffset = xpos - lastX;
	 float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	 lastX = xpos;
	 lastY = ypos;

	 camera.ProcessMouseMovement(xoffset, yoffset);

}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 200.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 200.0f)
		fov = 45.0f;
}


void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_F1)
	{

	}
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
	{
		shouldtrace = !shouldtrace;
	}
	else if (key == GLFW_KEY_ESCAPE)
	{
		//Close
		glfwSetWindowShouldClose(window, true);
	}


}


void framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	glViewport(0, 0, width, height);
}
