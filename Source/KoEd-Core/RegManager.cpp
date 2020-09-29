#include "RegManager.h"
#include "../../GitSubmodules/WinReg/WinReg/WinReg/WinReg.hpp"
#include "FileParser.h"

using namespace winreg;

#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

namespace KoEd
{
	std::vector<LibInfo> GetLibInfos()
	{
		const std::wstring keyFolder = L"SOFTWARE\\Native Instruments";
		RegKey key{ HKEY_CURRENT_USER, keyFolder };

		auto l_subKeys = key.EnumSubKeys();

		std::vector<LibInfo> l_result;
		l_result.reserve(l_subKeys.size());

		for (auto i : l_subKeys)
		{
			LibInfo l_libInfo;
			l_libInfo.name = std::string(i.begin(), i.end());

			RegKey key_path{ HKEY_CURRENT_USER, keyFolder + L"\\" + i };
			auto l_path = key_path.GetStringValue(L"ContentDir");

			l_libInfo.path = std::string(l_path.begin(), l_path.end());

			l_result.emplace_back(l_libInfo);
		}

		return l_result;
	}

	void ClearCurrentUserRegKeys(const std::string& kontaktLibPath)
	{
		RegKey key{ HKEY_CURRENT_USER, L"SOFTWARE\\Native Instruments" };

		auto l_subKeys = key.EnumSubKeys();

		for (auto i : l_subKeys)
		{
			if (i != L"AddLibSupport" && i != L"ALSupport" && i != L"Kontakt 5" && i != L"Kontakt Application" && i != L"Native Access")
			{
				key.DeleteKey(i, KEY_READ | KEY_WRITE);
			}
		}

		key.Close();
	}

	void ClearLocalMachineRegKeys(const std::string& kontaktLibPath)
	{
		RegKey key{ HKEY_LOCAL_MACHINE,  L"SOFTWARE\\Native Instruments" };

		auto l_subKeys = key.EnumSubKeys();

		for (auto i : l_subKeys)
		{
			if (i == L"Content")
			{
				RegKey key_content{ key.Get(), L"Content" };
				auto l_values = key_content.EnumValues();
				for (auto j : l_values)
				{
					key_content.DeleteValue(j.first);
				}
			}
			else if (i != L"Kontakt 5" && i != L"Kontakt Application")
			{
				key.DeleteKey(i, KEY_READ | KEY_WRITE);
			}
		}

		key.Close();
	}

	void ClearRegKeys(const std::string& kontaktLibPath)
	{
		ClearCurrentUserRegKeys(kontaktLibPath);
		ClearLocalMachineRegKeys(kontaktLibPath);
	}

	void AddValuesForKeyInCurrentUser(RegKey& parent, const std::wstring& kontaktLibPath, int32_t index)
	{
		parent.SetDwordValue(L"UserListIndex", index);
		parent.SetDwordValue(L"UserRemoved", 0);
	}

	void AddValuesForKeyInLocalMachine(RegKey& parent, const std::wstring& kontaktLibPath)
	{
		auto l_libPath = kontaktLibPath;
		std::replace(l_libPath.begin(), l_libPath.end(), '/', '\\');
		parent.SetStringValue(L"ContentDir", l_libPath);

		auto l_kontaktLibPath = fs::path(kontaktLibPath);
		ProductHints l_productHints;

		for (auto& i : fs::directory_iterator(l_kontaktLibPath))
		{
			if (i.path().extension() == ".nicnt")
			{
				l_productHints = findProductHints(i.path().stem().generic_string());
				if (!l_productHints.HU.size())
				{
					l_productHints = extractProductHints(i.path().generic_string());
				}
				break;
			}
			else if (i.path().extension() == ".nkx")
			{
			}
		}

		parent.SetStringValue(L"ContentVersion", L"1.0.0");
		if (l_productHints.HU.size())
		{
			parent.SetStringValue(L"HU", std::wstring(l_productHints.HU.begin(), l_productHints.HU.end()));
		}
		if (l_productHints.HU.size())
		{
			parent.SetStringValue(L"JDX", std::wstring(l_productHints.JDX.begin(), l_productHints.JDX.end()));
		}

		parent.SetDwordValue(L"Visibility", 3);
	}

	void RegisterLibraries(const std::string& kontaktLibPath)
	{
		//AppData\Local\Native Instruments\Kontakt\Db
		//AppData\Local\Native Instruments\Kontakt\LibrariesCache
		//Program Files\Common Files\Native Instruments\Service Center
		ClearRegKeys(kontaktLibPath);

		auto l_kontaktLibPath = fs::path(kontaktLibPath);

		int32_t index = 0;
		for (auto& i : fs::directory_iterator(l_kontaktLibPath))
		{
			RegKey key_current_user{ HKEY_CURRENT_USER, L"SOFTWARE\\Native Instruments" };
			key_current_user.Create(key_current_user.Get(), i.path().stem().generic_wstring());
			AddValuesForKeyInCurrentUser(key_current_user, i.path().generic_wstring(), index);

			RegKey key_local_machine{ HKEY_LOCAL_MACHINE, L"SOFTWARE\\Native Instruments" };
			key_local_machine.Create(key_local_machine.Get(), i.path().stem().generic_wstring());
			AddValuesForKeyInLocalMachine(key_local_machine, i.path().generic_wstring());

			index++;
		}
	}
}