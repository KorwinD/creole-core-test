#include <fstream>
#include <windows.h>
#include "Header.h"
#include "Dict.h"
#pragma warning(disable : 4996)

using namespace std;

string to_utf8_(const wchar_t* buffer, int len)
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

void main()
{
	char *filename = "tests//new.creole";	
	string str;
	ifstream file(filename);
	ofstream file1("tests//new.html");
	unsigned line_count = 1;

	while (getline(file, str))
	{
		auto end_it = utf8::find_invalid(str.begin(), str.end());
		if (end_it != str.end()) {
			cout << "Invalid UTF-8 encoding detected at line " << line_count << "\n";
			cout << "This part is fine: " << string(str.begin(), end_it) << "\n";
		}

		int length = utf8::distance(str.begin(), end_it);
		vector<wchar_t> utf16line;
		utf8::utf8to16(str.begin(), end_it, back_inserter(utf16line));
		utf16line.push_back(wchar_t('\n'));
		if (utf16line[0] == wchar_t(65279))
		{
			utf16line.erase(utf16line.begin());
			auto out = Creole::gString::St_string(vector<wchar_t>(utf16line.begin(), utf16line.end()), dict, list);
			out.insert(out.begin(), wchar_t(65279));
			wchar_t* buf = &out[0];
			file1 << to_utf8_(buf, out.size()) << endl;
		}
		else
		{

			auto out = Creole::gString::St_string(vector<wchar_t>(utf16line.begin(), utf16line.end()), dict, list);
			if (out.size())
			{
				wchar_t* buf = &out[0];
				file1 << to_utf8_(buf, out.size()) << endl;
			}

		}
	}
	auto out = Creole::gString::End_file(dict, list);
	if (out.size())
	{
		wchar_t* buf = &out[0];
		file1 << to_utf8_(buf, out.size()) << endl;
	}

	file1.close();
	file.close();
	

	cout << endl;
	system("pause");
}