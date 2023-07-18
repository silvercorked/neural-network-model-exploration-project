#pragma once

#include <string>
#include <assert.h>

#include "FeatureBase.hpp"
#include "HasCurr.hpp"
#include "Domain.hpp"

class DiscreteFeature : public FeatureBase, public HasCurr<int64_t> {
	std::string name;
	int64_t curr;
	Domain<int64_t> domain;

public:
	DiscreteFeature(std::string);
	DiscreteFeature(std::string, int64_t, int64_t);
	DiscreteFeature(const DiscreteFeature&);
	~DiscreteFeature() = default;

	std::string getName() const override;
	void setName(std::string) override;
	Domain<int64_t> getDomain() const;
	void setDomain(int64_t, int64_t);
	
	int64_t getCurr() const override;
	bool next() override;
	void reset() override;
};

DiscreteFeature::DiscreteFeature(std::string n)
	: name(n)
    , curr(0)
    , domain(Domain<int64_t>(0, 1))
{}
DiscreteFeature::DiscreteFeature(std::string n, int64_t min, int64_t max)
	: name(n)
    , curr(0)
    , domain(Domain<int64_t>(min, max))
{}
DiscreteFeature::DiscreteFeature(const DiscreteFeature& f)
	: name(f.getName())
    , curr(f.getCurr())
    , domain(Domain<int64_t>(f.domain.getMin(), f.domain.getMax()))
{}

std::string DiscreteFeature::getName() const {
	return this->name;
}
void DiscreteFeature::setName(std::string n) {
	this->name = n;
}
Domain<int64_t> DiscreteFeature::getDomain() const {
	return this->domain;
}
void DiscreteFeature::setDomain(int64_t min, int64_t max) {
	this->domain.setMinAndMax(min, max);
}

void DiscreteFeature::reset() {
	this->curr = this->domain.getMin();
}
int64_t DiscreteFeature::getCurr() const {
	return this->curr;
}
bool DiscreteFeature::next() {
	bool canNext = !(this->curr == this->domain.getMax());
	if (canNext)
		this->curr++;
	return canNext;
}
