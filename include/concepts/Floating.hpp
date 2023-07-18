#pragma once

#include <type_traits>

namespace Concepts {
	template <typename F>
	concept Floating = std::is_floating_point_v<F>;
}
