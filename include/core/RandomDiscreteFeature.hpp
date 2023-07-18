#pragma once

#include <assert.h>
#include <string>
#include <memory>
#include <random>

#include <stdio.h>

#include "Domain.hpp"
#include "GeneratingFeature.hpp"

class RandomDiscreteFeature : public GeneratingFeature<int64_t> {
	std::string name;
	int64_t curr;
	Domain<int64_t> domain;
	std::mt19937* gen;
public:
	RandomDiscreteFeature(std::string);
	RandomDiscreteFeature(std::string, int64_t, int64_t);
	RandomDiscreteFeature(const RandomDiscreteFeature&);
	~RandomDiscreteFeature() = default;

	int64_t getCurr() const override;
	bool next() override;

	std::string getName() const override;
	void setName(std::string) override;
	void setDomain(int64_t, int64_t);
	void setGenerator(std::mt19937*) override;
};

RandomDiscreteFeature::RandomDiscreteFeature(std::string n)
	: name(n), gen(nullptr)
{
	this->setDomain(0, 1.0);
}
RandomDiscreteFeature::RandomDiscreteFeature(std::string n, int64_t min, int64_t max)
	: name(n), gen(nullptr)
{
	this->setDomain(min, max);
}
RandomDiscreteFeature::RandomDiscreteFeature(const RandomDiscreteFeature& f)
	: name(f.getName()), gen(f.gen), domain(f.domain)
{}

int64_t RandomDiscreteFeature::getCurr() const {
	return this->curr;
}
bool RandomDiscreteFeature::next() {
	if (this->gen == nullptr) return false;
	auto dist = std::uniform_int_distribution<int64_t>(this->domain.getMin(), this->domain.getMax());
	this->curr = dist(*this->gen);
	return true;
}

std::string RandomDiscreteFeature::getName() const {
	return this->name;
}
void RandomDiscreteFeature::setName(std::string n) {
	this->name = n;
}
void RandomDiscreteFeature::setDomain(int64_t min, int64_t max) {
	this->domain.setMinAndMax(min, max);
}
void RandomDiscreteFeature::setGenerator(std::mt19937* gen) {
	this->gen = gen;
}
