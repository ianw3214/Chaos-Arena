#include "clock.hpp"

Clock::Clock() :
	m_elapsedTime(0),
	m_timeSeconds(0),
	m_timeScale(1.f),
	m_isPaused(false)
{
	m_lastTime = SDL_GetTicks();
	return;
}

Uint32 Clock::getTicks() {
	addCurrentElapsed();
	return m_elapsedTime;
}

void Clock::pause() {
	m_isPaused = true;
	addCurrentElapsed();
	return;
}

void Clock::unpause() {
	m_isPaused = false;
	return;
}

bool Clock::isPaused() const {
	return m_isPaused;
}

void Clock::setTimeScale(float scale) {
	addCurrentElapsed();
	m_timeScale = scale;
	return;
}

float Clock::getTimeScale() const {
	return m_timeScale;
}

void Clock::addCurrentElapsed() {
	Uint32 current = SDL_GetTicks();
	// First, calculate the amount of time to add to elapsed time based on time scale and elapsed time
	Uint32 raw_elapsed = current - m_lastTime;
	m_elapsedTime += static_cast<Uint32>(m_timeScale * raw_elapsed);
	// Reset m_lastTime to be the current time for the next calculation
	m_lastTime = current;
	return;
}
