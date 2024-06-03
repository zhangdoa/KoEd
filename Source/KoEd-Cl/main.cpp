#pragma once
#include "../KoEd-Core/FileParser.h"
#include "../KoEd-Core/RegManager.h"

using namespace KoEd;

#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

std::wstring charToWstring(const char* charStr) 
{
	if (charStr == nullptr) {
		return std::wstring();
	}

	// Determine the length of the required wide string
	size_t requiredSize = 0;
	mbstowcs_s(&requiredSize, nullptr, 0, charStr, 0);

	// Allocate buffer for the wide string, including the null terminator
	wchar_t* wStr = new wchar_t[requiredSize];

	// Convert the multibyte string to a wide string
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wStr, requiredSize, charStr, _TRUNCATE);

	// Create a std::wstring from the wide string buffer
	std::wstring wString(wStr);

	// Free the allocated buffer
	delete[] wStr;

	return wString;
}

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
	else if (l_option == "-r")
	{
		std::wstring wStr = charToWstring(argv[2]);
		RegisterLibraries(wStr);
	}
	else
	{
		std::cerr << "Unsupported option: " << l_option << "!" << std::endl;
	}

	return 0;
}