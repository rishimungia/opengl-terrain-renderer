#include <iostream>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <common/utils.hpp>
#include <common/controls.hpp>

//Variables
GLFWwindow* window;
static const int window_width = 1920;
static const int window_height = 1080;

static int n_points = 820; //minimum 2
static float m_scale = 5;

unsigned int nIndices;

GLuint programID;
GLuint skyboxProgramID;
GLuint billboardProgramID;

GLuint heightmapTextureID;
GLuint skyboxTextureID;
GLuint flowerTextureID;

// rock textures
GLuint rocksTextureID;
GLuint rocksRoughnessID;
GLuint rocksNormalmapID;

// grass textures
GLuint grassTextureID;
GLuint grassRoughnessID;
GLuint grassNormalmapID;

// snow textures
GLuint snowTextureID;
GLuint snowRoughnessID;
GLuint snowNormalmapID;

// VAO
GLuint VertexArrayID;
// Buffers for VAO
GLuint vertexbuffer;
GLuint uvbuffer;
GLuint normalbuffer;
GLuint elementbuffer;

std::vector<glm::vec3> terrainVertices;

glm::vec3 lightPos;
float heightScaler;

// Skybox variables
float skyboxVertices[] =
{
	//   Coordinates
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};

GLuint skyboxVAO, skyboxVBO, skyboxEBO;

bool initializeGL()
  {
	// Initialise GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_SAMPLES, 1); //no anti-aliasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(window_width, window_height, "OpenGLRenderer", NULL, NULL);

	if (window == NULL) {
		std::cerr << "Failed to open GLFW window. If you have an Intel GPU, they may not be 4.5 compatible." << std::endl;
			glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();
		return false;
	}

	//without
	/*int NumberOfExtensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &NumberOfExtensions);
	for (int i = 0; i < NumberOfExtensions; i++) {
		const GLubyte* ccc = glGetStringi(GL_EXTENSIONS, i);
		if (strcmp((char *)ccc, "GL_AMD_blend_minmax_factor") != 0) {
			std::cout << "Change sring in initializeGL() to:" << ccc << std::endl;
			return -1;
		}
	}*/

	//with
	if (!GLEW_ARB_debug_output) return false;

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwPollEvents();
	glfwSetCursorPos(window, window_width, window_height);
}

using namespace glm;
void LoadModel()
{
	//std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<unsigned int> indices;

	for (int i = 0; i < n_points; i++)
	{
		float x = (m_scale) * ((i / float(n_points - 1)) - 0.5f) * 2.0f;
		for (int j = 0; j < n_points; j++)
		{
			float z = (m_scale) * ((j / float(n_points - 1)) - 0.5f) * 2.0f;
			terrainVertices.push_back(vec3(x, std::rand() / ((RAND_MAX + 1u) / 1), z));
			uvs.push_back(vec2(float(i + 0.5f) / float(n_points - 1), float(j + 0.5f) / float(n_points - 1)));
		}
	}

	glEnable(GL_PRIMITIVE_RESTART);
	constexpr unsigned int restartIndex = std::numeric_limits<std::uint32_t>::max();
	glPrimitiveRestartIndex(restartIndex);

	int n = 0;
	for (int i = 0; i < n_points - 1; i++)
	{
		for (int j = 0; j < n_points; j++)
		{
			unsigned int topLeft = n;
			unsigned int bottomLeft = topLeft + n_points;
			indices.push_back(bottomLeft);
			indices.push_back(topLeft);
			n++;
		}
		indices.push_back(restartIndex);
	}

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glEnableVertexAttribArray(0);
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, terrainVertices.size() * sizeof(glm::vec3), &terrainVertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(
		0, // attribute
		3, // size (we have x,y,z)
		GL_FLOAT, // type of each individual element
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Generate a buffer for the indices as well
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	nIndices = indices.size();
}

void LoadTextures () {
	int width, height;
	unsigned char* data = nullptr;

	// load heightmap
	loadBMP_custom("mountains_height.bmp", width, height, data);

	glGenTextures(1, &heightmapTextureID);
	glBindTexture(GL_TEXTURE_2D, heightmapTextureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);

	// load rocks texture
	loadBMP_custom("rocks.bmp", width, height, data);

	glGenTextures(1, &rocksTextureID);
	glBindTexture(GL_TEXTURE_2D, rocksTextureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);
	
	// load rock roughness texture
	loadBMP_custom("rocks-r.bmp", width, height, data);

	glGenTextures(1, &rocksRoughnessID);
	glBindTexture(GL_TEXTURE_2D, rocksRoughnessID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);
	
	// load rock normal map
	loadBMP_custom("rocks-n.bmp", width, height, data);

	glGenTextures(1, &rocksNormalmapID);
	glBindTexture(GL_TEXTURE_2D, rocksNormalmapID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);
	
	// load grass texture
	loadBMP_custom("grass.bmp", width, height, data);

	glGenTextures(1, &grassTextureID);
	glBindTexture(GL_TEXTURE_2D, grassTextureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);
	
	// load grass roughness texture
	loadBMP_custom("grass-r.bmp", width, height, data);

	glGenTextures(1, &grassRoughnessID);
	glBindTexture(GL_TEXTURE_2D, grassRoughnessID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);
	
	// load grass normal map
	loadBMP_custom("grass-n.bmp", width, height, data);

	glGenTextures(1, &grassNormalmapID);
	glBindTexture(GL_TEXTURE_2D, grassNormalmapID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);
	
	// load snow texture
	loadBMP_custom("snow.bmp", width, height, data);

	glGenTextures(1, &snowTextureID);
	glBindTexture(GL_TEXTURE_2D, snowTextureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);
	
	// load snow roughness texture
	loadBMP_custom("snow-r.bmp", width, height, data);

	glGenTextures(1, &snowRoughnessID);
	glBindTexture(GL_TEXTURE_2D, snowRoughnessID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);
	
	// load snow normal map
	loadBMP_custom("snow-n.bmp", width, height, data);

	glGenTextures(1, &snowNormalmapID);
	glBindTexture(GL_TEXTURE_2D, snowNormalmapID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);
}

bool readAndCompileShader(const char* shader_path, const GLuint& id) {
	std::string shaderCode;
	std::ifstream shaderStream(shader_path, std::ios::in);
	if (shaderStream.is_open()) {
		std::stringstream sstr;
		sstr << shaderStream.rdbuf();
		shaderCode = sstr.str();
		shaderStream.close();
	}
	else {
		std::cout << "Impossible to open " << shader_path << ". Are you in the right directory ? " << std::endl;
			return false;
	}

	std::cout << "Compiling shader :" << shader_path << std::endl;
	char const* sourcePointer = shaderCode.c_str();
	glShaderSource(id, 1, &sourcePointer, NULL);
	glCompileShader(id);

	GLint Result = GL_FALSE;
	int InfoLogLength;
	glGetShaderiv(id, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> shaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(id, InfoLogLength, NULL, &shaderErrorMessage[0]);
		std::cout << &shaderErrorMessage[0] << std::endl;
	}
	std::cout << "Compilation of Shader: " << shader_path << " " << (Result == GL_TRUE ? "Success" : "Failed!") << std::endl;
	return Result == 1;
}

void LoadShaders(GLuint& program, const char* vertex_file_path, const char* fragment_file_path)
{
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	bool vok = readAndCompileShader(vertex_file_path, VertexShaderID);
	bool fok = readAndCompileShader(fragment_file_path, FragmentShaderID);

	if (vok && fok) {
		GLint Result = GL_FALSE;
		int InfoLogLength;
		std::cout << "Linking program" << std::endl;
		program = glCreateProgram();
		glAttachShader(program, VertexShaderID);
		glAttachShader(program, FragmentShaderID);
		glLinkProgram(program);
		glGetProgramiv(program, GL_LINK_STATUS, &Result);
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(program, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			std::cout << &ProgramErrorMessage[0];
		}
		std::cout << "Linking program: " << (Result == GL_TRUE ? "Success" : "Failed!") << std::endl;
	} else {
		std::cout << "Program will not be linked: one of the shaders has an error" << std::endl;
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
}

void LoadShaders(GLuint& program, const char* vertex_file_path, const char* fragment_file_path, const char* geometry_file_path) {
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);

	bool vok = readAndCompileShader(vertex_file_path, VertexShaderID);
	bool fok = readAndCompileShader(fragment_file_path, FragmentShaderID);
	bool gok = readAndCompileShader(geometry_file_path, GeometryShaderID);

	if (vok && fok && gok) {
		GLint Result = GL_FALSE;
		int InfoLogLength;
		std::cout << "Linking program" << std::endl;
		program = glCreateProgram();
		glAttachShader(program, VertexShaderID);
		glAttachShader(program, FragmentShaderID);
		glAttachShader(program, GeometryShaderID);
		glLinkProgram(program);
		glGetProgramiv(program, GL_LINK_STATUS, &Result);
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(program, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			std::cout << &ProgramErrorMessage[0];
		}
		std::cout << "Linking program: " << (Result == GL_TRUE ? "Success" : "Failed!") << std::endl;
	}
	else {
		std::cout << "Program will not be linked: one of the shaders has an error" << std::endl;
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
	glDeleteShader(GeometryShaderID);
}

void UnloadModel()
{
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);

	// skybox
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteBuffers(1, &skyboxEBO);
	glDeleteVertexArrays(1, &skyboxVAO);
}

void UnloadTextures()
{
	// terrain
	glDeleteTextures(1, &heightmapTextureID);
	
	glDeleteTextures(1, &rocksTextureID);
	glDeleteTextures(1, &rocksRoughnessID);
	glDeleteTextures(1, &rocksNormalmapID);
	
	glDeleteTextures(1, &grassTextureID);
	glDeleteTextures(1, &grassRoughnessID);
	glDeleteTextures(1, &grassNormalmapID);
	
	glDeleteTextures(1, &snowTextureID);
	glDeleteTextures(1, &snowRoughnessID);
	glDeleteTextures(1, &snowNormalmapID);

	// skybox
	glDeleteTextures(1, &skyboxTextureID);

	// billboard
	glDeleteTextures(1, &flowerTextureID);
}

void UnloadShaders() {
	glDeleteProgram(programID);
	glDeleteProgram(skyboxProgramID);
	glDeleteProgram(billboardProgramID);
}

void LoadSkybox() {
	// Create VAO, VBO, and EBO for the skybox
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	std::string facesCubemap[6] =
	{
		"skybox/right.bmp",
		"skybox/left.bmp",
		"skybox/top.bmp",
		"skybox/bottom.bmp",
		"skybox/front.bmp",
		"skybox/back.bmp"
	};

	// Creates the cubemap texture object
	glGenTextures(1, &skyboxTextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// These are very important to prevent seams
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	// Cycles through all the textures and attaches them to the cubemap object
	for (unsigned int i = 0; i < 6; i++)
	{
		int width, height;
		unsigned char* data;
		loadBMP_custom(facesCubemap[i].c_str(), width, height, data);
		if (data)
		{
			glTexImage2D
			(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,
				0,
				GL_BGR,
				GL_UNSIGNED_BYTE,
				data
			);
			delete[] data;
		}
		else
		{
			std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
			delete[] data;
		}
	}
}

void LoadBillboard() {
	// load texture
	glGenTextures(1, &flowerTextureID);
	glBindTexture(GL_TEXTURE_2D, flowerTextureID);

	int width, height;
	unsigned char* data;
	loadBMP_custom("sun_flower.bmp", width, height, data);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
}

int main() {
	if (!initializeGL()) return -1;

	LoadSkybox();
	LoadTextures();
	LoadModel();
	LoadBillboard();

	programID = glCreateProgram();
	LoadShaders(programID, "Basic.vert", "Texture.frag");

	skyboxProgramID = glCreateProgram();
	LoadShaders(skyboxProgramID, "Skybox.vert", "Skybox.frag");

	billboardProgramID = glCreateProgram();
	LoadShaders(billboardProgramID, "Billboard.vert", "Billboard.frag", "Billboard.geom");

	glClearColor(0.7f, 0.8f, 1.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	lightPos = glm::vec3(0, -0.5, -0.5);
	heightScaler = 1.0f;

	do {
		// reload shaders
		bool canReload = false; // to prevent multiple reloads
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) canReload = true;
		if (canReload && glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
			UnloadShaders();
			LoadShaders(programID, "Basic.vert", "Texture.frag");
			LoadShaders(skyboxProgramID, "Skybox.vert", "Skybox.frag");
			LoadShaders(billboardProgramID, "Billboard.vert", "Billboard.frag", "Billboard.geom");
			std::cout << "Shaders Reloaded!" << std::endl;
			canReload = false;
		}

		// modify terrain scale
		if (heightScaler < 1.0f && glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
			heightScaler += 0.01;
		}
		if (heightScaler > 0 && glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
			heightScaler -= 0.01;
		}

		// light rotation
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			glm::mat4 rotation = glm::rotate(glm::mat4(1.f), 0.01f, glm::vec3(0, 1, 0));
			lightPos = (vec3)normalize((rotation * vec4(lightPos, 0)));
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			glm::mat4 rotation = glm::rotate(glm::mat4(1.f), -0.01f, glm::vec3(0, 1, 0));
			lightPos = (vec3)normalize((rotation * vec4(lightPos, 0)));
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			glm::mat4 rotation = glm::rotate(glm::mat4(1.f), 0.01f, glm::vec3(0, 0, 1));
			lightPos = (vec3)normalize((rotation * vec4(lightPos, 0)));
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			glm::mat4 rotation = glm::rotate(glm::mat4(1.f), -0.01f, glm::vec3(0, 0, 1));
			lightPos = (vec3)normalize((rotation * vec4(lightPos, 0)));
		}
		
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glm::mat4 SkyboxViewMatrix = getSkyboxViewMatrix();

		// #### Base Terrain Pass ####
		glDepthFunc(GL_LESS);
		glCullFace(GL_BACK);

		glUseProgram(programID);

		// Get a handle for our uniforms
		GLuint MatrixID = glGetUniformLocation(programID, "MVP");
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// set view matrix
		GLuint viewMatrixID = glGetUniformLocation(programID, "viewMatrix");
		glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		// set model matrix
		GLuint modelMatrixID = glGetUniformLocation(programID, "modelMatrix");
		glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		// set nPoints uniform
		GLuint nPointsID = glGetUniformLocation(programID, "nPoints");
		glUniform1i(nPointsID, n_points);
		
		// set max height uniform
		GLuint heightScalerID = glGetUniformLocation(programID, "heightScaler");
		glUniform1f(heightScalerID, heightScaler);
		
		// set light position uniform
		GLuint lightPosID = glGetUniformLocation(programID, "lightPosition");
		glUniform3f(lightPosID, lightPos.x, lightPos.y, lightPos.z);

		// load terrain vertices
		glBindVertexArray(VertexArrayID);
		
		// activate heightmap texture
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, heightmapTextureID);
		glUniform1i(glGetUniformLocation(programID, "heightmapTexture"), 0);

		// activate rocks texture
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, rocksTextureID);
		glUniform1i(glGetUniformLocation(programID, "rockTexture"), 1);
		// roughness
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, rocksRoughnessID);
		glUniform1i(glGetUniformLocation(programID, "rockRoughness"), 2);
		// normal map
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_2D, rocksNormalmapID);
		glUniform1i(glGetUniformLocation(programID, "rockRoughness"), 3);
		
		// activate grass texture
		glActiveTexture(GL_TEXTURE0 + 4);
		glBindTexture(GL_TEXTURE_2D, grassTextureID);
		glUniform1i(glGetUniformLocation(programID, "grassTexture"), 4);
		// roughness
		glActiveTexture(GL_TEXTURE0 + 5);
		glBindTexture(GL_TEXTURE_2D, grassRoughnessID);
		glUniform1i(glGetUniformLocation(programID, "grassRoughness"), 5);
		// normal map
		glActiveTexture(GL_TEXTURE0 + 6);
		glBindTexture(GL_TEXTURE_2D, grassNormalmapID);
		glUniform1i(glGetUniformLocation(programID, "grassRoughness"), 6);
		
		// activate snow texture
		glActiveTexture(GL_TEXTURE0 + 7);
		glBindTexture(GL_TEXTURE_2D, snowTextureID);
		glUniform1i(glGetUniformLocation(programID, "snowTexture"), 7);
		// roughness
		glActiveTexture(GL_TEXTURE0 + 8);
		glBindTexture(GL_TEXTURE_2D, snowRoughnessID);
		glUniform1i(glGetUniformLocation(programID, "snowRoughness"), 8);
		// normal map
		glActiveTexture(GL_TEXTURE0 + 9);
		glBindTexture(GL_TEXTURE_2D, snowNormalmapID);
		glUniform1i(glGetUniformLocation(programID, "snowRoughness"), 9);

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glDrawElements(
			GL_TRIANGLE_STRIP, // mode
			(GLsizei)nIndices, // count
			GL_UNSIGNED_INT, // type
			(void*)0 // element array buffer offset
		);

		// #### Billboard Pass ####
		glm::mat4 gVP = ProjectionMatrix * ViewMatrix;

		glUseProgram(billboardProgramID);

		GLuint BillboardMatrixID = glGetUniformLocation(billboardProgramID, "gVP");
		glUniformMatrix4fv(BillboardMatrixID, 1, GL_FALSE, &gVP[0][0]);

		GLuint billBoardHeightScalerID = glGetUniformLocation(billboardProgramID, "heightScaler");
		glUniform1f(billBoardHeightScalerID, heightScaler);
		
		GLuint billboardRandomID = glGetUniformLocation(billboardProgramID, "random");
		glUniform1f(billboardRandomID, terrainVertices[0].y);

		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, heightmapTextureID);
		glUniform1i(glGetUniformLocation(billboardProgramID, "heightmapTexture"), 0);

		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, flowerTextureID);
		glUniform1i(glGetUniformLocation(billboardProgramID, "billboardTexture"), 1);

		glBindVertexArray(VertexArrayID);

		glDrawArrays(GL_POINTS, 0, nIndices);

		// #### Skybox Pass ####
		glDepthFunc(GL_LEQUAL);
		glCullFace(GL_FRONT);

		glUseProgram(skyboxProgramID);

		glUniformMatrix4fv(glGetUniformLocation(skyboxProgramID, "view"), 1, GL_FALSE, &SkyboxViewMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(skyboxProgramID, "projection"), 1, GL_FALSE, &ProjectionMatrix[0][0]);

		glBindVertexArray(skyboxVAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
		glUniform1i(glGetUniformLocation(skyboxProgramID, "skybox"), 0);

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	UnloadModel();
	UnloadShaders();
	UnloadTextures();
	glfwTerminate();

	return 0;
}