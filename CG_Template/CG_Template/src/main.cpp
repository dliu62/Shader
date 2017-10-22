
#include <Windows.h> //for output debug string


#include <iostream>
#include <string>
#include <sstream>

//GLEW
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>

//Freeglut
#include <GL/freeglut.h>

//Cg
#include <Cg/cgGL.h>

//Math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define MEMBER_OFFSET(s,m) ((char*)NULL + (offsetof(s,m)))
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

const char* g_ApplicationName = "Cg Template";

const unsigned int g_uiWindowWidth = 800;
const unsigned int g_uiWindowHeight = 600;

const char* g_ShaderProgramName = "Shader.cgfx";

glm::vec3 g_CameraPosition = glm::vec3(5, 0, 3);

//Keep track of our own matrices
glm::mat4 g_ProjectionMatrix;
glm::mat4 g_ViewMatrix;
glm::mat4 g_ModelMatrix;


//Our Geometries
struct VertexXYZColor
{
	glm::vec3 m_Pos;    // X, Y, Z
	glm::vec3 m_Color;  // R, G, B
};

// Define the 8 vertices of a unit cube
VertexXYZColor g_Vertices[8] = {
	{ glm::vec3(1,  1,  1), glm::vec3(1, 1, 1) }, // 0
	{ glm::vec3(-1,  1,  1), glm::vec3(0, 1, 1) }, // 1
	{ glm::vec3(-1, -1,  1), glm::vec3(0, 0, 1) }, // 2
	{ glm::vec3(1, -1,  1), glm::vec3(1, 0, 1) }, // 3
	{ glm::vec3(1, -1, -1), glm::vec3(1, 0, 0) }, // 4
	{ glm::vec3(-1, -1, -1), glm::vec3(0, 0, 0) }, // 5
	{ glm::vec3(-1,  1, -1), glm::vec3(0, 1, 0) }, // 6
	{ glm::vec3(1,  1, -1), glm::vec3(1, 1, 0) }, // 7
};



// Define the vertex indices for the triangles
// of the cube.
GLuint g_Indices[36] = {
	0, 1, 2, 2, 3, 0,   // Front face
	7, 4, 5, 5, 6, 7,   // Back face
	6, 5, 2, 2, 1, 6,   // Left face
	7, 0, 3, 3, 4, 7,   // Right face
	7, 6, 1, 1, 0, 7,   // Top face
	3, 2, 5, 5, 4, 3,   // Bottom face
};

//OpenGL vertex buffers (VBO) for our vertex data
GLuint g_VertexBuffer = 0;
GLuint g_IndexBuffer = 0;
//OpenGL vertex array object (VAO) for our cube mesh
GLuint g_VertexArray = 0;

//Cg variables
//The context
CGcontext g_cgContext = NULL;

//Cg effect and pass variables
CGeffect g_cgEffect = NULL;
CGtechnique g_cgTechnique = NULL;

//Cg effect parameters
//float4x4 modelViewProjection
CGparameter g_cgModelViewProjection = NULL;

//Define the default attribute ID's for the POSITION and DIFFUSE Cg semantics
#define POSITION_ATTRIBUTE 0 //vertex position bound to attribute ID 0
#define DIFFUSE_ATTRIBUTE 3 //vertex color bound to .....



#ifdef _DEBUG
#define checkGL() CheckOpenGLError( __FUNCSIG__, __FILE__, __LINE__ )
#else
#define checkGL() ((void*)0) // Do nothing in release builds.
#endif 

//OpenGL error handler
inline void CheckOpenGLError(const char* msg, const char* file, int line){
	GLenum errorCode = GL_NO_ERROR;
	while ((errorCode = glGetError()) != GL_NO_ERROR) 
	{
		std::stringstream ss;
		const GLubyte* errorString = gluErrorString(errorCode);
		if (errorString != NULL)
		{
			ss << file << "(" << line << "): OpenGL Error: " << errorString << ": " << msg << std::endl;
			OutputDebugStringA(ss.str().c_str());
		}
	}
}

//Cg error handler
typedef void(*CGerrorHandlerFunc)(CGcontext context, CGerror error, void* appdata);
void CgErrorHandler(CGcontext context, CGerror error, void* appdata) {
	if (error != CG_NO_ERROR) 
	{
		std::stringstream ss;
		const char* pStr = cgGetErrorString(error);
		std::string strError = (pStr == NULL) ? "" : pStr;
		ss << "Cg ERROR: " << strError << std::endl;

		std::string strListing;
		if (error == CG_COMPILER_ERROR)
		{
			pStr = cgGetLastListing(context);
			strListing = (pStr == NULL) ? "" : pStr;

			ss << strListing << std::endl;
		}
		OutputDebugStringA(ss.str().c_str());
	}
}


void OnDisplay()
{
	std::cout << "OnDisplay()" << std::endl;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); checkGL();

	glBindVertexArray(g_VertexArray); checkGL();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IndexBuffer); checkGL();

	glm::mat4 modelViewProjectionMatrix = g_ProjectionMatrix * g_ViewMatrix * g_ModelMatrix;
	cgSetParameterValuefc(g_cgModelViewProjection, 16, glm::value_ptr(modelViewProjectionMatrix));

	CGpass pass = cgGetFirstPass(g_cgTechnique);
	while (pass)
	{
		cgSetPassState(pass);
		glDrawElements(GL_TRIANGLES, sizeof(g_Indices) / sizeof(GLuint), GL_UNSIGNED_INT, BUFFER_OFFSET(0)); checkGL();
		cgResetPassState(pass);

		pass = cgGetNextPass(pass);
	}

	//unbind
	glBindVertexArray(0); checkGL();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); checkGL();

	glutSwapBuffers();
}

void OnClose()
{
	// Delete the buffers we allocated
	if (g_VertexBuffer != 0)
	{
		glDeleteBuffers(1, &g_VertexBuffer);
		g_VertexBuffer = 0;
	}

	if (g_IndexBuffer != 0)
	{
		glDeleteBuffers(1, &g_IndexBuffer);
		g_IndexBuffer = 0;
	}

	if (g_VertexArray != 0)
	{
		glDeleteVertexArrays(1, &g_VertexArray);
		g_VertexArray = 0;
	}

	if (g_cgEffect != NULL)
	{
		cgDestroyEffect(g_cgEffect);
		g_cgEffect = NULL;
	}

	if (g_cgContext != NULL)
	{
		cgDestroyContext(g_cgContext);
		g_cgContext = NULL;
	}

	cgSetErrorHandler(NULL, NULL);
}


void InitGLUT(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);


	glutInitContextVersion(3, 1);

	glutInitWindowSize(g_uiWindowWidth, g_uiWindowHeight);
	int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
	int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);

	glutInitWindowPosition((screenWidth - g_uiWindowWidth) / 2, (screenHeight - g_uiWindowHeight) / 2);


	glutCreateWindow(g_ApplicationName);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// Register GLUT callbacks
	glutDisplayFunc(&OnDisplay);
	glutCloseFunc(&OnClose);

	checkGL();
}

void InitGLEW()
{
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Problem initializing GLEW" << std::endl;
		exit(-1);
	}
	
	if (!GLEW_VERSION_3_1)
	{
		std::cerr << "OpenGL 3.1 required version support not present." << std::endl;
		exit(-1);
	}

	if (WGLEW_EXT_swap_control)
	{
		wglSwapIntervalEXT(0);
	}
	checkGL();
}

void InitGL()
{
	while (glGetError() != GL_NO_ERROR);

	// Output interesting information about the platform we are using
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl; checkGL();
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl; checkGL();
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl; checkGL();
	std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl; checkGL();

	glClearColor(0.0f, 0.0f, 0.0f, 1); checkGL();
	glClearDepth(1.0); checkGL();

	glEnable(GL_DEPTH_TEST); checkGL();
	glEnable(GL_CULL_FACE); checkGL();

	//create VAO
	glGenVertexArrays(1, &g_VertexArray); checkGL();
	glBindVertexArray(g_VertexArray); checkGL();

	//create VBO
	glGenBuffers(1, &g_VertexBuffer); checkGL();
	glGenBuffers(1, &g_IndexBuffer); checkGL();

	//copy the vertex data to the buffers
	glBindBuffer(GL_ARRAY_BUFFER, g_VertexBuffer); checkGL();
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_Vertices), g_Vertices, GL_STATIC_DRAW); checkGL();

	//bind data to the attribute streams
	glVertexAttribPointer(POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(VertexXYZColor), MEMBER_OFFSET(VertexXYZColor, m_Pos)); checkGL();
	glEnableVertexAttribArray(POSITION_ATTRIBUTE); checkGL();

	glVertexAttribPointer(DIFFUSE_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(VertexXYZColor), MEMBER_OFFSET(VertexXYZColor, m_Color)); checkGL();
	glEnableVertexAttribArray(DIFFUSE_ATTRIBUTE); checkGL();

	//unbind the vertex array
	glBindVertexArray(0); checkGL();
	//unbind the buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0); checkGL();

	//copy the index data to the buffers
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IndexBuffer); checkGL();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_Indices), g_Indices, GL_STATIC_DRAW); checkGL();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); checkGL();

	g_ViewMatrix = glm::lookAt(g_CameraPosition, glm::vec3(0,0,0), glm::vec3(0,1,0));
	
}

void InitCG()
{

	cgSetErrorHandler(&CgErrorHandler, NULL);

	g_cgContext = cgCreateContext();


	cgGLRegisterStates(g_cgContext);
	cgGLSetManageTextureParameters(g_cgContext, CG_TRUE);

    // Load the CGeffect (CGfx) file
	g_cgEffect = cgCreateEffectFromFile(g_cgContext, g_ShaderProgramName, NULL);
	if (g_cgEffect == NULL) exit(-1);

	//Validate the technique
	g_cgTechnique = cgGetFirstTechnique(g_cgEffect);
	while (g_cgTechnique != NULL && cgValidateTechnique(g_cgTechnique) == CG_FALSE)
	{
		std::cerr << "Cg ERROR: Technique with name " << cgGetTechniqueName(g_cgTechnique) << "did not pass validation" << std::endl;
		g_cgTechnique = cgGetNextTechnique(g_cgTechnique);
	}

	//make sure we found a valid technique
	if (g_cgTechnique == NULL || cgIsTechniqueValidated(g_cgTechnique) == CG_FALSE)
	{
		std::cerr << "Cg ERROR: Could not find any valid techniques." << std::endl;
		exit(-1);
	}

	//Get the effect parameters. Later used to set the parameter.
	g_cgModelViewProjection = cgGetNamedEffectParameter(g_cgEffect, "ModelViewProjection");
}


int main(int argc, char* argv[]) {
	InitGLUT(argc, argv);
	InitGLEW();
	InitGL();
	InitCG();

	g_ModelMatrix = glm::mat4(1.0f);
	g_ProjectionMatrix = glm::perspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);

	//glViewport(0, 0, 1920, 1080); checkGL();

	std::cout << "Before entering main loop" << std::endl;

	//Start processing our event loop
	glutMainLoop();

	return 0;
}

