#pragma once

#include <random>
#include <mutex>
#include <chrono>

namespace Singleton { // need to declare mt19937 instance with thread_local to create 1 per thread rather than use a singleton
	class mt19937Singleton {
		static mt19937Singleton* instance;
		static std::mutex mutex;
	protected:
		std::mt19937 generator;
	public:
		mt19937Singleton() {
			this->generator = std::mt19937(
				std::chrono::system_clock::now()
				.time_since_epoch()
				.count()
			);
		}
		~mt19937Singleton() {}
		mt19937Singleton(mt19937Singleton&) = delete;
		void operator=(const mt19937Singleton&) = delete;
		static mt19937Singleton* getInstance();
		float_t getRandom() {
			return this->generator();
		}
		float_t applyDistribution(std::uniform_real_distribution<float_t>& dist) {
			return dist(this->generator);
		}
	};

	mt19937Singleton* mt19937Singleton::instance = nullptr;
	std::mutex mt19937Singleton::mutex;

	mt19937Singleton* mt19937Singleton::getInstance() {
		std::lock_guard<std::mutex> lock = std::lock_guard<std::mutex>(mutex);
		if (instance == nullptr)
			instance = new mt19937Singleton();
		return instance;
	}
}