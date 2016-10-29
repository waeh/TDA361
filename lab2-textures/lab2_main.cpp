#ifdef WIN32
#include <windows.h>
#endif


#include <GL/glew.h>

// STB_IMAGE for loading images of many filetypes
#include <stb_image.h>

#include <cstdlib>

#include <labhelper.h>

#include <imgui.h>
#include <imgui_impl_sdl_gl3.h>

// The window we'll be rendering to
SDL_Window* g_window = nullptr;

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

static int mag = 1;
static int mini = 5;
static float anisotropy = 16.0f;

// The shaderProgram holds the vertexShader and fragmentShader
GLuint shaderProgram;

// The vertexArrayObject here will hold the pointers to 
// the vertex data (in positionBuffer) and color data per vertex (in colorBuffer)
GLuint		positionBuffer, colorBuffer, indexBuffer, vertexArrayObject;						



void initGL()
{
	///////////////////////////////////////////////////////////////////////////
	// Create the floor quad
	///////////////////////////////////////////////////////////////////////////	
	// Vertex positions
	const float positions[] = {
		// X Y Z
		-10.0f,   -10.0f, -30.0f,    // v0
		-10.0f,   -10.0f, -330.0f,   // v1
		 10.0f,   -10.0f, -330.0f,   // v2
		 10.0f,   -10.0f, -30.0f     // v3
	};

	// Vertex colors
	const float colors[] = {
		//  R     G		B
		1.0f, 1.0f, 1.0f,		// White
		0.5f, 0.5f, 0.8f,		// Lightblue
		0.5f, 0.5f, 0.8f,		// Lightblue
		1.0f, 1.0f, 1.0f		// White
	};

	// >>> @task 1.1

	const int indices[] = {
		0,1,3, // Triangle 1
		1,2,3  // Triangle 2
	};

	// Create the buffer objects
	glGenBuffers( 1, &positionBuffer );															// Create a handle for the vertex position buffer
	glBindBuffer( GL_ARRAY_BUFFER, positionBuffer );											// Set the newly created buffer as the current one
	glBufferData( GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW );			// Send the vetex position data to the current buffer

	glGenBuffers( 1, &colorBuffer );															// Create a handle for the vertex color buffer
	glBindBuffer( GL_ARRAY_BUFFER, colorBuffer );										// Set the newly created buffer as the current one
	glBufferData( GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW );			// Send the color data to the current buffer

	// >>> @task 1.2

	glGenBuffers( 1, &indexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer );										
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW );			

	// Create the vertex array object
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	glBindBuffer( GL_ARRAY_BUFFER, positionBuffer );	
	glVertexAttribPointer(0, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/ );	
	glBindBuffer( GL_ARRAY_BUFFER, colorBuffer );
	glVertexAttribPointer(1, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/ );
	// >>> @task 1.4

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer );
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	// >>> @task 1.5


	// The loadShaderProgram and linkShaderProgam functions are defined in glutil.cpp and 
	// do exactly what we did in lab1 but are hidden for convenience
	shaderProgram = labhelper::loadShaderProgram("simple.vert", "simple.frag");

	//**********************************************

	//************************************
	//			Load Texture
	//************************************
	// >>> @task 2.1

}

void display(void)
{
	// The viewport determines how many pixels we are rasterizing to
	int w, h;
	SDL_GetWindowSize(g_window, &w, &h);
	glViewport(0, 0, w, h);		// Set viewport

	glClearColor(0.2,0.2,0.8,1.0);						// Set clear color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clears the color buffer and the z-buffer

	// We disable backface culling for this tutorial, otherwise care must be taken with the winding order
	// of the vertices. It is however a lot faster to enable culling when drawing large scenes.
	glDisable(GL_CULL_FACE);
	// Disable depth testing
	glDisable(GL_DEPTH_TEST);
	// Shader Program
	glUseProgram( shaderProgram );				// Set the shader program to use for this draw call

	// Set up a projection matrix
	float fovy = radians(45.0f);
	float aspectRatio = float(w) / float(h);
	float nearPlane = 0.01f;
	float farPlane = 300.0f;
	mat4 projectionMatrix = perspective(fovy, aspectRatio, nearPlane, farPlane);
	// Send it to the vertex shader
	int loc = glGetUniformLocation(shaderProgram, "projectionMatrix");
	glUniformMatrix4fv(loc, 1, false, &projectionMatrix[0].x);

	// >>> @task 3.1

	glBindVertexArray(vertexArrayObject);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


	glUseProgram( 0 ); // "unsets" the current shader program. Not really necessary.
}

void gui() {
	// Inform imgui of new frame
	ImGui_ImplSdlGL3_NewFrame(g_window);

        // ----------------- Set variables --------------------------  
        ImGui::PushID("mag");
        ImGui::Text("Magnification");
        ImGui::RadioButton("GL_NEAREST", &mag, 0);
        ImGui::RadioButton("GL_LINEAR", &mag, 1);
        ImGui::PopID();

        ImGui::PushID("mini");
        ImGui::Text("Minification");
        ImGui::RadioButton("GL_NEAREST", &mini, 0);
        ImGui::RadioButton("GL_LINEAR", &mini, 1);
        ImGui::RadioButton("GL_NEAREST_MIPMAP_NEAREST", &mini, 2);
        ImGui::RadioButton("GL_NEAREST_MIPMAP_LINEAR", &mini, 3);
        ImGui::RadioButton("GL_LINEAR_MIPMAP_NEAREST", &mini, 4);
        ImGui::RadioButton("GL_LINEAR_MIPMAP_LINEAR", &mini, 5);
        ImGui::PopID();

        ImGui::SliderFloat("Anisotropic filtering", &anisotropy, 1.0, 16.0, "Number of samples: %.0f");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	// ----------------------------------------------------------


	// Render the GUI.
	ImGui::Render();
}

int main(int argc, char *argv[])
{
	g_window = labhelper::init_window_SDL("OpenGL Lab 2");

	initGL();

	// render-loop
	bool stopRendering = false;
	while (!stopRendering) {
		// render to window
		display();

                // Render overlay GUI.
                //gui();

		// Swap front and back buffer. This frame will now been displayed.
		SDL_GL_SwapWindow(g_window);			

		// check events (keyboard among other)
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			// Allow ImGui to capture events.
			ImGui_ImplSdlGL3_ProcessEvent(&event);

			if (event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)) {
				stopRendering = true;
			}
		}
	}

	// Shut down everything. This includes the window and all other subsystems.
	labhelper::shutDown(g_window);
	return 0;          
}
