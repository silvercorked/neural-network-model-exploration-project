#pragma once

#include <assert.h>
#include <string>
#include <memory>
#include <random>

#include "RandomFeatureBase.hpp"
#include "HasCurr.hpp"
#include "Domain.hpp"
#include "Numeric.hpp"
#include "Integral.hpp"

template <Concepts::Numeric N>
class RandomFeature : public RandomFeatureBase, HasCurr<N> {
	std::string name;
	N curr;
	Domain<N> domain;
	std::mt19937* gen;

	using DistType = std::conditional<Concepts::Integral<N>, std::uniform_int_distribution<N>, std::uniform_real_distribution<N>>::type;
public:
	RandomFeature(std::string);
	RandomFeature(std::string, N, N);
	RandomFeature(const RandomFeature&);
	~RandomFeature() = default;

	N getCurr() const override;
	bool next() override;

	std::string getName() const override;
	void setName(std::string) override;
	void setDomain(N, N);
	void setGenerator(std::mt19937*);
};
template <Concepts::Numeric N>
RandomFeature<N>::RandomFeature(std::string n)
	: name(n), gen(nullptr)
{
	this->setDomain(0, 1.0);
}
template <Concepts::Numeric N>
RandomFeature<N>::RandomFeature(std::string n, N min, N max)
	: name(n), gen(nullptr)
{
	this->setDomain(min, max);
}
template <Concepts::Numeric N>
RandomFeature<N>::RandomFeature(const RandomFeature<N>& f)
	: name(f.getName()), gen(f.gen), domain(f.domain)
{}
template <Concepts::Numeric N>
N RandomFeature<N>::getCurr() const {
	return this->curr;
}
template <Concepts::Numeric N>
bool RandomFeature<N>::next() {
	if (this->gen == nullptr) return false;
	auto dist = RandomFeature<N>::DistType(this->domain.getMin(), this->domain.getMax());
	this->curr = dist(*this->gen);
	return true;
}
template <Concepts::Numeric N>
std::string RandomFeature<N>::getName() const {
	return this->name;
}
template <Concepts::Numeric N>
void RandomFeature<N>::setName(std::string n) {
	this->name = n;
}
template <Concepts::Numeric N>
void RandomFeature<N>::setDomain(N min, N max) {
	this->domain.setMinAndMax(min, max);
}
template <Concepts::Numeric N>
void RandomFeature<N>::setGenerator(std::mt19937* gen) {
	this->gen = gen;
}
