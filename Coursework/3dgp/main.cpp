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
C3dglProgram ProgramParticle;


// 3D models
C3dglModel camera;
C3dglModel ceilinglamp;
C3dglModel desk;
C3dglModel table;
C3dglModel LivingRoom;
C3dglModel cigarette;
C3dglModel knife;
C3dglModel vase;
C3dglModel lamp;
C3dglModel player;

//Global texture variable
GLuint idTexWood;
GLuint idTexScreen;
GLuint idFBO;
GLuint idFBO1;
GLuint idTexFabric;
GLuint idTexNone;
GLuint bufQuad;
GLuint WImage = 800, HImage = 600;
GLuint idTexShadowMap;
GLuint idTexSmoke;

GLuint idBufferVelocity;
GLuint idBufferStartTime;


// Character position
vec3 posPlayer = vec3(5.2f, -10.0f, -25.0f);
bool walkRight = true;
bool walkLeft = false;
bool walkForward = false;
bool walkBack = false;

// Time for the animation
float time = 0;

// Particle System Params
const float PERIOD = 0.1f;
const float LIFETIME = 6;
const int NPARTICLES = (int)(LIFETIME / PERIOD);
const float M_PI = 3.141592;


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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(0x8642);    // !!!!
	glEnable(GL_POINT_SPRITE);
	

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
	
	Program.Use();
	
	// Initialise Particle Shader
	C3dglShader VertexShaderParticle;
	C3dglShader FragmentShaderParticle;
	
	if (!VertexShaderParticle.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShaderParticle.LoadFromFile("shaders/basicParticle.vert")) return false;
	if (!VertexShaderParticle.Compile()) return false;

	if (!FragmentShaderParticle.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShaderParticle.LoadFromFile("shaders/basicParticle.frag")) return false;
	if (!FragmentShaderParticle.Compile()) return false;
	
	if (!ProgramParticle.Create()) return false;
	
	if (!ProgramParticle.Attach(VertexShaderParticle)) return false;
	if (!ProgramParticle.Attach(FragmentShaderParticle)) return false;
	if (!ProgramParticle.Link()) return false;
	if (!ProgramParticle.Use(true)) return false;
	
	// Setup the particle system
	ProgramParticle.SendUniform("initialPos", 20.25, -1.25, 9.4);
	ProgramParticle.SendUniform("gravity", 0.0, 0.1, 0.0);
	ProgramParticle.SendUniform("particleLifetime", LIFETIME);

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
	if (!ceilinglamp.load("models\\ceilinglamp.3ds")) return false;
	if (!table.load("models\\table.obj")) return false;
	if (!desk.load("models\\desk.obj")) return false;
	if (!player.load("models\\tiny\\tiny.x")) return false;
	player.loadMaterials("models\\tiny");

	if (!LivingRoom.load("models\\LivingRoomObj\\LivingRoom.obj")) return false;
	LivingRoom.loadMaterials("models\\LivingRoomObj");

	if (!cigarette.load("models\\Cig\\OBJ_2009.obj")) return false;
	cigarette.loadMaterials("models\\Cig");

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
	Program.SendUniform("lightDir.diffuse", 0.02, 0.02, 0.02);	  // dimmed white lights

	//Spot light
	Program.SendUniform("lightSpot.on", 1);
	Program.SendUniform("lightSpot.position", 20.0f, 25.0f, -5.0f);
	Program.SendUniform("lightSpot.diffuse", 0.7, 0.7, 0.7);
	Program.SendUniform("lightSpot.specular", 1.0, 1.0, 1.0);
	Program.SendUniform("lightSpot.direction", 0.0f, -1.0f, 0.0f);
	Program.SendUniform("lightSpot.cutoff", 80.0f);
	Program.SendUniform("lightSpot.attenuation", 5.0f);
	Program.SendUniform("materialSpecular", 0.3, 0.3, 0.3);
	Program.SendUniform("shininess", 8.0);

	//Lamp 1 Point light
	Program.SendUniform("lightPoint1.on", 1);
	Program.SendUniform("lightPoint1.position", 19.0f, 0.5f, 10.1f);
	Program.SendUniform("lightPoint1.diffuse", 0.5, 0.5, 0.5);
	Program.SendUniform("lightPoint1.specular", 0.5, 0.5, 0.5);
	Program.SendUniform("lightPoint1.att_quadratic", 0.1);
	Program.SendUniform("materialSpecular", 0.3, 0.3, 0.3);
	Program.SendUniform("shininess", 8.0);
	
	//Lamp 2 Point Light
	Program.SendUniform("lightPoint2.on", 1);
	Program.SendUniform("lightPoint2.position", 48.8f, 4.0f, -46.85f);
	Program.SendUniform("lightPoint2.diffuse", 0.5, 0.5, 0.5);
	Program.SendUniform("lightPoint2.specular", 0.5, 0.5, 0.5);
	Program.SendUniform("lightPoint2.att_quadratic", 0.1);
	Program.SendUniform("materialSpecular", 0.3, 0.3, 0.3);
	Program.SendUniform("shininess", 8.0);
	
	// Send the texture info to the shaders
	Program.SendUniform("texture0", 0);
	ProgramEffect.SendUniform("texture0", 0);
	ProgramParticle.SendUniform("texture0", 0);

	C3dglBitmap bm;

	bm.Load("textures/smoke.bmp", GL_RGBA);
	if (!bm.GetBits()) return false;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexSmoke);
	glBindTexture(GL_TEXTURE_2D, idTexSmoke);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm.GetBits());

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

	// Create shadow map texture
	glActiveTexture(GL_TEXTURE7);
	glGenTextures(1, &idTexShadowMap);
	glBindTexture(GL_TEXTURE_2D, idTexShadowMap);

	// Texture parameters - to get nice filtering & avoid artefact on the edges of the shadowmap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

	// This will associate the texture with the depth component in the Z-buffer
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		WImage * 2, HImage * 2, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	// Send the texture info to the shaders
	Program.SendUniform("shadowMap", 7);

	// revert to texture unit 0
	glActiveTexture(GL_TEXTURE0);

	// Create a framebuffer object (FBO)
	glGenFramebuffers(1, &idFBO1);
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, idFBO1);

	// Instruct openGL that we won't bind a color texture with the currently binded FBO
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// attach the texture to FBO depth attachment point
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D,
		idTexShadowMap, 0);

	// switch back to window-system-provided framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

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
	
	// Prepare the particle buffers
	std::vector<float> bufferVelocity;
	std::vector<float> bufferStartTime;
	float time = 0;
	for (int i = 0; i < NPARTICLES; i++)
	{
		float theta = M_PI / 4.5f * (float)rand() / (float)RAND_MAX;
		float phi = M_PI * 5.0f * (float)rand() / (float)RAND_MAX;
		float x = sin(theta) * cos(phi);
		float y = cos(theta);
		float z = sin(theta) * sin(phi);
		float v = 0.1 + 0.2f * (float)rand() / (float)RAND_MAX;

		bufferVelocity.push_back(x * v);
		bufferVelocity.push_back(y * v);
		bufferVelocity.push_back(z * v);

		bufferStartTime.push_back(time);
		time += PERIOD;
	}
	glGenBuffers(1, &idBufferVelocity);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferVelocity.size(), &bufferVelocity[0],
		GL_STATIC_DRAW);
	glGenBuffers(1, &idBufferStartTime);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferStartTime.size(), &bufferStartTime[0],
		GL_STATIC_DRAW);

	

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
	ProgramParticle.SendUniform("matrixProjection", matrixProjection);

}

vec3 PlayerDirection(vec3 playerPos, float Tdelta)
{
	if (posPlayer.x > 35.0f) { walkRight = false; posPlayer.x = 35.0f;}
	if (posPlayer.x < -2.0f) { walkRight = true; posPlayer.x = -2.0f; }


	if (walkRight == true) return vec3(5.2f * Tdelta, 0, 0);
	if (walkRight == false) return vec3(-5.2f * Tdelta, 0, 0);
}


void renderObjects(mat4 m, float theta, float deltaT, bool renderRoom)
{	
	// setup materials - grey
	Program.SendUniform("materialDiffuse", 0.6f, 0.6f, 0.6f);

	glBindTexture(GL_TEXTURE_2D, idTexNone);;

	// Pendulum mechanics
	static float alpha = 0;
	static float delta = 0.2f;
	delta -= alpha / 5000;
	alpha += delta;

	// Ceiling lamp
	m = matrixView;
	m = translate(m, vec3(16.0f, 35.6f, -4.0f));
	m = rotate(m, radians(alpha), vec3(0.5, 0, 1));
	m = translate(m, vec3(0, -9, 0));
	mat4 m1 = m;
	m = translate(m, vec3(0, 9, 0));
	m = scale(m, vec3(0.3f, 0.15f, 0.3f));
	ceilinglamp.render(m);

	// light bulb
	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	Program.SendUniform("lightAmbient1.on", 1);
	m = m1;
	m = translate(m, vec3(0.0f, -6.5f, 0.0f));
	m = rotate(m, radians(alpha), vec3(0.5, 0, 1));
	m = scale(m, vec3(0.5f, 0.5f, 0.5f));
	Program.SendUniform("matrixModelView", m);
	Program.SendUniform("lightSpot.matrix", m);
	glutSolidSphere(1, 32, 32);
	Program.SendUniform("lightAmbient1.on", 0);
	

	// setup lamp2 lightbulb
	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	Program.SendUniform("lightAmbient1.on", lamp2);
	m = matrixView;
	m = translate(m, vec3(48.8f, 7.0f, -46.85f));
	m = scale(m, vec3(0.5f, 0.5f, 0.5f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);
	Program.SendUniform("lightAmbient1.on", 0);
	

	// lamp1
	Program.SendUniform("materialAmbient", 0.2, 0.2, 0.2);
	m = matrixView;
	m = translate(m, vec3(47.0f, 1.0, -50.0f));
	m = rotate(m, radians(120.f), vec3(0.0f, 1.0f, 0.0f));
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

	// lamp1
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

	// cigarette
	m = matrixView;
	m = translate(m, vec3(20.0f, -1.8f, 10.0f));
	m = rotate(m, radians(160.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.3f, 0.3f, 0.3f));
	cigarette.render(0, m);
	cigarette.render(2, m);
	cigarette.render(3, m);

	if (renderRoom == true)
	{
		// setup materials - dark grey
		Program.SendUniform("materialDiffuse", 0.3f, 0.3f, 0.3f);

		// living room
		m = matrixView;
		m = translate(m, vec3(20.0f, -17, 2.0f));
		m = rotate(m, radians(160.f), vec3(0.0f, 1.0f, 0.0f));
		m = scale(m, vec3(0.15f, 0.15f, 0.15f));
		LivingRoom.render(m);
	}

	// setup materials - black
	Program.SendUniform("materialDiffuse", 0.1f, 0.1f, 0.1f);

	// camera
	m = matrixView;
	m = translate(m, vec3(-15.0f, 5.5, -30.0f));
	m = rotate(m, radians(140.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.04f, 0.04f, 0.04f));
	camera.render(m);

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

	// desk
	m = matrixView;
	m = translate(m, vec3(55.0f, -17.0f, -45.0f));
	m = rotate(m, radians(-20.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.5f, 0.6f, 0.6f));
	desk.render(m);

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

	// player's position
	vec3 posDelta = PlayerDirection(posPlayer, deltaT);		// speed and direction of the walking character
	posPlayer += posDelta;


	// calculate and send bone transforms
	std::vector<float> transforms;
	player.getBoneTransforms(0, time * 90, transforms);
	if (transforms.capacity() != 0)
		Program.SendUniformMatrixv("bones", (float*)&transforms[0], transforms.size() / 16);
	
	m = matrixView;
	m = translate(m, posPlayer);
	m = rotate(m, radians(90.0f), vec3(1, 0, 0));
	if (walkRight == true) m = rotate(m, radians(270.0f), vec3(0, 0, 1));
	if (walkRight == false) m = rotate(m, radians(90.0f), vec3(0, 0, 1));
	m = scale(m, vec3(0.05f, 0.05f, 0.05f));
	player.render(m);

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

void createShadowMap(float theta, float deltaT, 					// animation control variable
	float x, float y, float z,				// coords of the source of the light
	float centerx, float centery, float centerz,	// cords of a point behind the scene
	float upx, float upy, float upz)			// just a reasonable "Up" vector
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	// Store the current viewport in a safe place
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	int w = viewport[2];
	int h = viewport[3];

	// setup the viewport to 2x2 the original and wide (120 degrees) FoV (Field of View)
	glViewport(0, 0, w * 2, h * 2);
	mat4 matrixProjection = perspective(radians(120.f), (float)w / (float)h, 0.5f, 90.0f);
	Program.SendUniform("matrixProjection", matrixProjection);

	// prepare the camera
	mat4 matrixView = lookAt(
		vec3(x, y, z),
		vec3(centerx, centery, centerz),
		vec3(upx, upy, upz));

	// send the View Matrix
	Program.SendUniform("matrixView", matrixView);

	// Bind the Framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, idFBO1);
	// OFF-SCREEN RENDERING FROM NOW!

	// Clear previous frame values - depth buffer only!
	glClear(GL_DEPTH_BUFFER_BIT);

	// Disable color rendering, we only want to write to the Z-Buffer (this is to speed-up)
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	//Prepare and send the Shadow Matrix - this is matrix transform every coordinate x,y,z
	//x = x* 0.5 + 0.5 
	//y = y* 0.5 + 0.5 
	//z = z* 0.5 + 0.5 
	// Moving from unit cube [-1,1] to [0,1]  

	const mat4 bias = {
		{ 0.5, 0.0, 0.0, 0.0 },
		{ 0.0, 0.5, 0.0, 0.0 },
		{ 0.0, 0.0, 0.5, 0.0 },
		{ 0.5, 0.5, 0.5, 1.0 }
	};
	Program.SendUniform("matrixShadow", bias * matrixProjection * matrixView);

	// Render all objects in the scene (except the lamps)	
	renderObjects(matrixView, theta, deltaT, false);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_CULL_FACE);
	reshape(w, h);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}


void render()
{
	// this global variable controls the animation
	float theta = glutGet(GLUT_ELAPSED_TIME) * 0.01f;

	ProgramParticle.SendUniform("time", glutGet(GLUT_ELAPSED_TIME) / 1000.f - 2);

	float tt = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
	float deltaT = tt - time;
	time = tt;

	if(lamp1 == 1) createShadowMap(theta, deltaT, 19.0f, 0.5f, 10.1f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	//if(lamp2 == 1) createShadowMap(theta, deltaT, 48.8f, 4.0f, -46.85f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f);



	// Pass 1: off-screen rendering
	glBindFramebufferEXT(GL_FRAMEBUFFER, idFBO);

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

	
	

	renderObjects(m, theta, deltaT, true);
	
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
	reshape(WImage, HImage);
	
	// setup the point size
	glEnable(GL_POINT_SPRITE);
	glPointSize(2);

	// particles
	//glDepthMask(GL_FALSE);				// disable depth buffer updates
	glActiveTexture(GL_TEXTURE0);			// choose the active texture
	glBindTexture(GL_TEXTURE_2D, idTexSmoke);	// bind the texture

	// RENDER THE PARTICLE SYSTEM
	ProgramParticle.Use();

	m = matrixView;
	ProgramParticle.SendUniform("matrixModelView", m);

	// render the buffer
	glEnableVertexAttribArray(0);	// velocity
	glEnableVertexAttribArray(1);	// start time
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, NPARTICLES);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);


	glDepthMask(GL_TRUE);		// don't forget to switch the depth test updates back on


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
	case 'w': cam.z = std::max(cam.z * 5.05f, 0.1f); break;
	case 's': cam.z = std::min(cam.z * 5.05f, -0.01f); break;
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

