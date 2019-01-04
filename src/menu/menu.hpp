#pragma once

class Menu {
	
public:

	static void init();
	static void shutdown();
	static void update(int delta);
	static void render();

private:

	static bool server_up;

};