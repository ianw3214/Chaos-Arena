#include "engine.hpp"

// External library includes
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <stb_image.h>

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
	m_window = SDL_CreateWindow("Chaos Arena", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 500, 500, SDL_WINDOW_OPENGL);
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

	// HARD CODED SHIT
	std::string path = "res/assets/icon.png";
	int width, height;
	// IMAGE LOADING TO EXE ICON
	int req_format = STBI_rgb_alpha;
	int original_format;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &original_format, req_format);
	if (data == NULL) {
		ERR("Couldn't load image: " << path);
		LOG(stbi_failure_reason());
	}

	SDL_Surface * loadSurface = SDL_CreateRGBSurfaceWithFormatFrom((void*)data, width, height, 32, 4 * width, SDL_PIXELFORMAT_RGBA32);
	if (!loadSurface) {
		ERR("Unable to load image: " << path);
		LOG(SDL_GetError());
	}

	SDL_SetWindowIcon(m_window, loadSurface);

	// free the temporary data
	SDL_FreeSurface(loadSurface);
	stbi_image_free(data);

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

void Engine::resize(int width, int height) {
	SDL_SetWindowSize(m_window, width, height);
	glViewport(0, 0, width, height);
	m_screenWidth	= width;
	m_screenHeight	= height;
	Renderer::setScreenSize(width, height);
}
