#include "shaderLoader.hpp"

#include "utils.hpp"

std::unordered_map<std::string, std::unique_ptr<Shader>> ShaderLoader::shaders;

ShaderRef ShaderLoader::loadShader(std::string vertex, std::string fragment) {
	loadShader(vertex + "-" + fragment, vertex, fragment);
	return shaders[vertex + "-" + fragment];
}

ShaderRef ShaderLoader::loadShader(std::string name, std::string vertex, std::string fragment) {
	shaders[name] = std::make_unique<Shader>(vertex, fragment);
	return shaders[name];
}

ShaderRef ShaderLoader::getShader(std::string name) {
	// Try to load the model with the name as the file path if it doesn't exist yet
	if (shaders.find(name) == shaders.end()) {
		Util::StrTokens tokens = Util::split(name, '-');
		loadShader(tokens[0], tokens[1]);
	}
	return shaders[name];
}
