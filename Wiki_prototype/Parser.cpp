#include "Header.h"
#include <stdexcept>
#include <ctype.h>
#pragma warning(disable : 4996)

using namespace std;

vector <wchar_t *> protocols =
{
	{ L"https" },
	{ L"http" },
	{ L"ftp" },
	{ L"mailto" },
};

vector <wchar_t> allowed_symbols = 
{
	{'!', '(', ')', ';', '@', '&', '+', '$', '?', '-', '_', '.', wchar_t(27) /* ' */}
};

wchar_t *https = L"https";

std::string to_utf8(const wchar_t* buffer, int len)
{
	int nChars = ::WideCharToMultiByte(
		CP_UTF8,
		0,
		buffer,
		len,
		NULL,
		0,
		NULL,
		NULL);
	if (nChars == 0) return "";

	string newbuffer;
	newbuffer.resize(nChars);
	::WideCharToMultiByte(
		CP_UTF8,
		0,
		buffer,
		len,
		const_cast< char* >(newbuffer.c_str()),
		nChars,
		NULL,
		NULL);

	return newbuffer;
}

int wctcmp(vector <wchar_t> a, wchar_t *b)
{
	for (int i = 0; i < min(a.size(), wcslen(b)); i++)
	{
		if (a[i] != b[i]) return -1;
	}
	if (a.size() != wcslen(b)) return 1;
	else return 0;
}

void insert(vector <wchar_t> &str, int shift, wchar_t *add, int len)
{
	for (int i = 0; i < len; i++)
	{
		str.insert(str.end() - shift, add[i]);
	}
}

void link_error1(vector <wchar_t> &new_str, vector <wchar_t> word, map <string, int> &dict)
{
	new_str.push_back(wchar_t('['));
	new_str.push_back(wchar_t('['));
	if (dict["link"] == 2)
	{
		for (int i = 0; i < word.size(); i++)
		{
			new_str.push_back(word[i]);
		}
	}
	dict["link"] = 0;
}

void link_end(vector <wchar_t> &new_str, vector <wchar_t> word, map <string, int> &dict)
{
	new_str.push_back(wchar_t('"'));
	new_str.push_back(wchar_t('>'));
	insert(new_str, 0, &word[0], word.size());
	insert(new_str, 0, L"</a>", 4);
	dict["link"] = 0;
}

void free_stand_link(vector <wchar_t> &word, vector <wchar_t> &new_str, map <string, int> &dict)
{
	if (word.size() == 0)
	{
		new_str.push_back(wchar_t(':'));
	}
	else
	{
		auto i = protocols.begin();
		for (i; ; i++)
		{
			if (i == protocols.end())
			{
				new_str.push_back(wchar_t(':'));
				word.clear();
				break;
			}
			else
			{
				if (wctcmp(word, *i) == 0)
				{
					insert(new_str, word.size(), L"<a href=", 8);
					new_str.insert(new_str.end() - word.size(), wchar_t('"'));
					new_str.push_back(wchar_t(':'));
					dict["link"] = 1;
					word.push_back(':');
					break;
				}
			}
			
		}

	}
}

void header_end(vector <wchar_t> &str, int &seqlen, wchar_t &suspect, map <string, int> &dict)
{
	str.erase(str.end() - seqlen + 2, str.end());
	insert(str, 0, L"</h", 3);
	str.push_back(wchar_t('0' + dict["header"]));
	insert(str, 0, L">\n", 2);
	dict["header"] = 0;
}

void changing(vector <wchar_t> &str, vector<wchar_t>::iterator &it, wchar_t &suspect, int &seqlen, map <string, int> &dict, int dist)
{
	switch (suspect)
	{
		case wchar_t('\\') :
		{
			if (seqlen == 1)
			{
				str.push_back(wchar_t('\\'));
				seqlen = 0;
				suspect = *it;
			}
			else
			{
				insert(str, 0, L"<br/>\n", 6);
				seqlen -= 2;
				it -= 1;
				suspect = *it;
			}
			break;
		}
		case wchar_t('=') :
		{
			cout << "test" << seqlen << " " << dist << endl;
			if (seqlen > 6)
			{
				for (int j = 0; j < seqlen; j++) str.push_back('=');
				seqlen = 0;
				suspect = *it;
			}
			else
			{
				if ((dist - seqlen) == 0)
				{
					insert(str , 0, L"<h", 2);
					str.push_back(wchar_t('0'+seqlen));
					str.push_back(wchar_t('>'));
					dict["header"] = seqlen;
				}
				else
				{
					for (int j = 0; j < seqlen; j++) str.push_back('=');
					seqlen = 0;
					suspect = *it;
				}
			}
			break;
		}
	}
}

void no_limitation_mode(vector <wchar_t> &str, vector<wchar_t>::iterator &it, wchar_t &suspect, int &seqlen, map <string, int> &dict, int dist)
{
	if (iswalpha(*it) || iswalnum(*it))
	{
		if (seqlen > 0)
		{
			//cout << seqlen;
			auto hlp = it;
			changing(str, it, suspect, seqlen, dict, dist);
			if (hlp == it) str.push_back(*it);
		}
		else
		{
			str.push_back(*it);
		}
	}
	else
	{
		switch (*it)
		{
			//syntax symbols
			case wchar_t('=') :
			{
				cout << "GETTT" << seqlen << endl;
				if (*it == suspect)
				{
					seqlen++;
				}
				else
				{
					if (seqlen > 0)
					{
						changing(str, it, suspect, seqlen, dict, dist);
					}
					else
					{
						suspect = *it;
						seqlen = 1;
					}
				}
				break;
			}
			case wchar_t('\\') :
			{
				if (*it == suspect)
				{
					seqlen++;
				}
				else
				{
					if (seqlen > 0)
					{
						changing(str, it, suspect, seqlen, dict, dist);
					}
					else
					{
						suspect = *it;
						seqlen = 1;
					}
				}
				break;
			}
			//non syntax symbols
			case wchar_t(' ') :
			{
				if (seqlen > 0)
				{
					auto hlp = it;
					changing(str, it, suspect, seqlen, dict, dist);
					if (hlp == it) str.push_back(*it);
				}
				else
				{
					str.push_back(*it);
				}
				break;
			}
			case wchar_t('\n') :
			{
				if (seqlen > 0)
				{
					auto hlp = it;
					changing(str, it, suspect, seqlen, dict, dist);
					if (hlp == it) str.push_back(*it);
				}
				else
				{
					//str.push_back(*it);
				}
				break;
			}
			default:
			{
				if (seqlen > 0)
				{
					auto hlp = it;
					changing(str, it, suspect, seqlen, dict, dist);
					if (hlp == it) str.push_back(*it);
				}
				else
				{
					str.push_back(*it);
				}
				break;
			}
		}
	}
}

void header_parsing_mode(vector <wchar_t> &str, vector<wchar_t>::iterator &it, wchar_t &suspect, int &seqlen, map <string, int> &dict)
{
	switch (*it)
	{
		case wchar_t('\n') :
		{
			header_end(str, seqlen, suspect, dict);
		}
		case wchar_t('=') :
		{
			if (suspect == *it) seqlen++;
			else
			{
				suspect = *it;
				seqlen = 1;
			}
		}
		default:
		{
			str.push_back(*it);
		}
	}
}

int mode_def(map <string, int> &dict)
{
	//0 — standart parsing mode
	//1 — link without caption([[example.com]]), main part of link([[example.com|something]]), free-stand link(https:example.com) parsing mode
	//2 — caption part of link([[example.com|something]]) parsing mode
	//3 — image without caption({{image.png}}), main part of image({{image.png|image}}) parsing mode
	//4 — caption part of image({{image.png|image}}) parsing mode
	//5 — header parsing mode
	//6 — tilda(~) parsing mode
	//7 — inline nowiki parsing mode(monospace)
	//8 — preformatted nowiki mode

	if (dict["header"] > 0) return 5;

	return 0;
}

namespace Creole
{
	std::vector <wchar_t> gString::St_string(vector <wchar_t> utfbuf, map <string, int> &dict)
	{
		int seqlen = 0;
		auto suspect = wchar_t('a');
		
		vector <wchar_t> new_str;
		if (!dict["section"])
		{
			insert(new_str, 0, L"<p>\n", 4);
			dict["section"] = 1;
		}
		auto it = utfbuf.begin();

		while (it != utfbuf.end())
		{
			auto mode = mode_def(dict);
			if (mode == 0)
			{
				no_limitation_mode(new_str, it, suspect, seqlen, dict, distance(utfbuf.begin(), it));
			}
			else if (mode == 5)
			{
				header_parsing_mode(new_str, it, suspect, seqlen, dict);
			}

				it++;
		}
		

		return new_str;
	}
}