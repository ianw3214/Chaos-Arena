#include "profiler.hpp"

#include "utils/utils.hpp"
#include <SDL2/SDL.h>

std::unordered_map<std::string, PROFILE_DATA> Profiler::data;

void Profiler::init() {
	// Do nothing...
}

void Profiler::shutdown() {
	// Do nothing...
}

void Profiler::profileStart(std::string name) {
	data[name].start = SDL_GetTicks();
}

int Profiler::profileEnd(std::string name, bool verbose) {
	if (data.find(name) == data.end()) {
		ERR("Could not find name in list of profiles: " << name);
		return -1;
	} else {
		int diff = SDL_GetTicks() - data[name].start;
		if (verbose) LOG(name << " : " << diff << " ms");
		data[name].total += diff;
		data[name].count++;
		return diff;
	}
}

int Profiler::profileAverage(std::string name, bool verbose) {
	if (data.find(name) == data.end()) {
		ERR("Could not find name in list of profiles: " << name);
		return -1;
	} else {
		int average = data[name].total / data[name].count;
		LOG("Average [" << name << "]: " << average << " ms");
		return average;
	}
}
