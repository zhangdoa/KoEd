#pragma once
#include <string>
#include <vector>

namespace KoEd
{
	struct ProductHints
	{
		std::string HU;
		std::string JDX;
		std::string Visibility;
	};

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

	void InitializeEnvironment();

	std::vector<char> readFile(const std::string& inputFilePath);
	void extractNICNT(const std::string& inputFilePath, const std::string& outputDirPath, const std::string& fileName);
	void createNICNT(const std::string& inputFilePath, const std::string& outputDirPath, const std::string& fileName);
	void extractNKX(const std::string& inputFilePath, const std::string& outputDirPath, const std::string& fileName);

	ProductHints findProductHints(const std::string& name);
}
