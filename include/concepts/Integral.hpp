#pragma once

#include <type_traits>

namespace Concepts {
	template <typename I>
	concept Integral = std::is_integral_v<I>;
}
