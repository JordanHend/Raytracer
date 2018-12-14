#ifndef RAY_H
#define RAY_H
#include <glm\glm.hpp>
struct Ray
{
	glm::vec3 ray_origin;
	glm::vec3 ray_direction;
};


Ray RayFromClick(int mouseX, int mouseY,             // Mouse position, in pixels, from bottom-left corner of the window
	int screenWidth, int screenHeight,  // Window size, in pixels
	glm::mat4 ViewMatrix,               // Camera position and orientation
	glm::mat4 ProjectionMatrix         // Camera parameters (ratio, field of view, near and far planes)
	);
#endif