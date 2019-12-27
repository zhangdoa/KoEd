#include "RegManager.h"
#include "../../GitSubmodules/WinReg/WinReg/WinReg/WinReg.hpp"

using namespace winreg;

namespace KoEd
{
	std::vector<LibInfo> GetLibInfos()
	{
		const std::wstring testSubKey = L"SOFTWARE\\Native Instruments";
		RegKey key{ HKEY_CURRENT_USER, testSubKey };

		auto l_subkeys = key.EnumSubKeys();

		std::vector<LibInfo> l_result;
		l_result.reserve(l_subkeys.size());

		for (auto i : l_subkeys)
		{
			LibInfo l_libInfo;
			l_libInfo.name = std::string(i.begin(), i.end());

			RegKey key_path{ HKEY_CURRENT_USER, testSubKey + L"\\" + i };
			auto l_path = key_path.GetStringValue(L"ContentDir");

			l_libInfo.path = std::string(l_path.begin(), l_path.end());

			l_result.emplace_back(l_libInfo);
		}

		return l_result;
	}
}