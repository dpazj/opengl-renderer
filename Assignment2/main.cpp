/*
//Joe Riemersma
*/


/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#pragma comment(lib, "assimp-vc140-mt.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glload.lib")
#pragma comment(lib, "assimp-vc140-mt.lib")
#endif
#pragma comment(lib, "opengl32.lib")

#define STB_IMAGE_IMPLEMENTATION

/* Include the header to the GLFW wrapper class which
also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>

// We'll use the STD stack class to make our stack or matrices
#include <stack>
#include <memory>
#include <vector>
#include <string>

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>


#include "OpenJoeL/Shaders/Shader.h"
#include "OpenJoeL/Meshes/ModelMesh.h"
#include "OpenJoeL/Meshes/SphereMesh.h"
#include "OpenJoeL/Meshes/CubeMesh.h"
#include "OpenJoeL/Meshes/PlaneMesh.h"

#include "OpenJoeL/Meshes/Object.h"

#include "OpenJoeL/Texture/Texture.h"

#include "OpenJoeL/Utils/Camera.h"
#include "OpenJoeL/Utils/InputManager.h"
#include "OpenJoeL/Environment/Skybox.h"

#include "OpenJoeL/Render/DynamicCubemap.h"
#include "OpenJoeL/Lighting/Shadows.h"



// Include headers for our objects

GLint screen_width = 1680;
GLint screen_height = 1050;
GLfloat aspect_ratio = (GLfloat)screen_width / (GLfloat)screen_height;


//SHADERS
//std::shared_ptr<Shader> shader;
Shader* shadow_shader;
Shader* debug;
Shader* pbr_shader;

PBRObject* cube1;
PBRObject* cube2;
PBRObject* plane;


Camera camera(glm::vec3(0, 2, 5));
InputManager* input_manager;


DirectionalShadowMap * shadow_map;


bool blue_render = true;

GLfloat delta_time = 0;
GLfloat last_frame = 0;

glm::vec3 lightPos(0, 4.0f, -1);

glm::vec3 lightPositions[] = {
	glm::vec3(0.0f, 4.0f, 0.0f)
};
glm::vec3 lightColors[] = {
	glm::vec3(300.0f, 300.0f, 300.0f)
};

unsigned int loadTexture(char const* path);



unsigned int planeVBO, planeVAO;
unsigned int woodTexture;
unsigned int depthMapFBO;
unsigned int depthMap;




void setup_inputs(GLWrapper* glw)
{
	//Camera controls
	input_manager->AddKey(GLFW_KEY_W, []() {
		camera.ProcessKeyboard(FORWARD, delta_time);
	});

	input_manager->AddKey(GLFW_KEY_S, []() {
		camera.ProcessKeyboard(BACK, delta_time);
	});

	input_manager->AddKey(GLFW_KEY_A, []() {
		camera.ProcessKeyboard(LEFT, delta_time);
	});

	input_manager->AddKey(GLFW_KEY_D, []() {
		camera.ProcessKeyboard(RIGHT, delta_time);
	});

	input_manager->SetMouseCallback([](GLFWwindow* window, double x, double y) {
		camera.ProcessMouse((GLfloat)x, (GLfloat)y);
	});

	input_manager->AddKey(GLFW_KEY_ESCAPE, [glw]() {
		glfwSetWindowShouldClose(glw->getWindow(), GL_TRUE);
	});

}


void init(GLWrapper* glw)
{

	//load shaders
	try
	{

		pbr_shader = new Shader("../shaders/pbr.vert", "../shaders/pbr.frag");
		shadow_shader = new Shader("../shaders/directionalshadow.vert", "../shaders/directionalshadow.frag");
		debug = new Shader("../shaders/debug.vert", "../shaders/debug.frag");

	}
	catch (std::exception & e)
	{
		std::cout << "Caught exception: " << e.what() << std::endl;
		std::cin.ignore();
		exit(0);
	}

	setup_inputs(glw);
	
	debug->UseShader();
	debug->SetInt("shadow_map", 7);

	pbr_shader->UseShader();
	pbr_shader->SetInt("shadow_map", 7);
	

	CubeMesh spheremesh;
	spheremesh.Init();
	
	PlaneMesh planemesh;
	planemesh.Init();

	plane = new PBRObject(planemesh.GetMesh());
	plane->transform.Scale(5.0f);
	plane->SetPBRProperties(glm::vec3(1,0,0));

	cube1 = new PBRObject(spheremesh.GetMesh());
	cube1->SetPBRProperties(glm::vec3(0, 1, 0));
	cube1->transform.Scale(2.0f);
	cube1->transform.Translate(glm::vec3(1, 1, 0));

	cube2 = new PBRObject(spheremesh.GetMesh());
	cube2->SetPBRProperties(glm::vec3(0, 0, 1));
	cube2->transform.Scale(1.0f);
	cube2->transform.Translate(glm::vec3(-1, 0.5, 0));

	shadow_map = new DirectionalShadowMap(1024);

}



void RenderScene(glm::vec3 camera_pos, glm::mat4 projection, glm::mat4 view, Shader* shader)
{
	shader->UseShader();
	shader->SetVec3("light_positions[0]", lightPos);
	shader->SetVec3("light_colors[0]", glm::vec3(1,1,1));
	shader->SetVec3("camera_position", camera_pos);
	shader->SetMat4("lightspacemat", shadow_map->GetMatrix());

	shader->SetMat4("projection", projection);
	shader->SetMat4("view",view);

	cube1->Draw(shader);
	cube2->Draw(shader);
	plane->Draw(shader);


	   	  
}

void display()
{
	const GLfloat currentFrame = (GLfloat)glfwGetTime();
	delta_time = currentFrame - last_frame;
	last_frame = currentFrame;

	input_manager->ProcessInput();

	glEnable(GL_DEPTH_TEST);
	//glClearColor(0.859375f, 0.859375f, 0.859375f, 1.0f);
	glClearColor(0, 0, 0, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	//SHADOW MAPPING
	shadow_map->RenderShadowMap(lightPos, [](glm::mat4 proj, glm::mat4 view) {
		RenderScene(lightPos, proj, view, shadow_shader);
	});


	// reset viewport
	glViewport(0, 0, screen_width, screen_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	//PROPERDRAW
	glm::mat4 projection = glm::perspective(glm::radians(10.f), aspect_ratio, 0.1f, 100.0f);
	glm::mat4 view = camera.GetView();

	shadow_map->BindShadowMap();
	RenderScene(camera.GetPosition(), projection, view, pbr_shader);

}

void modifyTexture(GLuint texture)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}



/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	// Store aspect ratio to use for our perspective projection
	aspect_ratio = float(w) / float(h);
}

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format = 0;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}


/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper* glw = new GLWrapper(screen_width, screen_height, "PBR");
	input_manager = new InputManager(glw->getWindow());

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	// Register the callback functions
	glw->setRenderer(display);
	glw->setReshapeCallback(reshape);

	/* Output the OpenGL vendor and version */
	glw->DisplayVersion();

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}
