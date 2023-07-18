#pragma once

#include "HasName.hpp"
#include "HasNext.hpp"
#include "HasReset.hpp"

struct FeatureBase : HasName, HasNext, HasReset {};
