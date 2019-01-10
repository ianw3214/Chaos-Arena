#pragma once

#include "engine/core.hpp"

// Constants
#define NUMBER_WIDTH	50
#define NUMBER_HEIGHT	60
#define NUMBER_SRC		"res/assets/UI/numbers.png"

namespace Number {

	// render a right aligned number at a certain screen coordinate
	inline void renderNumber(int number, int x = 0, int y = 0) {
		Sprite sprite(NUMBER_SRC);
		sprite.setSize(NUMBER_WIDTH, NUMBER_HEIGHT);
		sprite.setSourceSize(NUMBER_WIDTH, NUMBER_HEIGHT);
		if (number == 0) {
			sprite.setPos(x, y);
			sprite.setSourcePos(0, 0);
			sprite.render();
		} else {
			int current_x = x;
			while (number > 0) {
				// Render the number
				int num = number % 10;
				sprite.setPos(current_x, y);
				sprite.setSourcePos(num * NUMBER_WIDTH, 0);
				sprite.render();
				// Update variables
				number /= 10;
				current_x -= NUMBER_WIDTH;
			}
		}
	}

}