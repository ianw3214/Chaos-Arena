#include "animatedSprite.hpp"

#include "engine/opengl/texture.hpp"
#include "engine/textureManager/textureManager.hpp"
#include "engine/renderer/renderer.hpp"

AnimatedSprite::AnimatedSprite(const std::string & path, int frame_w, int frame_h) :
	Sprite(path),
	frame_index(0),
	animation_index(0)
{
	// TODO: (Ian) Better errorh handling
	if (path.size() == 0) return;
	if (TextureManager::getTexture(path) == nullptr) return;
	if (frame_w == 0 || frame_h == 0) return;
	// Set the source w/h to be the same as an animation frame
	src_w = frame_w;
	src_h = frame_h;

	// Calculate the number of sprites in the sprite sheet based on the input frame size and texture size
	if (TextureManager::getTexture(path) != nullptr) {
		spriteSheetWidth = TextureManager::getTexture(path)->getWidth() / frame_w;
		spriteSheetHeight = TextureManager::getTexture(path)->getHeight() / frame_h;
	}

	clock.reset();
}

AnimatedSprite::~AnimatedSprite() {
	// Do nothing...
}
void AnimatedSprite::setSource(const std::string & name) {
	Sprite::setSource(name);
	// RESET ANIMATIONS
	frames.clear();
	while (!animations.empty()) animations.pop();
}

void AnimatedSprite::setFrameSize(int width, int height) {
	src_w = width;
	src_h = height;

	// Calculate the number of sprites in the sprite sheet based on the input frame size and texture size
	// Assumes that the getTexture call doesn't fail, meaning that a valid sprite exists
	spriteSheetWidth = getTexture().getWidth() / width;
	spriteSheetHeight = getTexture().getHeight() / height;
}

void AnimatedSprite::render() {
	// Render a magenta square if something goes wrong
	if (frames.size() > 0 && TextureManager::getTexture(getName()) != nullptr) {
		updateFrame();
		Renderer::drawSprite(*this);
	} else {
		Renderer::drawRect({ x, y }, w, h, { 1.f, 0.f, 1.f });
	}
}

void AnimatedSprite::renderWithoutUpdate() {
	Renderer::drawSprite(*this);
}

void AnimatedSprite::addAnimation(unsigned int start, unsigned int end) {
	frames.push_back(Vec2ui{start, end});
}

void AnimatedSprite::updateFrame() {
	// Only update the frame if the timer is up
	// TODO: (Ian) Calculate FPS instead of hard coding it
	if (clock.getTicks() > 33) {
		frame_index++;
		if (frame_index > frames[animation_index].y) {
			// Determine if we are done with the current animation state
			if (!animations.empty()) {
				animations.front().loops--;
				// If we have no loops left, remove this animation state from the sprite
				if (animations.front().loops == 0) {
					animations.pop();
				}
				// If there is still an animation left, use that as the new animation index
				if (!animations.empty()) animation_index = animations.front().animation_index;
			}
			frame_index = frames[animation_index].x;
		}
		updateSourceFromFrame();
		clock.reset();
	}
}

void AnimatedSprite::playAnimation(unsigned int animation, unsigned int loops) {
	// Make sure the animation is valid
	// TODO: (Ian) Better erro handling
	if (animation >= frames.size()) return;

	// Reset the animation queue and set it to the new played animation
	while (!animations.empty()) animations.pop();
	queueAnimation(animation, loops);

	// Actually play the animation by setting it to the right frame
	animation_index = animation;
	frame_index = frames[animation_index].y;
	updateSourceFromFrame();
}

void AnimatedSprite::queueAnimation(unsigned int animation, unsigned int loops) {
	animations.push(AnimationState{ animation, loops });
}

void AnimatedSprite::updateSourceFromFrame() {
	int x = frame_index % spriteSheetWidth;
	int y = frame_index / spriteSheetWidth;

	// Dirty fix for inverted textures
	// TODO: Fix this in engine renderer code
	// y = spriteSheetHeight - y - 1;

	setSourcePos(x * src_w, y * src_h);
}
