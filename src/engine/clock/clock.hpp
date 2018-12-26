#pragma once

#include "SDL2/SDL.h"

class Clock {

public:

	explicit Clock();

	Uint32 getTicks();

	void pause();
	void unpause();
	bool isPaused() const;
	void setTimeScale(float scale);
	float getTimeScale() const;

private:
	Uint32  m_lastTime;
	Uint32  m_elapsedTime;
	Uint32	m_timeSeconds;
	float	m_timeScale;
	bool	m_isPaused;

	// Helper function to add the current elapsed time to m_elapsedTime
	void addCurrentElapsed();

};