#pragma once

#include "HasName.hpp"
#include "HasNext.hpp"
#include "HasGenerator.hpp"

struct RandomFeatureBase : HasName, HasNext, HasGenerator  {};
