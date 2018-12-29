#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "../opengl/shader.hpp"

typedef const std::unique_ptr<Shader>& ShaderRef;

class ShaderLoader {
public:

	static ShaderRef loadShader(std::string vertex, std::string fragment);
	static ShaderRef loadShader(std::string name, std::string vertex, std::string fragment);

	static ShaderRef getShader(std::string name);

private:
	static std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;

};