#pragma once

#include <string>

class Texture;

class Sprite {

public:
	
	Sprite(const std::string& name = "");
	~Sprite();

	virtual void render() const;

	const Texture& getTexture() const;
	void setSource(const std::string& name);

	int getOriginalWidth() const;
	int getOriginalHeight() const;

	// Property of the sprite
	int x, y;
	int w, h;

	// Property of the sprite source
	int src_x, src_y;
	int src_w, src_h;

	// Utility methods
	void setPos(int x, int y);
	void setSize(int w, int h);
	void setSourcePos(int x, int y);
	void setSourceSize(int w, int h);

private:

	std::string m_name;

	int original_w;
	int original_h;

};