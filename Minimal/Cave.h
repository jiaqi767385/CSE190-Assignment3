#ifndef _CAVE_H
#define _CAVE_H

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
// Use of degrees is deprecated. Use radians instead.
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Cave
{
public:
	Cave();
	~Cave();

	glm::mat4 toWorld;

	void initialize();
	void draw(GLuint shaderProgram, glm::mat4 Projection, glm::mat4 View, GLuint left, GLuint right, GLuint bottom);

	// PPM Loader
	unsigned char* loadPPM(const char* filename, int& width, int& height);

	// Texture Loader
	void loadTexture();
	void useCubemap(int eyeIdx);

	// These variables are needed for the shader program

	// GLuint VBO, VAO, uv_ID for LEfT, RIGHT, BOTTOM squares
	GLuint lVBO, lVAO, luv_ID;
	GLuint rVBO, rVAO, ruv_ID;
	GLuint bVBO, bVAO, buv_ID;

	GLuint uProjection, uModel, uView;
	GLuint texture_ID_left, texture_ID_right, texture_ID_self;
	GLuint texture_ID, curTextureID;
};

#endif