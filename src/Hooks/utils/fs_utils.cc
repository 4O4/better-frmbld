#include "utils/fs_utils.h"
#include <fstream>

namespace fs_utils {
	bool FileExists(LPCSTR fileName) {
		std::ifstream infile(fileName);
		return infile.good();
	}
}