#pragma once

#include <iostream>
#include <string>

// TODO: Implement more fully featured logger
#define LOG(x) std::cout << "[LOG] " <<  x << std::endl;
#define ERR(x) std::cerr << "[ERROR] " << x << std::endl;
#define ASSERT(x) if(!(x)) __debugbreak();