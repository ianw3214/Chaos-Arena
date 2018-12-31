#pragma once

#include <queue>
#include <vector>

#include "engine/sprite/sprite.hpp"
#include "engine/utils.hpp"
#include "engine/clock/clock.hpp"

// Structure to represent a queued animation
struct AnimationState {
	unsigned int animation_index;
	unsigned int loops;
};

class AnimatedSprite : public Sprite {

public:
	
	// Constructor of the animated sprite using the width/height of each animation frame
	AnimatedSprite(const std::string& path = "", int frame_w = 0, int frame_h = 0);
	~AnimatedSprite();
	
	virtual void setSource(const std::string& name) override;
	void setFrameSize(int width, int height);

	virtual void render() override;
	void renderWithoutUpdate();
	void addAnimation(unsigned int start, unsigned int end);
	void updateFrame();
	
	void playAnimation(unsigned int animation, unsigned int loops = 1);
	void queueAnimation(unsigned int animation, unsigned int loops = 1);

	// Sprite sheet in size in terms of number of animation frames
	int spriteSheetWidth;
	int spriteSheetHeight;

private:

	// Animated sprite information
	std::vector<Vec2ui> frames;

	// Animated sprite state variables
	Clock clock;
	unsigned int frame_index;
	unsigned int animation_index;

	// Helper function to calculate animation frame coordinates
	void updateSourceFromFrame();

	// Queue of upcoming animations
	mutable std::queue<AnimationState> animations;

};