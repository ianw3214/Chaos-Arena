#pragma once

#include <unordered_map>
#include <string>

// profiling tools
struct PROFILE_DATA {
	unsigned int start;
	unsigned int total;
	unsigned int count;
};

class Profiler {

public:

	static void init();
	static void shutdown();

	static void profileStart(std::string name);
	static int  profileEnd(std::string name, bool verbose = false);
	static int  profileAverage(std::string name, bool verbose = false);

private:
	static std::unordered_map<std::string, PROFILE_DATA> data;
};