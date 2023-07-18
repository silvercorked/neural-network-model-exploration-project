#pragma once

#include <string>

#include "FeatureBase.hpp"
#include "HasCurr.hpp"
#include "Domain.hpp"
#include "interpolate.hpp"

class ContinuousFeature : public FeatureBase, public HasCurr<double> {
    std::string name;
    uint64_t numer;
    uint64_t denom;
    Domain<double> domain;

public:
	ContinuousFeature(std::string);
    ContinuousFeature(std::string, double, double);
    ContinuousFeature(const ContinuousFeature&);
	~ContinuousFeature() = default;

    std::string getName() const override;
	void setName(std::string) override;

    Domain<double> getDomain() const;
	void setDomain(double, double);

	uint64_t getNumerator() const;
	void setNumerator(uint64_t);

	uint64_t getDenominator() const;
	void setDenominator(uint64_t);

	void setNumeratorAndDenominator(uint64_t, uint64_t);
	
	double getCurr() const override;
	bool next() override;
	void reset() override;
};

ContinuousFeature::ContinuousFeature(std::string name)
    : name(name)
    , numer(0)
    , denom(2)
    , domain(Domain<double>(0,1))
{}
ContinuousFeature::ContinuousFeature(std::string name, double min, double max)
    : name(name)
    , numer(0)
    , denom(2)
    , domain(Domain<double>(min, max))
{}
ContinuousFeature::ContinuousFeature(const ContinuousFeature& f)
	: name(f.getName())
	, domain(Domain<double>(f.domain.getMin(), f.domain.getMax()))
	, numer(f.getNumerator())
	, denom(f.getDenominator())
{}

std::string ContinuousFeature::getName() const {
	return this->name;
}
void ContinuousFeature::setName(std::string n) {
	this->name = n;
}
Domain<double> ContinuousFeature::getDomain() const {
	return this->domain;
}
void ContinuousFeature::setDomain(double min, double max) {
	this->domain.setMinAndMax(min, max);
}
uint64_t ContinuousFeature::getNumerator() const {
	return this->numer;
}
void ContinuousFeature::setNumerator(uint64_t nNumer) {
	assert(nNumer <= this->denom);
	this->numer = nNumer;
}
uint64_t ContinuousFeature::getDenominator() const {
	return this->denom;
}
void ContinuousFeature::setDenominator(uint64_t nDenom) {
	assert(nDenom >= this->numer && nDenom != 0);
	this->denom = nDenom;
}
void ContinuousFeature::setNumeratorAndDenominator(uint64_t nNumer, uint64_t nDenom) {
	assert(nNumer <= nDenom && nDenom != 0);
	this->numer = nNumer;
	this->denom = nDenom;
}

void ContinuousFeature::reset() {
	this->numer = 0;
}
double ContinuousFeature::getCurr() const {
	double percent = this->numer / ((double) this->denom);
	return Quantization::interpolate(percent, this->domain.getMin(), this->domain.getMax());
}
bool ContinuousFeature::next() {
	bool canNext = !(this->numer + 1 > this->denom || this->denom - 1 < this->numer);
	if (canNext)
		this->numer++; // hit all
		//this->numer = (((this->numer >> 1) + 1) << 1) | 0b1; // trick to find next add increasing. shift off 0th bit, increment, then add 0th bit as 1 back
	return canNext;
}
