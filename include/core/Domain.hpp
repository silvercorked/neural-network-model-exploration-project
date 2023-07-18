#pragma once

#include <assert.h>

#include "Numeric.hpp"

template<Concepts::Numeric T>
class Domain {
	T min;
	T max;
public:
	Domain() {
		this->min = 0;
		this->setMax(++this->min);
	}
	Domain(T min, T max) {
		assert((min <= max, "Min must be less or equal to max"));
		this->setMinAndMax(min, max);
	}
	~Domain() = default;
	const T getMin() const;
	const T getMax() const;
	bool setMinAndMax(T, T);
	bool setMin(T);
	bool setMax(T);
};

template<Concepts::Numeric T>
const T Domain<T>::getMin() const {
	return this->min;
}
template<Concepts::Numeric T>
const T Domain<T>::getMax() const {
	return this->max;
}
template<Concepts::Numeric T>
bool Domain<T>::setMinAndMax(T min, T max) {
	if (min > max) return false;
	this->min = min;
	this->max = max;
	return true;
}
template<Concepts::Numeric T>
bool Domain<T>::setMin(T min) {
	if (min > this->max) return false;
	this->min = min;
	return true;
}
template<Concepts::Numeric T>
bool Domain<T>::setMax(T max) {
	if (max < this->min) return false;
	this->max = max;
	return true;
}