#define _CRT_SECURE_NO_DEPRECATE
#include "Cave.h"
#include <iostream>
#include <fstream>


// LEFT Vertices
const GLfloat lvertices[18] = {
	-2.0, -2.0, -2.0,
	-2.0, -2.0,  2.0,
	-2.0,  2.0,  2.0,
	-2.0,  2.0,  2.0,
	-2.0,  2.0, -2.0,
	-2.0, -2.0, -2.0
};

// RIGHT Vertices
const GLfloat rvertices[] = {
	-2.0,  2.0, -2.0,
	2.0,  2.0, -2.0,
	2.0, -2.0, -2.0,
	2.0, -2.0, -2.0,
	-2.0, -2.0, -2.0,
	-2.0,  2.0, -2.0
};

// BOTTOM Vertices
const GLfloat bvertices[] = {
	-2.0, -2.0, -2.0,
	2.0, -2.0, -2.0,
	2.0, -2.0,  2.0,
	2.0, -2.0,  2.0,
	-2.0, -2.0,  2.0,
	-2.0, -2.0, -2.0
};

// LEFT Texture Coordinates
const GLfloat luvs[] = {
	1.0f, 0.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,
};

// RIGHT Texture Coordinates
const GLfloat ruvs[] = {
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,
};

// BOTTOM Texture Coordinates
const GLfloat buvs[] = {
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	0.0f, 1.0f
};

// Constructor
Cave::Cave()
{
	initialize();
}

// Destructor
Cave::~Cave()
{
	// Delete previously generated buffers
	glDeleteVertexArrays(1, &lVAO);
	glDeleteVertexArrays(1, &rVAO);
	glDeleteVertexArrays(1, &bVAO);
	glDeleteBuffers(1, &lVBO);
	glDeleteBuffers(1, &rVBO);
	glDeleteBuffers(1, &bVBO);
	glDeleteBuffers(1, &luv_ID);
	glDeleteBuffers(1, &ruv_ID);
	glDeleteBuffers(1, &buv_ID);
}

// Initialize
void Cave::initialize() {

	toWorld = glm::mat4(1.0f);

	// LEFT
	glGenVertexArrays(1, &lVAO);
	glGenBuffers(1, &lVBO);
	glGenBuffers(1, &luv_ID);

	glBindVertexArray(lVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lvertices), &lvertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		3 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

	glBindBuffer(GL_ARRAY_BUFFER, luv_ID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(luvs), luvs, GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader.
		2, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		2 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// RIGHT
	glGenVertexArrays(1, &rVAO);
	glGenBuffers(1, &rVBO);
	glGenBuffers(1, &ruv_ID);

	glBindVertexArray(rVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rvertices), &rvertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		3 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

	glBindBuffer(GL_ARRAY_BUFFER, ruv_ID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ruvs), ruvs, GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader.
		2, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		2 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	// BOTTOM
	glGenVertexArrays(1, &bVAO);
	glGenBuffers(1, &bVBO);
	glGenBuffers(1, &buv_ID);

	glBindVertexArray(bVAO);
	glBindBuffer(GL_ARRAY_BUFFER, bVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bvertices), &bvertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		3 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

	glBindBuffer(GL_ARRAY_BUFFER, buv_ID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(buvs), buvs, GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader.
		2, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		2 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Load Texture
	this->loadTexture();
}

// Draw
void Cave::draw(GLuint shaderProgram, glm::mat4 Projection, glm::mat4 View, GLuint left, GLuint right, GLuint bottom)
{
	
	uProjection = glGetUniformLocation(shaderProgram, "projection");
	uModel = glGetUniformLocation(shaderProgram, "model");
	uView = glGetUniformLocation(shaderProgram, "view");
	// Now send these values to the shader program
	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &Projection[0][0]);
	glUniformMatrix4fv(uModel, 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(uView, 1, GL_FALSE, &toWorld[0][0]);

	glUniform1i(glGetUniformLocation(shaderProgram, "textureShader"), 0);

	// LEFT
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, left);
	glBindVertexArray(lVAO);
	glBindBuffer(GL_ARRAY_BUFFER, luv_ID);
	glDrawArrays(GL_TRIANGLES, 0, 2 * 3);

	// RIGHT
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, right);
	glBindVertexArray(rVAO);
	glBindBuffer(GL_ARRAY_BUFFER, ruv_ID);
	glDrawArrays(GL_TRIANGLES, 0, 2 * 3);

	// BOTTOM
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bottom);
	glBindVertexArray(bVAO);
	glBindBuffer(GL_ARRAY_BUFFER, buv_ID);
	glDrawArrays(GL_TRIANGLES, 0, 2 * 3);

	glBindVertexArray(0);
}

// Texture Loader
void Cave::loadTexture() {

	glGenTextures(1, &texture_ID);
	glBindTexture(GL_TEXTURE_2D, texture_ID);

	int width, height;
	unsigned char* image;

	// Load front
	image = loadPPM("./plain.ppm", width, height);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// Free image
	free(image);

	// Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Cave::useCubemap(int eyeIdx)
{
	if (eyeIdx == 0) {
		curTextureID = texture_ID_left;
	}
	else if (eyeIdx == 1) {
		curTextureID = texture_ID_right;
	}
	else curTextureID = texture_ID_self;
}

// PPM Loader
#pragma warning(disable : 4996)  
unsigned char* Cave::loadPPM(const char* filename, int& width, int& height)
{
	const int BUFSIZE = 128;
	FILE* fp;
	unsigned int read;
	unsigned char* rawData;
	char buf[3][BUFSIZE];
	char* retval_fgets;
	size_t retval_sscanf;

	if ((fp = fopen(filename, "rb")) == NULL)
	{
		std::cerr << "error reading ppm file, could not locate " << filename << std::endl;
		width = 0;
		height = 0;
		return 0;
	}

	// Read magic number:
	retval_fgets = fgets(buf[0], BUFSIZE, fp);

	// Read width and height:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');
	retval_sscanf = sscanf(buf[0], "%s %s", buf[1], buf[2]);
	width = atoi(buf[1]);
	height = atoi(buf[2]);

	// Read maxval:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');

	// Read image data:
	rawData = new unsigned char[width * height * 3];
	read = fread(rawData, width * height * 3, 1, fp);
	fclose(fp);
	if (read != 1)
	{
		std::cerr << "error parsing ppm file, incomplete data" << std::endl;
		delete[] rawData;
		width = 0;
		height = 0;

		return 0;
	}

	return rawData;
}
