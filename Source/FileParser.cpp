#include "FileParser.h"

namespace KoEd
{
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

	void extractNICNT(const std::string& inputFilePath, const std::string& outputDirPath, const std::string& fileName)
	{
		NINCTChunkSize chunkSize;

		// Load .nicnt
		std::ofstream os;
		auto l_buffer = readFile(inputFilePath);
		auto l_size = l_buffer.size();

		// Extract ProductHints
		auto l_XMLNeedle = "<?xml";
		auto l_ProductHintsNeedle = "</ProductHints>";

		auto l_XMLStartPos = sstrstr(&l_buffer[0], l_XMLNeedle, l_size);
		auto l_ProductHintsPos = sstrstr(&l_buffer[0], l_ProductHintsNeedle, l_size);

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

		// Extract lib info
		auto l_LibEndNeedle = "</soundinfos>";
		auto l_LibInfoStartPos = sstrstr(l_ProductHintsPos, l_XMLNeedle, l_size);
		auto l_LibInfoEndPos = sstrstr(l_ProductHintsPos, l_LibEndNeedle, l_size);

		if (l_PNGStartPos != nullptr)
		{
			// For the ASCII character 89 before "PNG"
			l_PNGStartPos--;

			auto l_PNGEndPos = sstrstr(l_ProductHintsPos + strlen(l_ProductHintsNeedle), l_PNGEndNeedle, l_size);

			chunkSize.Bin4 = l_PNGStartPos - l_4thBinPos;

			// From [ASCII89]PNG to IEND + 4B useless chunk data
			chunkSize.Wallpaper = l_PNGEndPos - l_PNGStartPos + strlen(l_PNGEndNeedle) + 4;
		}
		else
		{
			chunkSize.Bin4 = l_LibInfoStartPos - l_4thBinPos;
		}

		// Separate string
		chunkSize.Bin1 = l_XMLStartPos - &l_buffer[0];
		chunkSize.ProductHints = l_ProductHintsPos - l_XMLStartPos + strlen(l_ProductHintsNeedle);
		chunkSize.Bin2 = l_3rdBinPos - l_2ndBinPos;
		chunkSize.Bin3 = l_4thBinPos - l_3rdBinPos;

		chunkSize.LibInfo = l_LibInfoEndPos - l_LibInfoStartPos + strlen(l_LibEndNeedle);

		auto l_1stBin = std::string(&l_buffer[0], chunkSize.Bin1);
		auto l_ProductHints = std::string(l_XMLStartPos, chunkSize.ProductHints);
		auto l_2ndBin = std::string(l_2ndBinPos, chunkSize.Bin2);
		auto l_3rdBin = std::string(l_3rdBinPos, chunkSize.Bin3);
		auto l_4thBin = std::string(l_4thBinPos, chunkSize.Bin4);
		auto l_Wallpaper = std::string(l_PNGStartPos, chunkSize.Wallpaper);
		auto l_LibInfo = std::string(l_LibInfoStartPos, chunkSize.LibInfo);

		// Write bin
		os.open(outputDirPath + fileName + "_Unknown_01.bin", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_1stBin;
		os.close();

		os.open(outputDirPath + fileName + "_Unknown_02.bin", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_2ndBin;
		os.close();

		os.open(outputDirPath + fileName + "_Unknown_03.bin", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_3rdBin;
		os.close();

		os.open(outputDirPath + fileName + "_Unknown_04.bin", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_4thBin;
		os.close();

		// Write ProductHints
		os.open(outputDirPath + fileName + "_ProductHints.xml", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_ProductHints;
		os.close();

		// Write wallpaper PNG
		os.open(outputDirPath + fileName + "_Wallpaper.png", std::ios::out | std::ios::ate | std::ios::binary);
		os << l_Wallpaper;
		os.close();

		// Write lib info
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
}