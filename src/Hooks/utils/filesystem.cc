#include "utils/filesystem.h"
#include <fstream>

namespace utils::filesystem {
	bool FileExists(LPCSTR fileName) {
		std::ifstream infile(fileName);
		return infile.good();
	}
}