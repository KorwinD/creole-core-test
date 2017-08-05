#include "Header.h"
#include <stdexcept>
#include <ctype.h>
#include <windows.h>
#pragma warning(disable : 4996)

using namespace std;

vector <wchar_t *> protocols =
{
	{ L"https:" },
	{ L"http:" },
	{ L"ftp:" },
	{ L"mailto:" },
};

vector <wchar_t> allowed_symbols = 
{
	{'!', '(', ')', ';', '@', '/', '&', '+', '$', '?', '-', '_', '.', '#', '*', '[', ']', wchar_t(27) /* ' */}
};

int wctcmp(vector <wchar_t> a, wchar_t *b)
{
	for (int i = 0; i < min(a.size(), wcslen(b)); i++) //TODO Write own min func
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
	insert(new_str, 0, &word[0], word.size());
	new_str.push_back(wchar_t('"'));
	new_str.push_back(wchar_t('>'));
	insert(new_str, 0, &word[0], word.size());
	insert(new_str, 0, L"</a>", 4);
	dict["flink"] = 0;
}

void link_identification(vector <wchar_t> &word, vector <wchar_t> &str, map <string, int> &dict)
{
	if (word.size() != 0)
	{
		auto i = protocols.begin();

		for (i; ; i++)
		{

			if (i == protocols.end())
			{
				word.clear();
				break;
			}
			if (wctcmp(word, *i) == 0)
			{
				str.erase(str.end() - word.size(), str.end());
				insert(str, 0, L"<a href=", 8);
				str.insert(str.end(), wchar_t('"'));
				dict["flink"] = 1;
				break;
			}
		}
	}
}

void filling(vector <wchar_t> &word, int mode, vector<wchar_t>::iterator &it, vector <wchar_t> str)
{
	if (mode == 0)
	{
		if (iswalpha(*it) || iswalnum(*it)) word.push_back(*it);
		else if (*it == wchar_t(':')) word.push_back(*it);
		else
		{
			word.clear();
		}
	}
}

void section_end(vector <wchar_t> &str, map <string, int> &dict)
{
	if (dict["cursive"] > dict["bold"])
	{
		insert(str, 0, L"</em>", 5);
		dict["cursive"] = 0;

		if (dict["bold"])
		{
			insert(str, 0, L"</strong>", 9);
			dict["bold"] = 0;
		}
	}
	else if (dict["cursive"] < dict["bold"])
	{
		insert(str, 0, L"</strong>", 9);
		dict["bold"] = 0;

		if (dict["cursive"])
		{
			insert(str, 0, L"</em>", 5);
			dict["cursive"] = 0;
		}
	}
	insert(str, 0, L"</p>", 4);
	dict["section"] = 0;
}

void header_end(vector <wchar_t> &str, int &seqlen, wchar_t &suspect, map <string, int> &dict)
{
	//cout << seqlen << endl;
	str.erase(str.end() - seqlen, str.end());
	insert(str, 0, L"</h", 3);
	str.push_back(wchar_t('0' + dict["header"]));
	insert(str, 0, L">", 2);
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
		case wchar_t('/') :
		{
			if (seqlen == 1)
			{
				str.push_back(wchar_t('/'));
				seqlen = 0;
				suspect = *it;
			}
			else
			{
				if (dict["cursive"])
				{
					dict["cursive"] = 0;
					insert(str, 0, L"</em>", 5);
				}
				else
				{
					if (dict["bold"])
					{
						dict["cursive"] = 1;
					}
					else
					{
						dict["cursive"] = 2;
					}
					insert(str, 0, L"<em>", 4);
				}
				seqlen -= 2;
				it -= 1;
				suspect = *it;
			}
			break;
		}
		case wchar_t('*') :
		{
			if (((seqlen != 2) && (dist == seqlen)) || ((seqlen == 2) && (dist == seqlen) && (dict["n_lvl"])))
			{
				//TODO Lists
			}
			else//bold
			{
				if (seqlen == 1)
				{
					str.push_back(wchar_t('*'));
					seqlen = 0;
					suspect = *it;
				}
				else
				{
					if (dict["bold"])
					{
						dict["bold"] = 0;
						insert(str, 0, L"</strong>", 9);
					}
					else
					{
						if (dict["cursive"])
						{
							dict["bold"] = 1;
						}
						else
						{
							dict["bold"] = 2;
						}
						insert(str, 0, L"<strong>", 8);
					}
					seqlen -= 2;
					it -= 1;
					suspect = *it;
				}
			}
			break;
		}
		case wchar_t('=') :
		{
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

void no_limitation_mode(vector <wchar_t> &str, vector<wchar_t>::iterator &it, wchar_t &suspect, int &seqlen, map <string, int> &dict, int dist, vector <wchar_t> &word)
{
	if (iswalpha(*it) || iswalnum(*it))
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
			case wchar_t('/') :
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
			case wchar_t('*') :
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
			case wchar_t(':') :
			{
				if (seqlen > 0)
				{
					auto hlp = it;
					changing(str, it, suspect, seqlen, dict, dist);
					if (hlp == it)
					{
						str.push_back(*it);
						link_identification(word, str, dict);
					}
				}
				else
				{
					str.push_back(*it);
					link_identification(word, str, dict);
				}
				break;
			}
			case wchar_t(' ') :
			{
				if (seqlen > 0)
				{
					auto hlp = it;
					changing(str, it, suspect, seqlen, dict, dist);
					if (hlp == it)
					{
						str.push_back(*it);
					}
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

void freelink_parsing_mode(vector <wchar_t> &str, vector<wchar_t>::iterator &it, map <string, int> &dict, vector <wchar_t> &word)
{
	if (iswalpha(*it) || iswalnum(*it))
	{
		word.push_back(*it);
	}
	else
	{
		for (auto i = allowed_symbols.begin(); ; i++)
		{
			if (i == allowed_symbols.end())
			{
				link_end(str, word, dict);
				break;
			}

			if (*it == *i)
			{
				word.push_back(*it);
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
			break;
		}
		case wchar_t('=') :
		{
			if (suspect == *it)
			{
				seqlen++;
				str.push_back(*it);
			}
			else
			{
				str.push_back(*it);
				suspect = *it;
				seqlen = 1;
			}
			break;
		}
		default:
		{
			seqlen = 0;
			suspect = wchar_t('a');
			str.push_back(*it);
			break;
		}
	}
}

int mode_def(map <string, int> &dict)
{
	//0 — standart parsing mode
	//1 — free-stand link(https:example.com) parsing mode
	//2 — link without caption([[example.com]]), main part of link([[example.com|something]]) parsing mode
	//3 — caption part of link([[example.com|something]]) parsing mode
	//4 — image without caption({{image.png}}), main part of image({{image.png|image}}) parsing mode
	//5 — caption part of image({{image.png|image}}) parsing mode
	//6 — header parsing mode
	//7 — tilda(~) parsing mode
	//8 — inline nowiki parsing mode(monospace)
	//9 — preformatted nowiki mode

	if (dict["header"]) return 6;

	if (dict["flink"]) return 1;

	return 0;
}

namespace Creole
{
	std::vector <wchar_t> gString::St_string(vector <wchar_t> utfbuf, map <string, int> &dict)
	{
		int seqlen = 0;
		auto suspect = wchar_t('a');
		vector <wchar_t> new_str, word1, word2;

		auto it = utfbuf.begin();

		while (it != utfbuf.end())
		{
			if ((it == utfbuf.begin()) && (*it == wchar_t('\n')))
			{
				if (dict["section"]) section_end(new_str, dict);
				
				int mode = 0;
			}
			else
			{
				if (!dict["section"])
				{
					insert(new_str, 0, L"<p>\n", 4);
					dict["section"] = 1;
				}

				auto mode = mode_def(dict);

				if (mode == 0)
				{
					filling(word1, mode, it, new_str);
					no_limitation_mode(new_str, it, suspect, seqlen, dict, distance(utfbuf.begin(), it), word1);
				}
				else if (mode == 1)
				{
					freelink_parsing_mode(new_str, it, dict, word1);
				}
				else if (mode == 6)
				{
					header_parsing_mode(new_str, it, suspect, seqlen, dict);
				}
			}
			it++;
		}
		word1.clear();
		word2.clear();
		return new_str;
	}
}