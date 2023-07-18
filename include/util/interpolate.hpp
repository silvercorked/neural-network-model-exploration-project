#pragma once

#include <assert.h>
#include <type_traits>

#include "Numeric.hpp"

namespace Quantization {
	template <Concepts::Numeric F>
	F interpolate(double percent, F min = -1.0, F max = 1.0) {
		assert((percent >= 0.0 && percent <= 1.0, "interpolate first parameter must be between 0.0 and 1.0"));
		if constexpr (Concepts::Floating<F>) {
			return (1.0 - percent) * min + percent * max;
		}
		else {
			double t = (1.0 - percent) * min + percent * max;
			return F(t + 0.5); // if 6.25 => 6.75 -> 6. if 6.75 => 7.25 -> 7. aka, round up if >= X.5, else round down
		}
	};
}
