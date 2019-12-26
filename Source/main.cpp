#pragma once
#include "FileParser.h"
using namespace KoEd;

#ifdef __GNUC__
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " -[Option] \"FilePath\"" << std::endl;
		return 0;
	}

	std::string l_option = argv[1];
	auto l_inputFilePath = fs::path(argv[2]);
	auto l_inputFilePathStr = l_inputFilePath.generic_string();
	auto l_fileName = l_inputFilePath.stem().generic_string();
	auto l_extension = l_inputFilePath.extension().generic_string();
	auto l_outputDirPath = l_inputFilePath.remove_filename().generic_string();

	if (l_option == "-e")
	{
		if (l_extension == ".nicnt")
		{
			extractNICNT(l_inputFilePathStr, l_outputDirPath, l_fileName);
		}
		else if (l_extension == ".nkx")
		{
			extractNKX(l_inputFilePathStr, l_outputDirPath, l_fileName);
		}
	}
	else if (l_option == "-c")
	{
		if (l_extension == ".nicnt")
		{
			createNICNT(l_inputFilePathStr, l_outputDirPath, l_fileName);
		}
	}
	else
	{
		std::cerr << "Unsupported option: " << l_option << "!" << std::endl;
	}

	return 0;
}