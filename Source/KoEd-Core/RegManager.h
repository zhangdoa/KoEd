#pragma once
#include <string>
#include <vector>

namespace KoEd
{
	struct LibInfo
	{
		std::wstring name;
		std::wstring path;
	};

	std::vector<LibInfo> GetLibInfos();
	void RegisterLibraries(const std::wstring& kontaktLibPath);
}
