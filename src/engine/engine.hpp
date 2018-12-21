#pragma once

// TODO: Initialize engine from config file

#include <SDL2/SDL.h>

class Engine {
public:

	static void init();
	static void shutdown();

	// Utility methods
	static void swapScreenBuffer();
	
private:

	static SDL_Window * m_window;
	static SDL_GLContext m_context;

};