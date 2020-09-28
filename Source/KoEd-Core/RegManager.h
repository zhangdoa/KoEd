#pragma once
#include <string>
#include <vector>

namespace KoEd
{
	struct LibInfo
	{
		std::string name;
		std::string path;
	};

	std::vector<LibInfo> GetLibInfos();
	void RegisterLibraries(const std::string& kontaktLibPath);
}
