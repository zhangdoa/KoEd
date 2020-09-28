#include "FileParser.h"
#include "../../GitSubmodules/tinyxml2/tinyxml2.h"
#include "../../GitSubmodules/tinyxml2/tinyxml2.cpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_map>

namespace KoEd
{
	std::unordered_map<std::string, ProductHints> productHintsMap;
	tinyxml2::XMLDocument NA;

	void InitializeEnvironment()
	{
		NA.LoadFile("C:\\Program Files\\Common Files\\Native Instruments\\Service Center\\NativeAccess.xml");
		auto elem = NA.RootElement()->FirstChildElement("Product");

		while (elem)
		{
			ProductHints l_ProductHints;

			std::string l_name = elem->FirstChildElement("Name")->GetText();

			if (auto l_ProductSpecific = elem->FirstChildElement("ProductSpecific"))
			{
				if (auto l_HU = l_ProductSpecific->FirstChildElement("HU"))
				{
					l_ProductHints.HU = l_HU->GetText();
				}
				if (auto l_JDX = l_ProductSpecific->FirstChildElement("JDX"))
				{
					l_ProductHints.JDX = l_JDX->GetText();
				}
				if (auto l_Visibility = l_ProductSpecific->FirstChildElement("Visibility"))
				{
					l_ProductHints.Visibility = l_Visibility->GetText();
				}
			}

			productHintsMap.emplace(l_name, l_ProductHints);

			if (elem->NextSiblingElement())
			{
				elem = elem->NextSiblingElement();
			}
			else
			{
				elem = nullptr;
			}
		}
	}

	std::vector<char> readFile(const std::string& inputFilePath)
	{
		std::ifstream is;

		is.open(inputFilePath, std::ios::in | std::ios::ate | std::ios::binary);

		if (!is.is_open())
		{
			std::cerr << "Can't open file : " << inputFilePath << "!";
			return std::vector<char>();
		}

		auto pbuf = is.rdbuf();
		std::size_t l_size = pbuf->pubseekoff(0, is.end, is.in);
		pbuf->pubseekpos(0, is.in);

		std::vector<char> buffer(l_size);
		pbuf->sgetn(&buffer[0], l_size);

		is.close();

		return buffer;
	}

	char* sstrstr(char* haystack, const char* needle, size_t length)
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

	// The file structure of .nicnt:

	// Bin 1 = 256B as:
	// 16B BinNeedle_1
	// 2F 5C 20 4E 49 20 46 43 20 4D 54 44 20 20 2F 5C
	// or "/\ NI FC MTD  /\"
	// 50B padding
	// 10B version string
	// 52B padding
	// 32B static unknown info (00 D0 07 00 = 512000 same as the size of the large padding next)
	// 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00
	// 00 D0 07 00 00 D0 07 00 00 00 00 00 00 00 00 00
	// 96B padding

	// Non-static length ProductHints XML

	// 512000B padding

	// Bin 2 = 288B as:
	// 16B BinNeedle_1
	// 112B padding
	// 32B static unknown info
	// 00 00 00 00 02 00 00 00 00 00 00 00 00 00 00 00
	// 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	// 96B padding
	// 16B static unknown info
	// 01 00 00 00 00 00 00 00 F0 F0 F0 F0 F0 F0 F0 F0
	// 8B the count of sections in Bin 3
	// NN 00 00 00 00 00 00 00
	// 8B variable unknown info
	// VV VV VV 00 00 00 00 00

	// Bin 3 has multiple sub-chunks, 608B + 32B for each chunk, looks like single-linked-list:
	// First chunk:
	// starts with:
	// 16B BinNeedle_2
	// 2F 5C 20 4E 49 20 46 43 20 54 4F 43 20 20 2F 5C
	// or "/\ NI FC TOC  /\"
	// then
	// Then :
	// file extension start with '.', pad until the:
	// 8B variable unknown info
	// VV VV VV 00 00 00 00 00
	// 8B index
	// II 00 00 00 00 00 00 00
	// 16B padding

	// Last chunk
	// ends with:
	// the same 8B variable unknown info at the end of Bin 2
	// VV VV VV 00 00 00 00 00
	// 8B static unknown info
	// F1 F1 F1 F1 F1 F1 F1 F1
	// 16B padding

	// @TODO: analysis the database structure of bin 3

	// Bin 4 = 608B as:
	// 16B BinNeedle_2
	// 592B padding

	// Wallpaper PNG

	// Lib info XML

	void extractNICNT(const std::string& inputFilePath, const std::string& outputDirPath, const std::string& fileName)
	{
		NINCTChunkSize chunkSize;

		// Load .nicnt
		std::ofstream os;
		auto l_buffer = readFile(inputFilePath);
		auto l_size = l_buffer.size();

		auto l_1stBinStartPos = &l_buffer[0];

		// Extract ProductHints
		auto l_ProductHintsStartNeedle = "<?xml";
		auto l_ProductHintsEndNeedle = "</ProductHints>";

		auto l_ProductHintsStartPos = sstrstr(l_1stBinStartPos, l_ProductHintsStartNeedle, l_size);
		auto l_ProductHintsEndPos = sstrstr(l_ProductHintsStartPos, l_ProductHintsEndNeedle, l_size);
		l_ProductHintsEndPos += strlen(l_ProductHintsEndNeedle);

		// Extract other custom bin
		auto l_BinNeedle_1 = "/\\ NI FC MTD  /\\";
		auto l_BinNeedle_2 = "/\\ NI FC TOC  /\\";

		auto l_2ndBinStartPos = sstrstr(l_ProductHintsEndPos, l_BinNeedle_1, l_size);
		auto l_3rdBinStartPos = sstrstr(l_2ndBinStartPos, l_BinNeedle_2, l_size);
		// since the 3rd bin has same needle as the 4th one, we need to ignore the 3rd's
		auto l_4thBinStartPos = sstrstr(l_3rdBinStartPos + sizeof(l_BinNeedle_2), l_BinNeedle_2, l_size);

		// Extract lib info
		auto l_LibStartNeedle = "<?xml";
		auto l_LibEndNeedle = "</soundinfos>";
		auto l_LibInfoStartPos = sstrstr(l_4thBinStartPos, l_LibStartNeedle, l_size);
		auto l_LibInfoEndPos = sstrstr(l_LibInfoStartPos, l_LibEndNeedle, l_size);
		l_LibInfoEndPos += strlen(l_LibEndNeedle);

		// Extract wallpaper PNG
		auto l_PNGStartNeedle = "PNG";
		auto l_PNGEndNeedle = "IEND";
		auto l_PNGStartPos = sstrstr(l_ProductHintsEndPos, l_PNGStartNeedle, l_size);
		if (l_PNGStartPos != nullptr)
		{
			// For the ASCII character 89 before "PNG"
			l_PNGStartPos--;

			auto l_PNGEndPos = sstrstr(l_ProductHintsEndPos, l_PNGEndNeedle, l_size);
			l_PNGEndPos += strlen(l_PNGEndNeedle);

			// From [ASCII89]PNG to IEND + 4B useless chunk data
			chunkSize.Wallpaper = l_PNGEndPos - l_PNGStartPos + 4;
		}

		// Write bin
		chunkSize.Bin1 = l_ProductHintsStartPos - l_1stBinStartPos;
		auto l_1stBin = std::string(&l_buffer[0], chunkSize.Bin1);
		os.open(outputDirPath + fileName + "_Unknown_01.bin", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_1stBin;
		os.close();

		chunkSize.Bin2 = l_3rdBinStartPos - l_2ndBinStartPos;
		auto l_2ndBin = std::string(l_2ndBinStartPos, chunkSize.Bin2);
		os.open(outputDirPath + fileName + "_Unknown_02.bin", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_2ndBin;
		os.close();

		chunkSize.Bin3 = l_4thBinStartPos - l_3rdBinStartPos;
		auto l_3rdBin = std::string(l_3rdBinStartPos, chunkSize.Bin3);
		os.open(outputDirPath + fileName + "_Unknown_03.bin", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_3rdBin;
		os.close();

		if (l_PNGStartPos != nullptr)
		{
			chunkSize.Bin4 = l_PNGStartPos - l_4thBinStartPos;
		}
		else
		{
			chunkSize.Bin4 = l_LibInfoStartPos - l_4thBinStartPos;
		}
		auto l_4thBin = std::string(l_4thBinStartPos, chunkSize.Bin4);
		os.open(outputDirPath + fileName + "_Unknown_04.bin", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_4thBin;
		os.close();

		// Write ProductHints
		chunkSize.ProductHints = l_ProductHintsEndPos - l_ProductHintsStartPos;
		auto l_ProductHints = std::string(l_ProductHintsStartPos, chunkSize.ProductHints);
		os.open(outputDirPath + fileName + "_ProductHints.xml", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_ProductHints;
		os.close();

		// Write wallpaper PNG
		if (l_PNGStartPos != nullptr)
		{
			auto l_Wallpaper = std::string(l_PNGStartPos, chunkSize.Wallpaper);
			os.open(outputDirPath + fileName + "_Wallpaper.png", std::ios::out | std::ios::ate | std::ios::binary);
			os << l_Wallpaper;
			os.close();
		}

		// Write lib info
		chunkSize.LibInfo = l_LibInfoEndPos - l_LibInfoStartPos;
		auto l_LibInfo = std::string(l_LibInfoStartPos, chunkSize.LibInfo);
		os.open(outputDirPath + fileName + "_LibInfo.xml", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_LibInfo;
		os.close();
	}

	void createNICNT(const std::string& inputFilePath, const std::string& outputDirPath, const std::string& fileName)
	{
		std::ofstream os;
		os.open(outputDirPath + fileName + "_export.nicnt", std::ios::out | std::ios::ate | std::ios::binary);

		auto l_buffer = readFile(outputDirPath + fileName + "_Unknown_01.bin");
		os.write(&l_buffer[0], l_buffer.size());

		l_buffer = readFile(outputDirPath + fileName + "_ProductHints.xml");
		os.write(&l_buffer[0], l_buffer.size());

		auto l_size = l_buffer.size();
		auto l_paddingSize = 512000 - l_size;
		std::vector<char> l_padding(l_paddingSize);
		os.write(&l_padding[0], l_padding.size());

		l_buffer = readFile(outputDirPath + fileName + "_Unknown_02.bin");
		os.write(&l_buffer[0], l_buffer.size());
		l_buffer = readFile(outputDirPath + fileName + "_Unknown_03.bin");
		os.write(&l_buffer[0], l_buffer.size());
		l_buffer = readFile(outputDirPath + fileName + "_Unknown_04.bin");
		os.write(&l_buffer[0], l_buffer.size());

		l_buffer = readFile(outputDirPath + fileName + "_Wallpaper.png");
		os.write(&l_buffer[0], l_buffer.size());

		l_buffer = readFile(outputDirPath + fileName + "_LibInfo.xml");
		os.write(&l_buffer[0], l_buffer.size());

		os.close();
	}

	// The file structure of .nkx
	// Bin 1
	// Icon
	// ProductHints
	// Bin 2 53B
	// Lib info
	// Bin 3 261B
	// Wallpaper
	// Bin 4 ~ Product Name

	void extractNKX(const std::string& inputFilePath, const std::string& outputDirPath, const std::string& fileName)
	{
		// Load .nkx
		std::ofstream os;
		auto l_buffer = readFile(inputFilePath);
		auto l_size = l_buffer.size();

		// Extract icon PNG
		auto l_PNGStartNeedle = "PNG";
		auto l_PNGEndNeedle = "IEND";

		auto l_iconPNGStartPos = sstrstr(&l_buffer[0], l_PNGStartNeedle, l_size);

		size_t l_iconPNGLen;
		if (l_iconPNGStartPos != nullptr)
		{
			// For the ASCII character 89 before "PNG"
			l_iconPNGStartPos--;

			auto l_iconPNGEndPos = sstrstr(l_iconPNGStartPos + strlen(l_PNGStartNeedle), l_PNGEndNeedle, l_size);

			// From [ASCII89]PNG to IEND + 4B useless chunk data
			l_iconPNGLen = l_iconPNGEndPos - l_iconPNGStartPos + strlen(l_PNGEndNeedle) + 4;
		}

		// Extract ProductHints
		auto l_XMLNeedle = "<?xml";
		auto l_ProductHintsNeedle = "</ProductHints>";

		auto l_XMLStartPos = sstrstr(&l_buffer[0], l_XMLNeedle, l_size);
		auto l_ProductHintsPos = sstrstr(&l_buffer[0], l_ProductHintsNeedle, l_size);

		// Extract lib info
		auto l_LibEndNeedle = "</soundinfos>";
		auto l_LibInfoStartPos = sstrstr(l_ProductHintsPos, l_XMLNeedle, l_size);
		auto l_LibInfoEndPos = sstrstr(l_ProductHintsPos, l_LibEndNeedle, l_size);

		// Extract wallpaper PNG
		auto l_wallpaperPNGStartPos = sstrstr(l_LibInfoEndPos, l_PNGStartNeedle, l_size);

		size_t l_wallpaperPNGLen;
		size_t l_libInfoLen;
		if (l_wallpaperPNGStartPos != nullptr)
		{
			// For the ASCII character 89 before "PNG"
			l_wallpaperPNGStartPos--;

			auto l_wallpaperPNGEndPos = sstrstr(l_wallpaperPNGStartPos + strlen(l_PNGStartNeedle), l_PNGEndNeedle, l_size);

			// From [ASCII89]PNG to IEND + 4B useless chunk data
			l_wallpaperPNGLen = l_wallpaperPNGEndPos - l_wallpaperPNGStartPos + strlen(l_PNGEndNeedle) + 4;
		}

		l_libInfoLen = l_LibInfoEndPos - l_LibInfoStartPos + strlen(l_LibEndNeedle);

		// Separate string
		auto l_icon = std::string(l_iconPNGStartPos, l_iconPNGLen);
		auto l_ProductHints = std::string(l_XMLStartPos, l_ProductHintsPos - l_XMLStartPos + strlen(l_ProductHintsNeedle));
		auto l_Wallpaper = std::string(l_wallpaperPNGStartPos, l_wallpaperPNGLen);
		auto l_LibInfo = std::string(l_LibInfoStartPos, l_libInfoLen);

		// Write icon PNG
		os.open(outputDirPath + fileName + "_Icon.png", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_icon;
		os.close();

		// Write ProductHints
		os.open(outputDirPath + fileName + "_ProductHints.xml", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_ProductHints;
		os.close();

		// Write lib info
		os.open(outputDirPath + fileName + "_LibInfo.xml", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_LibInfo;
		os.close();

		// Write wallpaper PNG
		os.open(outputDirPath + fileName + "_Wallpaper.png", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_Wallpaper;
		os.close();
	}

	ProductHints findProductHints(const std::string& name)
	{
		auto l_result = productHintsMap.find(name);
		if (l_result != productHintsMap.end())
		{
			return l_result->second;
		}
		return ProductHints();
	}
}