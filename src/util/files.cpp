
#include "util/util.h"

#include <fstream>

using namespace std;

namespace pd2hook
{
namespace Util{

	string GetFileContents(const string& filename){
		ifstream t(filename, std::ifstream::binary);
		string str;

		t.seekg(0, std::ios::end);
		str.reserve(static_cast<string::size_type>(t.tellg()));
		t.seekg(0, std::ios::beg);
		str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

		return str;
	}

	// FIXME this function really should return a boolean, if it succeeded
	void EnsurePathWritable(const std::string& path){
		int finalSlash = path.find_last_of('/');
		std::string finalPath = path.substr(0, finalSlash);
		if (DirectoryExists(finalPath)) return;
		CreateDirectoryPath(finalPath);
	}

	bool CreateDirectoryPath(const std::string& path){
		std::string newPath = "";
		std::vector<std::string> paths = Util::SplitString(path, '/');
		for (const auto& i : paths) {
			newPath = newPath + i + "/";
			CreateDirectorySingle(newPath);
		}
		return true;
	}

	void SplitString(const std::string &s, char delim, std::vector<std::string> &elems) {
		std::istringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			if (!item.empty()){
				elems.push_back(item);
			}
		}
	}

	std::vector<std::string> SplitString(const std::string &s, char delim) {
		std::vector<std::string> elems;
		SplitString(s, delim, elems);
		return elems;
	}

}}
