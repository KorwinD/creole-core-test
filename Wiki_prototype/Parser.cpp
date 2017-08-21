#include "Header.h"
#include <stdexcept>
#include <ctype.h>

#pragma warning(disable : 4996)

#define min(a,b) (((a) < (b)) ? (a) : (b))

using namespace std;

vector <wchar_t *> protocols =
{
	{ L"https:" },
	{ L"http:" },
	{ L"ftp:" },
	{ L"mailto:" },
};

map <wchar_t *, wchar_t *> wikies = 
{
	{L"main:", L"tests/"},
	{L"main_pics", L"pics/"},
	{L"Wikipedia:", L"https://en.wikipedia.org/wiki/"},
};

vector <wchar_t> allowed_symbols = 
{
	{'!', '(', ')', ';', '@', '/', '&', '+', '$', '?', '-', '_', '.', '#', '*', '[', ']', wchar_t(27) /* ' */}
};

vector <wchar_t> punctuation =
{
	{ '!', ';', ':', '"', '?', '.', ',', wchar_t(27) /* ' */ }
};

void clearing_forward(vector <wchar_t> &str)
{
	auto i = str.begin();

	while ((i != str.end()) && (*i == wchar_t(' ')))
	{
		i++;
	}
	str.erase(str.begin(), i);
}

void clearing_backward(vector <wchar_t> &str)
{
	if (str.size() == 1) return;
	auto i = str.end() - 2;
	while ((i != str.begin()) && (*i == wchar_t(' ')))
	{
		i--;
	}
	str.erase(i+1, str.end() - 1);
}

void clearing(vector <wchar_t> &str, int type)
{
	if (type == 0) clearing_forward(str);
	else if (type == 1) clearing_backward(str);
	else if (type == 2)
	{
		clearing_backward(str);
		clearing_forward(str);
	}
	else
	{
		//TODO Erorrs.
	}
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

void insert(vector <wchar_t> &str, int shift, vector <wchar_t> word)
{
	for (int i = 0; i < word.size(); i++)
	{
		str.insert(str.end() - shift, word[i]);
	}
}

void list_item_end(vector <wchar_t> &str, map <string, int> &dict)
{
	if (dict["n_lvl"]) insert(str, 0, L"</li>", 5);
}


void link_error(vector <wchar_t> &str, vector <wchar_t> &word, map <string, int> &dict)//error in main part of link
{
	insert(str, 0, L"[[", 2);
	if (word.size()) insert(str, 0, &word[0], word.size());
	word.clear();
	dict["mlink"] = 0;
}

void pic_error(vector <wchar_t> &str, vector <wchar_t> &word, map <string, int> &dict)
{
	insert(str, 0, L"{{", 2);
	if (word.size()) insert(str, 0, &word[0], word.size());
	word.clear();
	dict["mpic"] = 0;
}

void link_error(vector <wchar_t> &str, vector <wchar_t> &word1, vector <wchar_t> &word2, map <string, int> &dict)//error in second part of link
{
	insert(str, 0, L"[[", 2);
	insert(str, 0, &word1[0], word1.size());
	word1.clear();
	str.push_back(wchar_t('|'));
	insert(str, 0, &word2[0], word2.size());
	word2.clear();
	dict["slink"] = 0;
}

void pic_error(vector <wchar_t> &str, vector <wchar_t> &word1, vector <wchar_t> &word2, map <string, int> &dict)
{
	insert(str, 0, L"{{", 2);
	insert(str, 0, &word1[0], word1.size());
	word1.clear();
	str.push_back(wchar_t('|'));
	insert(str, 0, &word2[0], word2.size());
	word2.clear();
	dict["spic"] = 0;
}


void link_end(vector <wchar_t> &new_str, vector <wchar_t> &word, map <string, int> &dict)
{
	insert(new_str, 0, &word[0], word.size());
	new_str.push_back(wchar_t('"'));
	new_str.push_back(wchar_t('>'));
	insert(new_str, 0, &word[0], word.size());
	insert(new_str, 0, L"</a>", 4);
	
	word.clear();
	dict["flink"] = 0;
}

void pic_end(vector <wchar_t> &new_str, vector <wchar_t> word, map <string, int> &dict)
{

	insert(new_str, 0, &word[0], word.size());
	new_str.push_back(wchar_t('"'));
	insert(new_str, 0, L"/>", 2);

	dict["mpic"] = 0;
}

void link_end(vector <wchar_t> &new_str, vector <wchar_t> word1, vector <wchar_t> word2, map <string, int> &dict)
{

	insert(new_str, 0, &word1[0], word1.size());
	new_str.push_back(wchar_t('"'));
	new_str.push_back(wchar_t('>'));
	insert(new_str, 0, &word2[0], word2.size());
	insert(new_str, 0, L"</a>", 4);


	//dict["flink"] = 0;
}

void pic_end(vector <wchar_t> &new_str, vector <wchar_t> word1, vector <wchar_t> word2, map <string, int> &dict)
{

	insert(new_str, 0, &word1[0], word1.size());
	new_str.push_back(wchar_t('"'));
	insert(new_str, 0, L" alt=", 5);
	new_str.push_back(wchar_t('"'));
	insert(new_str, 0, &word2[0], word2.size());
	new_str.push_back(wchar_t('"'));
	insert(new_str, 0, L"/>", 2);
}


void link_identification(vector <wchar_t> &word, vector <wchar_t> &str, map <string, int> &dict)
{

	//cout << word.size() << endl;
	if (word.size() != 0)
	{
		auto i = protocols.begin();
		auto j = wikies.begin();
		for (i; ; i++)
		{

			if (i == protocols.end())
			{
				break;
			}

			if (wctcmp(word, *i) == 0)
			{
				str.erase(str.end() - word.size(), str.end());
				insert(str, 0, L"<a href=", 8);
				str.insert(str.end(), wchar_t('"'));
				if (!dict["mlink"]) dict["flink"] = 1;
				return;
			}
		}

		for (j; ; j++)
		{
			if (j == wikies.end())
			{
				return;
			}
			if (wctcmp(word, j->first) == 0)
			{
				str.erase(str.end() - word.size(), str.end());
				word.clear();
				insert(word, 0, j->second, wcslen(j->second));
				insert(str, 0, L"<a href=", 8);
				str.insert(str.end(), wchar_t('"'));
				if (!dict["mlink"]) dict["flink"] = 1;
				return;
			}
		}
	}
}

bool link_identification(vector <wchar_t> &word)
{


	if (word.size() != 0)
	{
		auto i = protocols.begin();
		auto j = wikies.begin();
		for (i; ; i++)
		{

			if (i == protocols.end())
			{
				break;
			}

			if (wctcmp(word, *i) == 0)
			{
				return true;
			}
		}
		
		for (j; ; j++)
		{
			if (j == wikies.end())
			{
				return false;
			}
			if (wctcmp(word, j->first) == 0)
			{
				word.clear();
				insert(word, 0, j->second, wcslen(j->second));
				return true;
			}
		}
	}
	return false;
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
	else if (mode == 2)
	{
		word.push_back(*it);
	}
	else if (mode == 3)
	{
		word.push_back(*it);
	}
	else if (mode == 4)
	{
		word.push_back(*it);
	}
	else if (mode == 5)
	{
		word.push_back(*it);
	}
	else if (mode == 6)
	{
		if ((word.size() == 0) && (*it == wchar_t(' '))) return;

		if (*it == ' ') word.push_back(wchar_t('-'));
		else if (*it == '\n') return;
		else word.push_back(*it);
	}
}

void list_end(vector <wchar_t> &str, map <string, int> &dict, vector <int> &list)
{
	int h = 0;
	for (h; h < (dict["n_lvl"]); h++)
	{
		if (list[list.size() - 1 - h] == 0) insert(str, 0, L"</ul>\n", 6);
		if (list[list.size() - 1 - h] == 1) insert(str, 0, L"</ol>\n", 6);
	}
	list.clear();
	dict["n_lvl"] = 0;
}

void block_end(vector <wchar_t> &str, map <string, int> &dict)
{

	if (dict["cursive"] > dict["bold"])
	{
		if (dict["bold"])
		{
			insert(str, 0, L"</strong>", 9);
			dict["bold"] = 0;
		}
		insert(str, 0, L"</em>", 5);
		dict["cursive"] = 0;
	}
	else if (dict["cursive"] < dict["bold"])
	{
		if (dict["cursive"])
		{
			insert(str, 0, L"</em>", 5);
			dict["cursive"] = 0;
		}
		insert(str, 0, L"</strong>", 9);
		dict["bold"] = 0;
	}
}

void section_end(vector <wchar_t> &str, map <string, int> &dict, vector <int> &list)
{

	block_end(str, dict);

	list_end(str, dict, list);

	if (dict["table"]) insert(str, 0, L"</table>\n", 9);
	dict["table"] = 0;

	if (dict["section"]) insert(str, 0, L"</p>", 4);
	dict["section"] = 0;
}

void header_end(vector <wchar_t> &str, int &seqlen, wchar_t &suspect, map <string, int> &dict, vector <wchar_t> &word)
{
	//cout << seqlen << endl;
	str.erase(str.end() - seqlen, str.end());
	insert(str, 0, L"<a name=", 8);
	str.push_back(wchar_t('"'));
	insert(str, 0, word);
	str.push_back(wchar_t('"'));
	str.push_back(wchar_t('>'));
	insert(str, 0, L"</a>", 4);
	insert(str, 0, L"</span>", 7);
	insert(str, 0, L"</h", 3);
	str.push_back(wchar_t('0' + dict["header"]));
	insert(str, 0, L">", 2);
	dict["header"] = 0;
}

void catch_(vector<wchar_t>::iterator &it, wchar_t &suspect, int &seqlen)
{
	seqlen = 0;
	suspect = *it;
	//TODO Replace other code with this function.
}

void changing(vector <wchar_t> &str, vector<wchar_t>::iterator &it, wchar_t &suspect, int &seqlen, map <string, int> &dict, int dist, vector <wchar_t> &word, vector <int> &list)
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
				seqlen -= 1;
				suspect = *it;
			}
			break;
		}
		case wchar_t('-') :
		{
			if ((seqlen == 4) && (*it == wchar_t('\n')))
			{
				insert(str, 0, L"<hr/>", 5);
			}
			else
			{
				for (int j = 0; j < seqlen; j++) str.push_back('-');
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
				seqlen -= 1;
				suspect = *it;
			}
			break;
		}
		case wchar_t('#') :
		{
			if (seqlen == dist)
			{
				if (seqlen > dict["n_lvl"])
				{
					for (int h = 0; h < (seqlen - dict["n_lvl"]); h++)
					{
						insert(str, 0, L"<ol>\n", 5);
						list.push_back(1);
					}
				}
				else if (seqlen < dict["n_lvl"])
				{
					int h = 0;
					for (h; h < (dict["n_lvl"] - seqlen); h++)
					{
						if (list[list.size() - 1 - h] == 0) insert(str, 0, L"</ul>\n", 6);
						if (list[list.size() - 1 - h] == 1) insert(str, 0, L"</ol>\n", 6);
					}
					list.erase(list.end() - h, list.end());
				}
				insert(str, 0, L"<li>", 4);

				dict["n_lvl"] = seqlen;
				suspect = *it;
				seqlen = 1;
			}
			break;
		}
		case wchar_t('*') :
		{
			if ((seqlen == dist) && ((dict["n_lvl"] && (dist == 2)) || (dist != 2)))
			{
				if (seqlen > dict["n_lvl"])
				{
					for (int h = 0; h < (seqlen - dict["n_lvl"]); h++)
					{
						insert(str, 0, L"<ul>\n", 5);
						list.push_back(0);
					}
				}
				else if (seqlen < dict["n_lvl"])
				{
					int h = 0;
					for (h; h < (dict["n_lvl"] - seqlen); h++)
					{
						if (list[list.size() - 1 - h] == 0) insert(str, 0, L"</ul>\n", 6);
						if (list[list.size() - 1 - h] == 1) insert(str, 0, L"</ol>\n", 6);
					}
					list.erase(list.end() - h, list.end());
				}
				insert(str, 0, L"<li>", 4);

				dict["n_lvl"] = seqlen;
				suspect = *it;
				seqlen = 1;
				return;
			}

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
				seqlen -= 1;
				suspect = *it;
			}
			break;
		}
		case wchar_t('[') :
		{
			if (seqlen == 2)
			{
				word.clear();
				seqlen = 0;
				dict["mlink"] = 1;
				it--;
			}
			else
			{
				for (int j = 0; j < seqlen; j++) str.push_back('[');
				seqlen = 0;
				suspect = *it;
			}
			break;
		}
		case wchar_t('{') :
		{
			if (seqlen == 2)
			{
				word.clear();
				seqlen = 0;
				dict["mpic"] = 1;
				it--;
			}
			else
			{
				for (int j = 0; j < seqlen; j++) str.push_back('{');
				seqlen = 0;
				suspect = *it;
			}
			break;
		}
		case wchar_t('~') :
		{
			if (seqlen == 1)
			{
				word.clear();
				seqlen = 0;
				dict["tilde"] = 1;

			}
			else
			{
				for (int j = 0; j < seqlen; j++) str.push_back('~');
				seqlen = 0;
				suspect = *it;
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
					insert(str, 0, L"<h", 2);
					str.push_back(wchar_t('0' + seqlen));
					str.push_back(wchar_t('>'));
					insert(str, 0, L"<span>", 6);
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
		case wchar_t('|') :
		{
			if (!dict["table"])
			{
				if (dist == seqlen)
				{
					if ((*it == wchar_t('\n')) && (seqlen == 1))
					{
						str.push_back(wchar_t('|'));
						catch_(it, suspect, seqlen);
						return;
					}

					insert(str, 0, L"<table>\n", 8);
					insert(str, 0, L"<tr>\n", 5);

					for (int h = 0; h < seqlen - 1; h++)
					{
						insert(str, 0, L"<td>", 4);
						insert(str, 0, L"</td>", 5);
					}

					if (*it == wchar_t('='))
					{
						insert(str, 0, L"<th>", 4);
						dict["table"] = 3;
						catch_(it, suspect, seqlen);
						return;
					}
					else if (*it == wchar_t('\n'))
					{
						insert(str, 0, L"<td>", 4);
						insert(str, 0, L"</td>\n", 6);
						insert(str, 0, L"</tr>", 5);
						dict["table"] = 1;
						catch_(it, suspect, seqlen);
						return;
					}
					else
					{
						insert(str, 0, L"<td>", 4);
						dict["table"] = 2;
						catch_(it, suspect, seqlen);
						return;
					}
				}
				else
				{
					for (int j = 0; j < seqlen; j++) str.push_back('|');
					seqlen = 0;
					suspect = *it;
				}
			}
			else
			{
				if (dist == seqlen)
				{
					insert(str, 0, L"<tr>\n", 5);
					for (int h = 0; h < seqlen - 1; h++)
					{
						insert(str, 0, L"<td>", 4);
						insert(str, 0, L"</td>", 5);
					}

					if (*it == wchar_t('='))
					{
						insert(str, 0, L"<th>", 4);
						dict["table"] = 3;
						catch_(it, suspect, seqlen);
						return;
					}
					else if (*it == wchar_t('\n'))
					{
						insert(str, 0, L"<td>", 4);
						insert(str, 0, L"</td>\n", 6);
						insert(str, 0, L"</tr>", 5);
						catch_(it, suspect, seqlen);
						return;
					}
					else
					{
						insert(str, 0, L"<td>", 4);
						dict["table"] = 2;
						catch_(it, suspect, seqlen);
						return;
					}
				}
				else
				{
					if (dict["table"] == 1)
					{
						for (int j = 0; j < seqlen; j++) str.push_back('|');
						seqlen = 0;
						suspect = *it;
						dict["table"] = 0;
					}
					else if (dict["table"] == 2)
					{
						block_end(str, dict);

						insert(str, 0, L"</td>\n", 6);

						for (int h = 0; h < seqlen - 1; h++)
						{
							insert(str, 0, L"<td>", 4);
							insert(str, 0, L"</td>\n", 6);
						}

						if (*it == wchar_t('='))
						{
							insert(str, 0, L"<th>", 4);
							dict["table"] = 3;
							catch_(it, suspect, seqlen);
							return;
						}
						else if (*it == wchar_t('\n'))
						{
							insert(str, 0, L"</tr>", 5);
							dict["table"] = 1;
							catch_(it, suspect, seqlen);
							return;
						}
						else
						{
							insert(str, 0, L"<td>", 4);
							dict["table"] = 2;
							catch_(it, suspect, seqlen);
							return;
						}
					}
					else if(dict["table"] == 3)
					{
						block_end(str, dict);
						
						insert(str, 0, L"</th>\n", 6);

						for (int h = 0; h < seqlen - 1; h++)
						{
							insert(str, 0, L"<td>", 4);
							insert(str, 0, L"</td>\n", 6);
						}

						if (*it == wchar_t('='))
						{
							insert(str, 0, L"<th>", 4);
							catch_(it, suspect, seqlen);
							return;
						}
						else if (*it == wchar_t('\n'))
						{
							insert(str, 0, L"</tr>", 5);
							dict["table"] = 1;
							catch_(it, suspect, seqlen);
							return;
						}
						else
						{
							insert(str, 0, L"<td>", 4);
							dict["table"] = 2;
							catch_(it, suspect, seqlen);
							return;
						}
					}
				}
			}
			break;
		}
	}
}

void no_limitation_mode(vector <wchar_t> &str, vector<wchar_t>::iterator &it, wchar_t &suspect, int &seqlen, map <string, int> &dict, int dist, vector <wchar_t> &word, vector <int> &list)
{
	if ((dist == 0) && (dict["n_lvl"]) && (*it != wchar_t('*')) && (*it != wchar_t('#')))
	{
		list_end(str, dict, list);
	}

	if ((dist == 0) && (*it != wchar_t('|')) && dict["table"])
	{
		insert(str, 0, L"</table>\n", 9);
		dict["table"] = 0;
	}

	if (iswalpha(*it) || iswalnum(*it))
	{
		if (seqlen > 0)
		{
			auto hlp = it;
			changing(str, it, suspect, seqlen, dict, dist, word, list);
			if (hlp == it) str.push_back(*it);
		}
		else
		{
			str.push_back(*it);
		}
		seqlen = 0;
	}
	else
	{
		switch (*it)
		{
			//syntax symbols
			case wchar_t('=') :
			{
				if (*it == suspect)
				{
					seqlen++;
				}
				else
				{
					if (seqlen > 0)
					{
						changing(str, it, suspect, seqlen, dict, dist, word, list);
					}
					else
					{
						suspect = *it;
						seqlen = 1;
					}
				}
				break;
			}
			case wchar_t('~') :
			{
				if (*it == suspect)
				{
					seqlen++;
				}
				else
				{
					if (seqlen > 0)
					{
						changing(str, it, suspect, seqlen, dict, dist, word, list);
					}
					else
					{
						suspect = *it;
						seqlen = 1;
					}
				}
				break;
			}
			case wchar_t('-') :
			{
				if (*it == suspect)
				{
					seqlen++;
				}
				else
				{
					if (seqlen > 0)
					{
						changing(str, it, suspect, seqlen, dict, dist, word, list);
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
						changing(str, it, suspect, seqlen, dict, dist, word, list);
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
						changing(str, it, suspect, seqlen, dict, dist, word, list);
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
						changing(str, it, suspect, seqlen, dict, dist, word, list);
					}
					else
					{
						suspect = *it;
						seqlen = 1;
					}
				}
				break;
			}
			case wchar_t('#') :
			{
				if (*it == suspect)
				{
					seqlen++;
				}
				else
				{
					if (seqlen > 0)
					{
						changing(str, it, suspect, seqlen, dict, dist, word, list);
					}
					else
					{
						suspect = *it;
						seqlen = 1;
					}
				}
				break;
			}
			case wchar_t('[') :
			{
				if (*it == suspect)
				{
					seqlen++;
				}
				else
				{
					if (seqlen > 0)
					{
						changing(str, it, suspect, seqlen, dict, dist, word, list);
					}
					else
					{
						suspect = *it;
						seqlen = 1;
					}
				}
				break;
			}
			case wchar_t('{') :
			{
				if (*it == suspect)
				{
					seqlen++;
				}
				else
				{
					if (seqlen > 0)
					{
						changing(str, it, suspect, seqlen, dict, dist, word, list);
					}
					else
					{
						suspect = *it;
						seqlen = 1;
					}
				}
				break;
			}
			case wchar_t('|') :
			{
				if (*it == suspect)
				{
					seqlen++;
				}
				else
				{
					if (seqlen > 0)
					{
						changing(str, it, suspect, seqlen, dict, dist, word, list);
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
					changing(str, it, suspect, seqlen, dict, dist, word, list);
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
			case wchar_t('\n') :
			{
				if (seqlen > 0)
				{
					auto hlp = it;
					changing(str, it, suspect, seqlen, dict, dist, word, list);
					if (hlp == it) list_item_end(str, dict);
				}
				else
				{
					list_item_end(str, dict);
				}
				break;
			}
			default:
			{
				if (seqlen > 0)
				{
					auto hlp = it;
					changing(str, it, suspect, seqlen, dict, dist, word, list);
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
				auto u = punctuation.begin();
				for (u; ; u++)
				{
					if (u == punctuation.end()) break;

					if (*u == word[word.size() - 1])
					{
						word.erase(word.end() - 1);
						it--;
					}
				}

				link_end(str, word, dict);
				str.push_back(*it);
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

void mainlink_parsing_mode(vector <wchar_t> &str, vector<wchar_t>::iterator &it, map <string, int> &dict, vector <wchar_t> &word, int &seqlen, bool &foundation)
{
	if (*it == wchar_t('\n')) link_error(str, word, dict);
	else if (*it == wchar_t('|'))
	{
		if (word.size())
		{
			word.erase(word.end() - 1, word.end());
			dict["slink"] = 1;
			dict["mlink"] = 0;
			seqlen = 0;
		}
		else
		{
			insert(str, 0, L"[[", 2);
		}
	}
	else if (*it == wchar_t(']')) seqlen++;
	else if (*it == wchar_t(':'))
	{
		foundation = link_identification(word);
	}

	if (seqlen == 2)
	{
		word.erase(word.end() - 2, word.end());
		if (word.size())
		{
			insert(str, 0, L"<a href=", 8);
			str.push_back(wchar_t('"'));
			auto u = wikies.begin(); // I don't know why wikies[L"main"] doesn't work.
			if (!foundation) insert(str, 0, u->second, wcslen(u->second));
			link_end(str, word, dict);
		}
		else
		{
			insert(str, 0, L"[[]]", 4);
		}

		seqlen = 0;
		dict["mlink"] = 0;
	}
}

void captionlink_parsing_mode(vector <wchar_t> &str, vector<wchar_t>::iterator &it, map <string, int> &dict, vector <wchar_t> &word1, vector <wchar_t> &word2, int &seqlen, bool &foundation)
{
	if (*it == wchar_t('\n')) link_error(str, word1, word2, dict);
	else if (*it == wchar_t(']')) seqlen++;

	if (seqlen == 2)
	{
		word2.erase(word2.end() - 2, word2.end());
		if (word2.size())
		{
			insert(str, 0, L"<a href=", 8);
			str.push_back(wchar_t('"'));
			auto u = wikies.begin(); // I don't know why wikies[L"main"] doesn't work.
			if (!foundation) insert(str, 0, u->second, wcslen(u->second));
			link_end(str, word1, word2, dict);
		}
		else
		{
			link_end(str, word1, dict);
		}

		seqlen = 0;
		dict["slink"] = 0;
	}
}

void mainpic_parsing_mode(vector <wchar_t> &str, vector<wchar_t>::iterator &it, map <string, int> &dict, vector <wchar_t> &word, int &seqlen, bool &foundation)
{
	if (*it == wchar_t('\n')) pic_error(str, word, dict);
	else if (*it == wchar_t('|'))
	{
		
		if (word.size())
		{
			word.erase(word.end() - 1, word.end());
			dict["spic"] = 1;
			dict["mpic"] = 0;
			seqlen = 0;
		}
		else
		{
			insert(str, 0, L"{{", 2);
		}
		
	}
	else if (*it == wchar_t('}')) seqlen++;
	else if (*it == wchar_t(':'))
	{
		foundation = link_identification(word);
	}

	if (seqlen == 2)
	{
		word.erase(word.end() - 2, word.end());
		if (word.size())
		{
			insert(str, 0, L"<img src=", 9);
			str.push_back(wchar_t('"'));
			auto u = wikies.begin(); 
			u++;
			if (!foundation) insert(str, 0, u->second, wcslen(u->second));
			pic_end(str, word, dict);
		}
		else
		{
			insert(str, 0, L"{{}}", 4);
		}

		seqlen = 0;
		dict["mpic"] = 0;
	}
}

void captionpic_parsing_mode(vector <wchar_t> &str, vector<wchar_t>::iterator &it, map <string, int> &dict, vector <wchar_t> &word1, vector <wchar_t> &word2, int &seqlen, bool &foundation)
{
	if (*it == wchar_t('\n')) pic_error(str, word1, word2, dict);
	else if (*it == wchar_t('}')) seqlen++;

	if (seqlen == 2)
	{
		word2.erase(word2.end() - 2, word2.end());
		if (word2.size())
		{
			insert(str, 0, L"<img src=", 9);
			str.push_back(wchar_t('"'));
			auto u = wikies.begin(); // I don't know why wikies[L"main"] doesn't work.
			u++;
			if (!foundation) insert(str, 0, u->second, wcslen(u->second));
			pic_end(str, word1, word2, dict);
		}
		else
		{
			pic_end(str, word1, dict);
		}

		seqlen = 0;
		dict["spic"] = 0;
	}
}

void header_parsing_mode(vector <wchar_t> &str, vector<wchar_t>::iterator &it, wchar_t &suspect, int &seqlen, map <string, int> &dict, vector <wchar_t> &word)
{
	switch (*it)
	{
		case wchar_t('\n') :
		{
			header_end(str, seqlen, suspect, dict, word);
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

void tilde_parsing_mode(vector <wchar_t> &str, vector<wchar_t>::iterator &it, map <string, int> &dict)
{
	if ((*it == wchar_t(' ')) || (*it == wchar_t('\n')))
	{
		str.push_back(*it);
		dict["tilde"] = 0;
	}
	else
	{
		str.push_back(*it);
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
	//7 — tilde(~) parsing mode
	//8 — inline nowiki parsing mode(monospace)
	//9 — preformatted nowiki mode

	if (dict["flink"]) return 1;

	if (dict["mlink"]) return 2;

	if (dict["slink"]) return 3;

	if (dict["mpic"]) return 4;

	if (dict["spic"]) return 5;

	if (dict["header"]) return 6;

	if (dict["tilde"]) return 7;

	if (dict["table"]) return 8;

	return 0;
}

namespace Creole
{
	std::vector <wchar_t> gString::St_string(vector <wchar_t> utfbuf, map <string, int> &dict, std::vector <int> &list)
	{
		int seqlen = 0;
		auto suspect = wchar_t('a');
		vector <wchar_t> new_str, word1, word2;
		bool foundation = false;
		if (mode_def(dict) == 0) clearing(utfbuf, 2);
		auto it = utfbuf.begin();
		while (it != utfbuf.end())
		{
			if ((it == utfbuf.begin()) && (*it == wchar_t('\n')))
			{
				if (dict["section"]) section_end(new_str, dict, list);
				
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
					no_limitation_mode(new_str, it, suspect, seqlen, dict, distance(utfbuf.begin(), it), word1, list);
				}
				else if (mode == 1)
				{
					freelink_parsing_mode(new_str, it, dict, word1);
				}
				else if (mode == 2)
				{
					filling(word1, mode, it, new_str);
					mainlink_parsing_mode(new_str, it, dict, word1, seqlen, foundation);
				}
				else if (mode == 3)
				{
					filling(word2, mode, it, new_str);
					captionlink_parsing_mode(new_str, it, dict, word1, word2, seqlen, foundation);
				}
				else if (mode == 4)
				{
					filling(word1, mode, it, new_str);
					mainpic_parsing_mode(new_str, it, dict, word1, seqlen, foundation);
				}
				else if (mode == 5)
				{
					filling(word2, mode, it, new_str);
					captionpic_parsing_mode(new_str, it, dict, word1, word2, seqlen, foundation);
				}
				else if (mode == 6)
				{
					filling(word1, mode, it, new_str);
					header_parsing_mode(new_str, it, suspect, seqlen, dict, word1);
				}
				else if (mode == 7)
				{
					tilde_parsing_mode(new_str, it, dict);
				}
				else if (mode == 8)
				{
					filling(word1, mode, it, new_str);
					no_limitation_mode(new_str, it, suspect, seqlen, dict, distance(utfbuf.begin(), it), word1, list);
				}
			}
			it++;
		}
		word1.clear();
		word2.clear();
		return new_str;
	}

	std::vector <wchar_t> gString::End_file(std::map <std::string, int> &dict, vector <int> &list)
	{
		vector <wchar_t> v;
		section_end(v, dict, list);
		return v;
	}
}