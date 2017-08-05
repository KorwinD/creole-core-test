#pragma once
#include <string>
#include <map> 
#include <vector>
#include <string>
#include <iostream>
#include "utf8.h"

namespace Creole
{
	class gString
	{
	public:
		//standart string
		static std::vector <wchar_t> St_string(std::vector <wchar_t> utfbuf, std::map <std::string, int> &dict);

		static std::vector <wchar_t> End_file(std::map <std::string, int> &dict);
	};
}