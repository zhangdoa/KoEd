#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#ifdef __GNUC__
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

char *sstrstr(char *haystack, const char *needle, size_t length)
{
	size_t needle_length = strlen(needle);
	size_t i;
	for (i = 0; i < length; i++) {
		if (i + needle_length > length) {
			return NULL;
		}
		if (strncmp(&haystack[i], needle, needle_length) == 0) {
			return &haystack[i];
		}
	}
	return NULL;
}

void parseContent(const std::string& inputFilePath, const std::string& outputDirPath, const std::string& fileName)
{
	// Load nicnt
	std::ifstream is;
	std::ofstream os;

	is.open(inputFilePath, std::ios::in | std::ios::ate | std::ios::binary);

	if (!is.is_open())
	{
		std::cout << "Can't open file : " << inputFilePath << "!";
		return;
	}

	auto pbuf = is.rdbuf();
	std::size_t l_size = pbuf->pubseekoff(0, is.end, is.in);
	pbuf->pubseekpos(0, is.in);

	std::vector<char> buffer(l_size);
	pbuf->sgetn(&buffer[0], l_size);

	//// Parsing char array
	// Extract ProductHints
	auto l_xmlNeedle = "<?xml";
	auto l_ProductHintsNeedle = "</ProductHints>";

	auto p1 = sstrstr(&buffer[0], l_xmlNeedle, l_size);
	auto p2 = sstrstr(&buffer[0], l_ProductHintsNeedle, l_size);
	auto l_len1 = p2 - p1 + strlen(l_ProductHintsNeedle);

	auto l_ProductHints = std::string(p1, l_len1);

	// Write ProductHints
	os.open(outputDirPath + fileName + "_ProductHints.xml");
	os << l_ProductHints;
	os.close();

	// Extract wallpaper PNG
	auto l_PNGStartNeedle = "PNG";
	auto l_PNGEndNeedle = "IEND";

	auto p3 = sstrstr(p2 + strlen(l_ProductHintsNeedle), l_PNGStartNeedle, l_size) - 1;
	auto p4 = sstrstr(p2 + strlen(l_ProductHintsNeedle), l_PNGEndNeedle, l_size);
	auto l_len2 = p4 - p3 + strlen(l_PNGEndNeedle) + 4;

	// Write wallpaper PNG
	auto l_PNG = std::string(p3, l_len2);

	os.open(outputDirPath + fileName + "_Wallpaper.png", std::ios::binary);
	os << l_PNG;
	os.close();

	// Extract library dir
	auto p5 = p4 + strlen(l_PNGEndNeedle) + 4;
	auto l_p5Pos = p5 - &buffer[0];
	auto l_LibInfo = std::string(p5, l_size - l_p5Pos);

	os.open(outputDirPath + fileName + "_LibInfo.xml");
	os << l_LibInfo;
	os.close();
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " Input" << argv[1] << std::endl;
		return 0;
	}

	auto l_inputFilePath = fs::path(argv[1]);
	auto l_inputFilePathStr = l_inputFilePath.generic_string();
	auto l_fileName = l_inputFilePath.stem().generic_string();
	auto l_outputDirPath = l_inputFilePath.remove_filename().generic_string();

	parseContent(l_inputFilePathStr, l_outputDirPath, l_fileName);

	return 0;
}