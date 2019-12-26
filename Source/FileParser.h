#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

namespace KoEd
{
	struct NINCTChunkSize
	{
		size_t Bin1 = 0;
		size_t ProductHints = 0;
		size_t Bin2 = 0;
		size_t Bin3 = 0;
		size_t Bin4 = 0;
		size_t Wallpaper = 0;
		size_t LibInfo = 0;
	};

	std::vector<char> readFile(const std::string& inputFilePath);
	void extractNICNT(const std::string& inputFilePath, const std::string& outputDirPath, const std::string& fileName);
	void createNICNT(const std::string& inputFilePath, const std::string& outputDirPath, const std::string& fileName);
	void extractNKX(const std::string& inputFilePath, const std::string& outputDirPath, const std::string& fileName);
}
