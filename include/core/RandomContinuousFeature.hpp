#pragma once

#include <assert.h>
#include <string>
#include <memory>
#include <random>

#include "Domain.hpp"
#include "GeneratingFeature.hpp"

class RandomContinuousFeature : public GeneratingFeature<double> {
	std::string name;
	double curr;
	Domain<double> domain;
	std::mt19937* gen;
	std::uniform_real_distribution<double> dist;
public:
	RandomContinuousFeature(std::string);
	RandomContinuousFeature(std::string, double, double);
	RandomContinuousFeature(const RandomContinuousFeature&);
	~RandomContinuousFeature() = default;

	double getCurr() const override;
	bool next() override;

	std::string getName() const override;
	void setName(std::string) override;
	void setDomain(double, double);
	void setGenerator(std::mt19937*);
};

RandomContinuousFeature::RandomContinuousFeature(std::string n)
	: name(n), gen(nullptr)
{
	this->setDomain(0, 1.0);
}
RandomContinuousFeature::RandomContinuousFeature(std::string n, double min, double max)
	: name(n), gen(nullptr)
{
	this->setDomain(min, max);
}
RandomContinuousFeature::RandomContinuousFeature(const RandomContinuousFeature& f)
	: name(f.getName()), gen(f.gen), domain(f.domain)
{}

double RandomContinuousFeature::getCurr() const {
	return this->curr;
}
bool RandomContinuousFeature::next() {
	if (this->gen == nullptr) return false;
	auto dist = std::uniform_real_distribution<int64_t>(this->domain.getMin(), this->domain.getMax());
	this->curr = dist(*this->gen);
	return true;
}

std::string RandomContinuousFeature::getName() const {
	return this->name;
}
void RandomContinuousFeature::setName(std::string n) {
	this->name = n;
}
void RandomContinuousFeature::setDomain(double min, double max) {
	assert(min <= max);
	this->min = min;
	this->max = max;
	this->dist = std::uniform_real_distribution<double>(this->min, this->max);
}
void RandomContinuousFeature::setGenerator(std::mt19937* gen) {
	this->gen = gen;
}
