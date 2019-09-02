/*********************************************************
FILE : main.cpp (csci3260 2018-2019 Assignment 2)
*********************************************************/
/*********************************************************
Student Information
Student ID: 1155072307
Student Name: Mega Gunawan
*********************************************************/

#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include "Dependencies\glm\glm.hpp"
#include "Dependencies\glm\gtc\matrix_transform.hpp"
#include "Dependencies\glm\gtc\type_ptr.hpp"
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;
using glm::vec2;
using glm::vec3;
using glm::mat4;
using glm::radians;

GLint programID;
// Could define the Vao&Vbo and interaction parameter here
GLuint WIDTH = 500;
GLuint HEIGHT = 500;

GLuint vaoJeep, vboJeep, uvJeep, normalJeep, drawSizeJeep, textureJeep, specularJeep;
GLuint vaoBlock, vboBlock, uvBlock, normalBlock, drawSizeBlock, textureBlock, specularBlock;
GLuint vaoPlane, vboPlane, uvPlane, normalPlane, drawSizePlane, texturePlane, specularPlane;

glm::vec3 cameraPos = glm::vec3(0.0f, 12.0f, 20.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
GLfloat cameraSpeed = 1;

GLint xAxis = 0, yAxis = 0, zAxis = 0;
GLfloat x_delta = 0.01f, y_delta = 0.01f, z_delta = 0.01f;
GLfloat scaleFactor = 1, deltaScale = 1.005f;

GLint rotation = 0;
GLfloat deltaAngle = 0.01f;

glm::vec3 carFront = glm::vec3(0.0f, 0.0f, -1.0f);
GLfloat carDx = 0;
GLfloat carDy = 0;
GLfloat carDz = 0;

GLfloat jeepYaw = -90.0f;
GLfloat jeepPitch = 0.0f;
GLfloat jeepSpeed = 0.1f;

GLfloat deltaCameraTime = 0.0f;
GLfloat lastCameraFrame = 0.0f;

GLfloat mouseControl_x_sensitivity = 2.0f;
GLfloat mouseControl_y_sensitivity = 2.0f;
GLfloat camera_sensitivity = 0.0f;
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool firstMouse = true;

GLfloat yaw = -90.0f;
GLfloat pitch = -20.0f;

GLfloat fov = radians(45.0f);

bool keys[1024] = { false };
bool special_keys[1024] = { false };

glm::vec3 pointLightPositions[] = {
	glm::vec3(0.7f,  0.2f,  0.0f),
	glm::vec3(2.3f, 1.0f, -0.0f),
	glm::vec3(-4.0f,  2.0f, 0.0f),
	glm::vec3(-18.0f,  1.0f, -8.0f)
};

GLfloat sLightIntensityFactor = 1, sLightIntensityDeltaFactor = 1.005f;
GLfloat dLightIntensityFactor = 1, dLightIntensityDeltaFactor = 1.005f;

vec3 spotDLightColor = vec3(0.8f, 0.3f, 0.8f);
vec3 spotSLightColor = vec3(0.8f, 0.3f, 0.8f);

vec3 pointDLightColor = vec3(0.9f, 0.9f, 0.9f);
vec3 pointSLightColor = vec3(1.0f, 1.0f, 1.0f);

vec3 directionDLightColor = vec3(0.6f, 0.6f, 0.6f);
vec3 directionSLightColor = vec3(0.5f, 0.5f, 0.5f);

GLint selfRotation = 0;

GLboolean wireframeMode = false;

GLboolean freeCameraMode = false;

GLboolean autoRotate = true;
GLint autoRotateSpeed = 1;

GLulong autoRotateCount = 0;


GLboolean shadows = true;

//time for calculating FPS
int lastTime = 0;
int deltaTime = 0;
int deltaFrame = 0;

//a series utilities for setting shader parameters 
void setMat4(const std::string &name, glm::mat4& value)
{
	unsigned int transformLoc = glGetUniformLocation(programID, name.c_str());
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(value));
}

void setVec4(const std::string &name, glm::vec4 value)
{
	glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void setVec3(const std::string &name, glm::vec3 value)
{
	glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void setFloat(const std::string &name, float value)
{
	glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}
void setInt(const std::string &name, int value)
{
	glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		cout << buffer << endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID)
{
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID)
{
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

string readShaderCode(const char* fileName)
{
	ifstream meInput(fileName);
	if (!meInput.good())
	{
		cout << "File failed to load..." << fileName;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))
		return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(programID);
}

void keyboard(unsigned char key, int x, int y)
{
	//TODO: Use keyboard to do interactive events and animation
	if (key >= 0 && key < 1024)
	{
		keys[key] = true;
	}
}

void move(int key, int x, int y) 
{
	//TODO: Use arrow keys to do interactive events and animation
	if (special_keys[GLUT_KEY_UP])
	{
		//zAxis -= 1;
		carDx += glm::cos(radians(jeepYaw));
		carDz += -glm::sin(radians(jeepYaw));
	}
	if (special_keys[GLUT_KEY_DOWN])
	{
		//zAxis += 1;
		carDx -= glm::cos(radians(jeepYaw));
		carDz -= -glm::sin(radians(jeepYaw));
	}
	if (special_keys[GLUT_KEY_LEFT])
	{
		//xAxis -= 1;
		selfRotation++;
	}
	if (special_keys[GLUT_KEY_RIGHT])
	{
		//xAxis += 1;
		selfRotation--;
	}
	if (keys['q'])
	{

		spotDLightColor *= dLightIntensityDeltaFactor;
		pointDLightColor *= dLightIntensityDeltaFactor;
		directionDLightColor *= dLightIntensityDeltaFactor;
		//keys['q'] = false;
	}
	if (keys['w'])
	{
		spotDLightColor /= dLightIntensityDeltaFactor;
		pointDLightColor /= dLightIntensityDeltaFactor;
		directionDLightColor /= dLightIntensityDeltaFactor;

		//keys['w'] = false;
	}
	if (keys['z'])
	{
		spotSLightColor *= sLightIntensityDeltaFactor;
		pointSLightColor *= sLightIntensityDeltaFactor;
		directionSLightColor *= sLightIntensityDeltaFactor;
		//keys['z'] = false;
	}
	if (keys['x'])
	{
		spotSLightColor /= sLightIntensityDeltaFactor;
		pointSLightColor /= sLightIntensityDeltaFactor;
		directionSLightColor /= sLightIntensityDeltaFactor;
		//keys['x'] = false;
	}
}

void PassiveMouse(int x, int y)
{
	//TODO: Use Mouse to do interactive events and animation
	if (firstMouse)
	{
		lastX = (GLfloat)x;
		lastY = (GLfloat)y;
		firstMouse = false;
	}


	//TODO: Use Mouse to do interactive events and animation
	if (!freeCameraMode)
	{
		/*		if (firstMouse)
		{
		lastX = (GLfloat)WIDTH/2;
		lastY = (GLfloat)HEIGHT/2;
		firstMouse = false;
		}*/
		GLfloat xoffset = x - lastX;
		GLfloat yoffset = lastY - y;
		//xoffset *= camera_sensitivity;
		yoffset *= mouseControl_y_sensitivity - 10;
		yoffset /= HEIGHT;
		cameraPos.y -= yoffset;
		xoffset *= mouseControl_x_sensitivity;
		xoffset /= WIDTH;
		cameraPos.x -= xoffset;

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);

	}
	lastX = (GLfloat)x;
	lastY = (GLfloat)y;
}

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 6 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; 
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i<vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}

	return true;
}

GLuint loadBMP_custom(const char * imagepath) {

	printf("Reading image %s\n", imagepath);

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	unsigned char * data;

	FILE * file = fopen(imagepath, "rb");
	if (!file) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0; }

	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	if (imageSize == 0)    imageSize = width*height * 3; 
	if (dataPos == 0)      dataPos = 54; 

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);

	
	GLuint textureID;
	//TODO: Create one OpenGL texture and set the texture parameter 
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	delete[] data;
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

void bindJeep()
{
	std::vector<vec3> vertices;
	std::vector<vec2> uvs;
	std::vector<vec3> normals;
	bool res = loadOBJ("jeep.obj", vertices, uvs, normals);
	glGenVertexArrays(1, &vaoJeep);
	glBindVertexArray(vaoJeep);

	glGenBuffers(1, &vboJeep);
	glBindBuffer(GL_ARRAY_BUFFER, vboJeep);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvJeep);
	glBindBuffer(GL_ARRAY_BUFFER, uvJeep);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalJeep);
	glBindBuffer(GL_ARRAY_BUFFER, normalJeep);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	drawSizeJeep = vertices.size();

	glBindBuffer(GL_ARRAY_BUFFER, vboJeep);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, uvJeep);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, normalJeep);
	glVertexAttribPointer(
		2,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

void bindBlock()
{
	std::vector<vec3> vertices;
	std::vector<vec2> uvs;
	std::vector<vec3> normals;
	bool res = loadOBJ("block.obj", vertices, uvs, normals);
	glGenVertexArrays(1, &vaoBlock);
	glBindVertexArray(vaoBlock);

	glGenBuffers(1, &vboBlock);
	glBindBuffer(GL_ARRAY_BUFFER, vboBlock);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvBlock);
	glBindBuffer(GL_ARRAY_BUFFER, uvBlock);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalBlock);
	glBindBuffer(GL_ARRAY_BUFFER, normalBlock);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	drawSizeBlock = vertices.size();

	glBindBuffer(GL_ARRAY_BUFFER, vboBlock);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, uvBlock);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, normalBlock);
	glVertexAttribPointer(
		2,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

void bindPlane()
{
	std::vector<vec3> vertices;
	std::vector<vec2> uvs;
	std::vector<vec3> normals;
	bool res = loadOBJ("plane.obj", vertices, uvs, normals);
	glGenVertexArrays(1, &vaoPlane);
	glBindVertexArray(vaoPlane);

	glGenBuffers(1, &vboPlane);
	glBindBuffer(GL_ARRAY_BUFFER, vboPlane);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvPlane);
	glBindBuffer(GL_ARRAY_BUFFER, uvPlane);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalPlane);
	glBindBuffer(GL_ARRAY_BUFFER, normalPlane);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	drawSizeBlock = vertices.size();

	glBindBuffer(GL_ARRAY_BUFFER, vboPlane);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, uvPlane);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, normalPlane);
	glVertexAttribPointer(
		2,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

void sendDataToOpenGL()
{
	//TODO:
	//Load objects and bind to VAO & VBO
	//Load texture
	bindJeep();
	bindBlock();
	bindPlane();

	textureJeep = loadBMP_custom("jeep_texture.bmp");
	textureBlock = loadBMP_custom("block_texture.bmp");
	texturePlane = loadBMP_custom("theme1.bmp");
}

void drawJeep()
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	projection = glm::perspective(fov, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

	model = glm::translate(model, glm::vec3(x_delta * xAxis, y_delta * yAxis, z_delta*zAxis));
	glm::vec3 front;
	jeepYaw = glm::degrees(deltaAngle*(rotation + autoRotateCount*0.2f*autoRotateSpeed + selfRotation));
	front.x = cos(glm::radians(jeepYaw))*cos(glm::radians(jeepPitch));
	front.y = sin(glm::radians(jeepPitch));
	front.z = sin(glm::radians(jeepYaw))*cos(glm::radians(jeepPitch));
	carFront = glm::normalize(front);

	model = glm::translate(model, glm::vec3(x_delta*xAxis + (GLfloat)jeepSpeed*carDx, y_delta*yAxis + (GLfloat)jeepSpeed*carDy, z_delta*zAxis + (GLfloat)jeepSpeed*carDz));
	model = glm::rotate(model, deltaAngle*(rotation + autoRotateCount*0.2f*autoRotateSpeed + selfRotation), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, scaleFactor*glm::vec3(1.0f));

	GLint modelLoc = glGetUniformLocation(programID, "model");
	GLint viewLoc = glGetUniformLocation(programID, "view");
	GLint projLoc = glGetUniformLocation(programID, "projection");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	GLint viewPosLoc = glGetUniformLocation(programID, "viewPos");
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
	// Set material properties
	glUniform1f(glGetUniformLocation(programID, "material.shininess"), 32.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureJeep);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureJeep);

	glBindVertexArray(vaoJeep);
	glDrawArrays(GL_TRIANGLES, 0, drawSizeJeep);
	glBindVertexArray(0);
}

void drawBlock()
{

}

void drawPlane()
{

}

void paintGL(void)
{
	glClearColor(0.4f, 0.6f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//TODO:
	//Set lighting information, such as position and color of lighting source
	//Set transformation matrix
	//Bind different textures

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	GLfloat currentFrame = (GLfloat)glutGet(GLUT_ELAPSED_TIME);
	deltaCameraTime = currentFrame - lastCameraFrame;
	lastCameraFrame = currentFrame;

	cameraSpeed = 0.0f*deltaCameraTime;

	//lighting
	/*glUniform3f(glGetUniformLocation(programID, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
	glUniform3f(glGetUniformLocation(programID, "dirLight.ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(programID, "dirLight.diffuse"), directionDLightColor.x, directionDLightColor.y, directionDLightColor.z);
	glUniform3f(glGetUniformLocation(programID, "dirLight.specular"), directionSLightColor.x, directionSLightColor.y, directionSLightColor.z);
	// Point light 1
	glUniform3f(glGetUniformLocation(programID, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[0].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(programID, "pointLights[0].diffuse"), pointDLightColor.x, pointDLightColor.y, pointDLightColor.z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[0].specular"), pointSLightColor.x, pointSLightColor.y, pointSLightColor.z);
	glUniform1f(glGetUniformLocation(programID, "pointLights[0].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[0].linear"), 0.09);
	glUniform1f(glGetUniformLocation(programID, "pointLights[0].quadratic"), 0.032); */
	
	drawJeep();
	//drawBlock();
	//drawPlane();

	glFlush();
	glutPostRedisplay();
}

void initializedGL(void) //run only once
{
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glutSetOption(GLUT_MULTISAMPLE, 4);
	installShaders();
	sendDataToOpenGL();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE | GLUT_MULTISAMPLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Assignment 2");
	
	//TODO:
	/*Register different CALLBACK function for GLUT to response
	with different events, e.g. window sizing, mouse click or
	keyboard stroke */
	initializedGL();
	glutDisplayFunc(paintGL);
	
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(move);
	glutPassiveMotionFunc(PassiveMouse);
	
	glutMainLoop();

	return 0;
}