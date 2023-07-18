#pragma once

#include <type_traits>

namespace Concepts {
	template <typename I>
	concept ByteSizedIntegral = !std::is_same_v<I, bool> && std::is_integral_v<I>;
}
