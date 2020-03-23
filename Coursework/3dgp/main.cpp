#include <iostream>
#include "GL/glew.h"
#include "GL/3dgl.h"
#include "GL/glut.h"
#include "GL/freeglut_ext.h"

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// Include GLM extensions
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// GLSL Program
C3dglProgram Program;
C3dglProgram ProgramEffect;


// 3D models
C3dglModel camera;
C3dglModel table;
C3dglModel LivingRoom;
C3dglModel knife;
C3dglModel vase;
C3dglModel lamp;

//Global texture variable
GLuint idTexWood;
GLuint idTexScreen;
GLuint idFBO;
GLuint idTexFabric;
GLuint idTexNone;
GLuint bufQuad;
GLuint WImage = 800, HImage = 600;

//lamps on and off
int lamp1 = 1, lamp2 = 1;

//triangle verts
float vertices[] = {
			4, 0, 4, -4, 0, 4, 0, -7, 0, 4, 0, -4, -4, 0,-4, 0, -7, 0,
			4, 0, 4, 4, 0, -4, 0, -7, 0, -4, 0, 4, -4, 0, -4, 0, -7, 0,
			4, 0, 4, 4, 0, -4, -4, 0, 4, -4, 0, -4 };
float normals[] = {
	0, -4, 7, 0, -4, 7, 0, -4, 7, 0, -4, -7, 0, -4, -7, 0, -4, -7,
	7, -4, 0, 7, -4, 0, 7, -4, 0, -7, -4, 0, -7, -4, 0, -7, -4, 0,
	0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 };
unsigned indices[] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 13, 14, 15 };

// buffers names
unsigned vertexBuffer = 0;
unsigned normalBuffer = 0;
unsigned indexBuffer = 0;

// this global variable controls the animation
float theta = glutGet(GLUT_ELAPSED_TIME) * 0.01f;

// camera position (for first person type camera navigation)
mat4 matrixView;			// The View Matrix
float angleTilt = 15;		// Tilt Angle
float angleRot = 0.05f;		// Camera orbiting angle
vec3 cam(0);				// Camera movement values

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// Initialise Shaders
	C3dglShader VertexShader;
	C3dglShader FragmentShader;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/basic.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/basic.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!Program.Create()) return false;
	if (!Program.Attach(VertexShader)) return false;
	if (!Program.Attach(FragmentShader)) return false;
	if (!Program.Link()) return false;
	if (!Program.Use(true)) return false;

	// Initialise Shaders
	C3dglShader VertexShaderEffect;
	C3dglShader FragmentShaderEffect;

	if (!VertexShaderEffect.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShaderEffect.LoadFromFile("shaders/basicEffect.vert")) return false;
	if (!VertexShaderEffect.Compile()) return false;

	if (!FragmentShaderEffect.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShaderEffect.LoadFromFile("shaders/basicEffect.frag")) return false;
	if (!FragmentShaderEffect.Compile()) return false;

	if (!ProgramEffect.Create()) return false;
	if (!ProgramEffect.Attach(VertexShaderEffect)) return false;
	if (!ProgramEffect.Attach(FragmentShaderEffect)) return false;
	if (!ProgramEffect.Link()) return false;
	if (!ProgramEffect.Use(true)) return false;

	// Reenable main shader
	Program.Use();


	// glut additional setup
	glutSetVertexAttribCoord3(Program.GetAttribLocation("aVertex"));
	glutSetVertexAttribNormal(Program.GetAttribLocation("aNormal"));

	// prepare vertex data
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// prepare normal data
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

	// prepare indices array
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	// load your 3D models here!
	if (!camera.load("models\\camera.3ds")) return false;
	if (!table.load("models\\table.obj")) return false;
	if (!LivingRoom.load("models\\LivingRoom.obj")) return false;
	if (!vase.load("models\\vase.obj")) return false;
	if (!lamp.load("models\\lamp.obj")) return false;
	if (!knife.load("models\\02_-_Knife.obj")) return false;

	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));
	matrixView *= lookAt(
		vec3(0.0, 5.0, 10.0),
		vec3(0.0, 5.0, 0.0),
		vec3(0.0, 1.0, 0.0));

	// setup the screen background colour
	glClearColor(0.8f, 0.25f, 0.22f, 1.0f);   // deep grey background

	// setup lighting

	//Emissive Light
	Program.SendUniform("lightAmbient1.on", 0);
	Program.SendUniform("lightAmbient1.color", 1.0, 1.0, 1.0);
	
	//Natural ambient light
	Program.SendUniform("lightAmbient2.on", 1);
	Program.SendUniform("lightAmbient2.color", 0.1, 0.1, 0.1);
	Program.SendUniform("materialAmbient", 0.2, 0.2, 0.2);

	//Directional Light
	Program.SendUniform("lightDir.on", 1);
	Program.SendUniform("lightDir.direction", 1.0, 0.5, 1.0);
	Program.SendUniform("lightDir.diffuse", 0.2, 0.2, 0.2);	  // dimmed white lights

	//Lamp 1 Point light
	Program.SendUniform("lightPoint1.on", 1);
	Program.SendUniform("lightPoint1.position", 18.7f, 1.7f, 10.5f);
	Program.SendUniform("lightPoint1.diffuse", 0.5, 0.5, 0.5);
	Program.SendUniform("lightPoint1.specular", 0.5, 0.5, 0.5);
	Program.SendUniform("lightPoint1.att_quadratic", 0.01);
	Program.SendUniform("materialSpecular", 0.3, 0.3, 0.3);
	Program.SendUniform("shininess", 8.0);
	

	//Lamp 2 Point Light
	Program.SendUniform("lightPoint2.on", 1);
	Program.SendUniform("lightPoint2.position", 20.0f, 1.7f, -8.0f);
	Program.SendUniform("lightPoint2.diffuse", 0.5, 0.5, 0.5);
	Program.SendUniform("lightPoint2.specular", 0.5, 0.5, 0.5);
	Program.SendUniform("lightPoint2.att_quadratic", 0.01);
	Program.SendUniform("materialSpecular", 0.3, 0.3, 0.3);
	Program.SendUniform("shininess", 8.0);
	

	// Send the texture info to the shaders
	Program.SendUniform("texture0", 0);
	ProgramEffect.SendUniform("texture0", 0);

	C3dglBitmap bm;

	bm.Load("textures/oak.bmp", GL_RGBA);
	if (!bm.GetBits()) return false;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexWood);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("textures/cloth.bmp", GL_RGBA);
	if (!bm.GetBits()) return false;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexFabric);
	glBindTexture(GL_TEXTURE_2D, idTexFabric);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm.GetBits());

	// none (simple-white) texture
	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);

	
	// Create screen space texture
	glGenTextures(1, &idTexScreen);
	glBindTexture(GL_TEXTURE_2D, idTexScreen);

	// Texture parameters - to get nice filtering 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// This will allocate an uninitilised texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WImage, HImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// Create a framebuffer object (FBO)
	glGenFramebuffers(1, &idFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, idFBO);

	// Attach a depth buffer
	GLuint depth_rb;
	glGenRenderbuffers(1, &depth_rb);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, WImage, HImage);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);

	// attach the texture to FBO depth attachment point
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, idTexScreen, 0);

	// switch back to window-system-provided framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	
	// Create Quad
	float vertices[] = {
		0.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,	1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,	0.0f, 1.0f
	};
	// Generate the buffer name
	glGenBuffers(1, &bufQuad);
	// Bind the vertex buffer and send data
	glBindBuffer(GL_ARRAY_BUFFER, bufQuad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Shift+AD or arrow key to auto-orbit" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;



	return true;
}

void done()
{
}


// called before window opened or resized - to setup the Projection Matrix
void reshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 matrixProjection = perspective(radians(60.f), ratio, 0.02f, 1000.f);

	// Setup the Projection Matrix
	Program.SendUniform("matrixProjection", matrixProjection);

}


void renderObjects(mat4 m, float theta)
{	
	glBindTexture(GL_TEXTURE_2D, idTexNone);

	// setup materials - grey
	Program.SendUniform("materialDiffuse", 0.6f, 0.6f, 0.6f);

	// setup lightbulb

	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	Program.SendUniform("lightAmbient1.on", lamp2);
	m = matrixView;
	m = translate(m, vec3(20.23f, 4.24f, -7.35f));
	m = scale(m, vec3(0.5f, 0.5f, 0.5f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);
	Program.SendUniform("lightAmbient1.on", 0);


	// lamp1
	Program.SendUniform("materialAmbient", 0.2, 0.2, 0.2);
	m = matrixView;
	m = translate(m, vec3(23.0f, -1.7, -5.0f));
	m = rotate(m, radians(-40.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.1f, 0.1f, 0.1f));
	lamp.render(m);

	// setup lightbulb
	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	Program.SendUniform("lightAmbient1.on", lamp1);
	m = matrixView;
	m = translate(m, vec3(17.77f, 4.24f, 9.33f));
	m = scale(m, vec3(0.5f, 0.5f, 0.5f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);
	Program.SendUniform("lightAmbient1.on", 0);

	Program.SendUniform("materialAmbient", 0.2, 0.2, 0.2);

	// lamp2
	m = matrixView;
	m = translate(m, vec3(15.0f, -1.7, 7.0f));
	m = rotate(m, radians(140.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.1f, 0.1f, 0.1f));
	lamp.render(m);

	// vase
	m = matrixView;
	m = translate(m, vec3(20.0f, -2, 2.0f));
	m = rotate(m, radians(160.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.3f, 0.3f, 0.3f));
	vase.render(m);

	// setup materials - dark grey
	Program.SendUniform("materialDiffuse", 0.3f, 0.3f, 0.3f);

	Program.SendUniform("lightPoint2.diffuse", 0.2, 0.2, 0.2);
	Program.SendUniform("lightPoint2.specular", 0.2, 0.2, 0.2);
	Program.SendUniform("lightPoint1.diffuse", 0.2, 0.2, 0.2);
	Program.SendUniform("lightPoint1.specular", 0.2, 0.2, 0.2);
	Program.SendUniform("materialSpecular", 0.1, 0.1, 0.1);
	Program.SendUniform("shininess", 1.0);

	// living room
	m = matrixView;
	m = translate(m, vec3(20.0f, -17, 2.0f));
	m = rotate(m, radians(160.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.15f, 0.15f, 0.15f));
	LivingRoom.render(m);

	// setup materials - black
	Program.SendUniform("materialDiffuse", 0.1f, 0.1f, 0.1f);

	// camera
	m = matrixView;
	m = translate(m, vec3(-15.0f, 5.5, -30.0f));
	m = rotate(m, radians(140.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.04f, 0.04f, 0.04f));
	camera.render(m);

	Program.SendUniform("lightPoint1.diffuse", 0.5, 0.5, 0.5);
	Program.SendUniform("lightPoint1.specular", 0.5, 0.5, 0.5);
	Program.SendUniform("lightPoint2.diffuse", 0.5, 0.5, 0.5);
	Program.SendUniform("lightPoint2.specular", 0.5, 0.5, 0.5);
	Program.SendUniform("materialSpecular", 0.3, 0.3, 0.3);
	Program.SendUniform("shininess", 8.0);

	// setup materials -Darker Brown
	Program.SendUniform("materialDiffuse", 0.25f, 0.15f, 0.1f);

	// chairs
	m = matrixView;
	glBindTexture(GL_TEXTURE_2D, idTexFabric);
	m = translate(m, vec3(20.0f, -17, 0.0f));
	m = rotate(m, radians(0.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.02f, 0.02f, 0.02f));
	table.render(0, m);

	// chairs
	m = matrixView;
	glBindTexture(GL_TEXTURE_2D, idTexFabric);
	m = translate(m, vec3(20.0f, -17, 0.0f));
	m = rotate(m, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.02f, 0.02f, 0.02f));
	table.render(0, m);

	// chairs
	m = matrixView;
	glBindTexture(GL_TEXTURE_2D, idTexFabric);
	m = translate(m, vec3(20.0f, -17, 0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.02f, 0.02f, 0.02f));
	table.render(0, m);

	// chairs
	m = matrixView;
	glBindTexture(GL_TEXTURE_2D, idTexFabric);
	m = translate(m, vec3(20.0f, -17, 0.0f));
	m = rotate(m, radians(270.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.02f, 0.02f, 0.02f));
	table.render(0, m);

	// setup materials -Brown
	Program.SendUniform("materialDiffuse", 0.35f, 0.15f, 0.1f);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	// table
	m = matrixView;
	m = translate(m, vec3(20.0f, -17, 0.0f));
	m = rotate(m, radians(270.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.02f, 0.02f, 0.02f));
	table.render(1, m);


	// setup materials - Yellow
	Program.SendUniform("materialDiffuse", 0.8f, 0.8f, 0.2f);

	// teapot
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	m = matrixView;
	m = translate(m, vec3(15.0f, -0.5, 0.0f));
	m = rotate(m, radians(120.f), vec3(0.0f, 1.0f, 0.0f));
	// the GLUT objects require the Model View Matrix setup
	Program.SendUniform("matrixModelView", m);
	glutSolidTeapot(2.0);

	// setup materials - dark grey
	Program.SendUniform("materialDiffuse", 0.2f, 0.2f, 0.9f);

	// knife
	m = matrixView;
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	m = translate(m, vec3(18.4f, 3.0f, -8.f));
	m = rotate(m, radians(-270.f), vec3(0.0f, 0.0f, 1.0f));
	m = scale(m, vec3(0.7f, 0.7f, 0.7f));
	m = rotate(m, radians(90.f) * theta * 0.1f, vec3(1.0f, 0.0f, 0.0f));

	knife.render(m);

	// setup materials - blue
	Program.SendUniform("materialDiffuse", 0.2f, 0.2f, 0.5f);

	// triangle
	m = matrixView;
	m = translate(m, vec3(18.0f, 0.16, -8.0f));
	m = rotate(m, radians(-45.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.3f, 0.3f, 0.3f));
	m = rotate(m, radians(90.f) * theta * 0.1f, vec3(0.0f, 1.0f, 0.0f));
	// the GLUT objects require the Model View Matrix setup
	Program.SendUniform("matrixModelView", m);

	// Get Attribute Locations
	GLuint attribVertex = Program.GetAttribLocation("aVertex");
	GLuint attribNormal = Program.GetAttribLocation("aNormal");

	// Enable vertex attribute arrays
	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribNormal);

	// Bind (activate) the vertex buffer and set the pointer to it
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind (activate) the normal buffer and set the pointer to it
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Draw triangles – using index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

	// Disable arrays
	glDisableVertexAttribArray(attribVertex);
	glDisableVertexAttribArray(attribNormal);


}


void render()
{
	// Pass 1: off-screen rendering
	glBindFramebufferEXT(GL_FRAMEBUFFER, idFBO);

	// this global variable controls the animation
	float theta = glutGet(GLUT_ELAPSED_TIME) * 0.01f;

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// setup the View Matrix (camera)
	mat4 m = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));// switch tilt off
	m = translate(m, cam);												// animate camera motion (controlled by WASD keys)
	m = rotate(m, radians(-angleTilt), vec3(1.0f, 0.f, 0.f));			// switch tilt on
	m = m * matrixView;
	//m = rotate(m, radians(angleRot), vec3(0.f, 0.1f, 0.f));				// animate camera orbiting
	matrixView = m;
	
	// setup View Matrix
	Program.SendUniform("matrixView", matrixView);

	renderObjects(m, theta);
		
	// Pass 2: on-screen rendering
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	ProgramEffect.Use();

	// setup ortographic projection
	ProgramEffect.SendUniform("matrixProjection", ortho(0, 1, 0, 1, -1, 1));

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, idTexScreen);

	// setup identity matrix as the model-view
	ProgramEffect.SendUniform("matrixModelView", mat4(1));

	GLuint attribVertex = ProgramEffect.GetAttribLocation("aVertex");
	GLuint attribTextCoord = ProgramEffect.GetAttribLocation("aTexCoord");
	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribTextCoord);
	glBindBuffer(GL_ARRAY_BUFFER, bufQuad);
	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glVertexAttribPointer(attribTextCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
		(void*)(3 * sizeof(float)));
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableVertexAttribArray(attribVertex);
	glDisableVertexAttribArray(attribTextCoord);

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}



// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': cam.z = std::max(cam.z * 1.05f, 0.01f); break;
	case 's': cam.z = std::min(cam.z * 1.05f, -0.01f); break;
	case 'a': cam.x = std::max(cam.x * 1.05f, 0.01f); angleRot = 0.1f; break;
	case 'd': cam.x = std::min(cam.x * 1.05f, -0.01f); angleRot = -0.1f; break;
	case 'e': cam.y = std::max(cam.y * 1.05f, 0.01f); break;
	case 'q': cam.y = std::min(cam.y * 1.05f, -0.01f); break;
	case '1':
		lamp1 = 1;
		Program.SendUniform("lightPoint1.on", 1); break;
	case '2':
		lamp2 = 1;
		Program.SendUniform("lightPoint2.on", 1); break;
	case '3':
		lamp1 = 0;
		Program.SendUniform("lightPoint1.on", 0); break;
	case '4':
		lamp2 = 0;
		Program.SendUniform("lightPoint2.on", 0); break;

	}
	// speed limit
	cam.x = std::max(-0.15f, std::min(0.15f, cam.x));
	cam.y = std::max(-0.15f, std::min(0.15f, cam.y));
	cam.z = std::max(-0.15f, std::min(0.15f, cam.z));
	// stop orbiting
	if ((glutGetModifiers() & GLUT_ACTIVE_SHIFT) == 0) angleRot = 0;
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': cam.z = 0; break;
	case 'a':
	case 'd': cam.x = 0; break;
	case 'q':
	case 'e': cam.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
bool bJustClicked = false;
void onMouse(int button, int state, int x, int y)
{
	bJustClicked = (state == GLUT_DOWN);
	glutSetCursor(bJustClicked ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
}

// handle mouse move
void onMotion(int x, int y)
{
	if (bJustClicked)
		bJustClicked = false;
	else
	{
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// find delta (change to) pan & tilt
		float deltaPan = 0.25f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
		float deltaTilt = 0.25f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// View = Tilt * DeltaPan * Tilt^-1 * DeltaTilt * View;
		angleTilt += deltaTilt;
		mat4 m = mat4(1.f);
		m = rotate(m, radians(angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaPan), vec3(0.f, 1.f, 0.f));
		m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaTilt), vec3(1.f, 0.f, 0.f));
		matrixView = m * matrixView;
	}
}

int main(int argc, char** argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("CI5520 3D Graphics Programming");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
		return 0;
	}
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	// register callbacks
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);

	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Version: " << glGetString(GL_VERSION) << endl;

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		cerr << "Application failed to initialise" << endl;
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	done();

	return 1;
}

