#pragma once

#include <string>

struct HasName {
	virtual std::string getName() const = 0;
	virtual void setName(std::string n) = 0;
	virtual ~HasName() = default;
};
