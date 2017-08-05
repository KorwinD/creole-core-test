#pragma once
#include <string>
#include <map> 
#include <vector>
#include <string>
#include <iostream>
#include <windows.h>
#include "utf8.h"
namespace Creole
{
	class gString
	{
	public:
		//standart string
		static std::vector <wchar_t> St_string(std::vector <wchar_t> utfbuf, std::map <std::string, int> &dict);
	};
}