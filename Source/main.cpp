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

// The file structure of .nicnt
// Bin 1 256 B
// ProductHints + padding 512000 B
// Bin 2 288 B
// Bin 3 1920 B
// Bin 4 608 B
// Wallpaper
// Lib info

void parseContent(const std::string& inputFilePath, const std::string& outputDirPath, const std::string& fileName)
{
	// Load .nicnt
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
	auto l_XMLNeedle = "<?xml";
	auto l_ProductHintsNeedle = "</ProductHints>";

	auto l_XMLStartPos = sstrstr(&buffer[0], l_XMLNeedle, l_size);
	auto l_ProductHintsPos = sstrstr(&buffer[0], l_ProductHintsNeedle, l_size);

	// Extract NI's custom bin
	// "2F 5C 20 4E 49 20 46 43  20 4D 54 44 20 20 2F 5C"
	// "/\ NI FC MTD  /\"
	// "2F 5C 20 4E 49 20 46 43  20 54 4F 43 20 20 2F 5C"
	// "/\ NI FC TOC  /\"
	auto l_BinNeedle_1 = "NI FC MTD";
	auto l_BinNeedle_2 = "NI FC TOC";

	auto l_2ndBinPos = sstrstr(l_XMLStartPos, l_BinNeedle_1, l_size);
	l_2ndBinPos -= 3;
	auto l_3rdBinPos = sstrstr(l_2ndBinPos, l_BinNeedle_2, l_size);
	l_3rdBinPos -= 3;
	auto l_4thBinPos = sstrstr(l_3rdBinPos + 16, l_BinNeedle_2, l_size);
	l_4thBinPos -= 3;

	// Extract wallpaper PNG
	auto l_PNGStartNeedle = "PNG";
	auto l_PNGEndNeedle = "IEND";

	auto l_PNGStartPos = sstrstr(l_ProductHintsPos + strlen(l_ProductHintsNeedle), l_PNGStartNeedle, l_size);

	// For the ASCII character 89 before "PNG"
	l_PNGStartPos--;

	auto l_PNGEndPos = sstrstr(l_ProductHintsPos + strlen(l_ProductHintsNeedle), l_PNGEndNeedle, l_size);

	// Extract lib info
	auto l_LibInfoPos = sstrstr(l_ProductHintsPos, l_XMLNeedle, l_size);

	// Separate string
	NINCTChunkSize chunkSize;

	chunkSize.Bin1 = l_XMLStartPos - &buffer[0];
	chunkSize.ProductHints = l_ProductHintsPos - l_XMLStartPos + strlen(l_ProductHintsNeedle);
	chunkSize.Bin2 = l_3rdBinPos - l_2ndBinPos;
	chunkSize.Bin3 = l_4thBinPos - l_3rdBinPos;
	chunkSize.Bin4 = l_PNGStartPos - l_4thBinPos;
	chunkSize.Wallpaper = l_PNGEndPos - l_PNGStartPos + strlen(l_PNGEndNeedle) + 4;
	chunkSize.LibInfo = l_size - (l_LibInfoPos - &buffer[0]);

	auto l_1stBin = std::string(&buffer[0], chunkSize.Bin1);
	auto l_ProductHints = std::string(l_XMLStartPos, chunkSize.ProductHints);
	auto l_2ndBin = std::string(l_2ndBinPos, chunkSize.Bin2);
	auto l_3rdBin = std::string(l_3rdBinPos, chunkSize.Bin3);
	auto l_4thBin = std::string(l_4thBinPos, chunkSize.Bin4);
	auto l_Wallpaper = std::string(l_PNGStartPos, chunkSize.Wallpaper);
	auto l_LibInfo = std::string(l_LibInfoPos, chunkSize.LibInfo);

	// Write bin
	os.open(outputDirPath + fileName + "_Unknown_01.bin", std::ios::binary);
	os << l_1stBin;
	os.close();

	os.open(outputDirPath + fileName + "_Unknown_02.bin", std::ios::binary);
	os << l_2ndBin;
	os.close();

	os.open(outputDirPath + fileName + "_Unknown_03.bin", std::ios::binary);
	os << l_3rdBin;
	os.close();

	os.open(outputDirPath + fileName + "_Unknown_04.bin", std::ios::binary);
	os << l_4thBin;
	os.close();

	// Write ProductHints
	os.open(outputDirPath + fileName + "_ProductHints.xml");
	os << l_ProductHints;
	os.close();

	// Write wallpaper PNG
	os.open(outputDirPath + fileName + "_Wallpaper.png", std::ios::binary);
	os << l_Wallpaper;
	os.close();

	// Write lib info
	os.open(outputDirPath + fileName + "_LibInfo.xml");
	os << l_LibInfo;
	os.close();

	// Write size info
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