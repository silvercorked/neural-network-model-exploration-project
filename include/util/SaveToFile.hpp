#pragma once

#include <fstream>
#include <string>

namespace Savers {
	class SaveToFile {
		std::ofstream out;
	public:
		SaveToFile(std::string filename) {
			this->out.open(filename);
		}
		std::ofstream& getStream() {
			return this->out;
		}
		~SaveToFile() {
			this->out.close();
		}
	};
}
