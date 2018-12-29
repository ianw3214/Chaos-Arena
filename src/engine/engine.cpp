#include "engine.hpp"

// External library includes
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

// Project includes
#include "utils/utils.hpp"
#include "profiler/profiler.hpp"
#include "renderer/renderer.hpp"
#include "textureManager/textureManager.hpp"
#include "renderer/textRenderer.hpp"

// Engine static variables
SDL_Window *	Engine::m_window;
SDL_GLContext	Engine::m_context;
int Engine::m_screenWidth;
int Engine::m_screenHeight;

#include <ctime>

void Engine::init() {

	// Initialize SDL
	int result = SDL_Init(SDL_INIT_EVERYTHING);
	if (result > 0) {
		ERR("Could not initialize SDL")
	}

	// Set openGL attributes
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	// Create SDL window
	// m_window = SDL_CreateWindow("TEST", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 500, 500, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
	m_window = SDL_CreateWindow("TEST", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 500, 500, SDL_WINDOW_OPENGL);
	if (m_window == NULL) {
		ERR("Could not create a window")
	}
	// TODO: (Ian) Set these to actual dimensions of the window
	m_screenWidth = 500;
	m_screenHeight = 500;

	// Create OpenGL context
	m_context = SDL_GL_CreateContext(m_window);
	if (m_context == NULL) {
		ERR("Unable to create OpenGL context")
	}

	// Enable vsync
	SDL_GL_SetSwapInterval(1);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		std::string error_message = "Could not initialize GLEW";
		// glewGetErrorString(glewError);
		ERR(error_message);
	}

	// Setup blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// Initialize subsystems here
	Profiler::init();
	Renderer::init();
	TextureManager::init();
	TextRenderer::init();

	// Initialize random seed
	srand(static_cast<unsigned int>(time(nullptr)));

}

void Engine::shutdown() {
	TextRenderer::shutdown();
	TextureManager::shutdown();
	Renderer::shutdown();
	Profiler::shutdown();

	SDL_Quit();
}

int Engine::getScreenWidth() {
	return m_screenWidth;
}

int Engine::getScreenHeight() {
	return m_screenHeight;
}

void Engine::swapScreenBuffer() {
	SDL_GL_SwapWindow(m_window);
}
