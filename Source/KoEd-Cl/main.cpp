#pragma once
#include "../KoEd-Core/FileParser.h"
#include "../KoEd-Core/RegManager.h"

using namespace KoEd;

#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " -[Option] \"FilePath\"" << std::endl;
		return 0;
	}

	InitializeEnvironment();

	std::string l_option = argv[1];
	auto l_inputFilePath = fs::path(argv[2]);
	auto l_inputFilePathStr = l_inputFilePath.generic_string();
	auto l_fileName = l_inputFilePath.stem().generic_string();
	auto l_extension = l_inputFilePath.extension().generic_string();
	auto l_outputDirPathStr = l_inputFilePath.remove_filename().generic_string();

	if (l_option == "-e")
	{
		if (l_extension == ".nicnt")
		{
			extractNICNT(l_inputFilePathStr, l_outputDirPathStr, l_fileName);
		}
		else if (l_extension == ".nkx")
		{
			extractNKX(l_inputFilePathStr, l_outputDirPathStr, l_fileName);
		}
	}
	else if (l_option == "-c")
	{
		if (l_extension == ".nicnt")
		{
			createNICNT(l_inputFilePathStr, l_outputDirPathStr, l_fileName);
		}
	}
	if (l_option == "-r")
	{
		RegisterLibraries(argv[2]);
	}
	else
	{
		std::cerr << "Unsupported option: " << l_option << "!" << std::endl;
	}

	return 0;
}