#include "string_view.h"
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using namespace std;
using namespace experimental;

// I'm not really happy because split allocates new strings that
// I actually don't need since *str* contains them all
vector<string> split(const string& str, const char* delims)
{
	vector<string> ret;

	string::size_type start = 0;
	auto pos = str.find_first_of(delims, start);
	while (pos != string::npos)
	{
		if (pos != start)
		{
			ret.push_back(str.substr(start, pos - start));
		}
		start = pos + 1;
		pos = str.find_first_of(delims, start);
	}
	if (start < str.length())
		ret.push_back(str.substr(start, str.length() - start));

	return ret;
}

// string_view is a lightweight wrapper that hides - under
// a const string-like interface - a const char* + length
// If you compare this code with the previous, it's exactly the same
// (apart from vector<string_view> and parameters)
vector<string_view> vsplit(string_view str, string_view delims)
{
	vector<string_view> ret;

	string::size_type start = 0;
	auto pos = str.find_first_of(delims, start);
	while (pos != string::npos)
	{
		if (pos != start)
		{
			ret.push_back(str.substr(start, pos - start));
		}
		start = pos + 1;
		pos = str.find_first_of(delims, start);
	}
	if (start < str.length())
		ret.push_back(str.substr(start, str.length() - start));

	return ret;
}

int main()
{
	for (auto token : vsplit("this/is/a-path/", "/"))
	{
		cout << token << "\n";
	}

	// suppose from legacy API
	vector<char> carray{ 'h', 'e', 'l', 'l', '\0' }; 
	
	// length automatically inferred (\0)
	for (auto c : string_view(carray.data()))
	{
		cout << c << " - ";
	}
	cout << "\n";

	string str = "     This is a long complicated string";
	string_view view = str;

	// actually, this trim is only "logical"
	auto trimmed = view.substr(view.find_first_not_of(' '));
	cout << trimmed << "\n";

	// comparisons for free
	vector<char> actual = {'M', 'A', 'R', 'E', 'N', 'A', '\0'};
	const char* expected = "MARENA";
	cout << (actual.data() == expected) << "\n";

	cout << (actual.data() == string_view(expected)) << "\n";

	// >>>>>>>>> BONUS: string_view + transparent comparators

	//					v--- this is a *transparent* comparator
	map<string, int, less<>> mm{
           {"a", 1},
           {"b", 2}
        };
   
        auto entry = string_view("  a");
	// logical trim
        entry = entry.substr(entry.find_first_not_of(' '));
        //	           v--- this will not create a temporary std::string!
        cout << mm.find(entry.data())->second;
	// Note: calling mm.find(entry) directly causes a linker error (only in Debug):
	// fatal error LNK1179: invalid or corrupt file: duplicate COMDAT '??$?MDU?$char_traits@D@std@@@experimental@@YA_NV?$basic_string_view@DU?$char_traits@D@std@@@0@0@Z'
	// I have not investigated in deep, but I think it's related to some strange template expansion.
	// Release mode does not exhibit this issue.
	// Using .data() is valid in this example, but it may be not in general since the string_view may refer
	// only to a portion of the underlying const char*.
	// string_view uses its size to perform the correct comparison, instead less<> with .data() will consider the whole const char*.
}
