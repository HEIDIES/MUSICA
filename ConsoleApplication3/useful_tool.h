#pragma once
#include "xiaozhi.h"

namespace myMusica {

	static float fast_sqrt(float x) {

		float xhalf = 0.5f * x;
		int i = *(int *)&x;
		i = 0x5f3504f3 - (i >> 1);
		x = *(float*)&i;
		x = x * (1.5f - (xhalf*x*x));
		return 1 / x;

	}

} // namespace myMusica