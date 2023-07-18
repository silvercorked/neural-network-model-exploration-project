#pragma once

#include "Integral.hpp"
#include "Floating.hpp"

namespace Concepts {
	template <typename T>
	concept Numeric = Concepts::Floating<T> || Concepts::Integral<T>;
}
