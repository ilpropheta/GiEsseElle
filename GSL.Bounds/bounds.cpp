#include <iostream>
#include <vector>
#include <map>
#include <iostream>
#include "gsl.h"
#include "span.h"

using namespace std;
using namespace gsl;

int sum_elements(int* arr, int size)
{
	int sum = 0; 
	for (int i = 0; i < size; i++)
	{
		sum += arr[i]; //  warning C26481: Don't use pointer arithmetic. Use span instead. (bounds.1)
	}
	return sum;
}

// Refactoring 1: using span
int sum_elements(span<int> range)
{
	int sum = 0;
	for (int i = 0; i < range.size(); i++)
	{
		sum += range[i]; // run-time checked
	}
	return sum;
}

// ES.1: Prefer the standard library to other libraries and to "handcrafted code"
// https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#es1-prefer-the-standard-library-to-other-libraries-and-to-handcrafted-code
// Refactoring 2: using STL
int modern_sum_elements(span<int> range)
{
	return accumulate(begin(range), end(range), 0);
}

// sum_elements examples
void call_sum_elements()
{
	int arr[5]{ 1,2,3,4,5 };
	// warning C26485: Expression 'arr': No array to pointer decay. (bounds.3)
	cout << sum_elements(arr, 5) << "\n"; // passing this size manually is risky

	cout << modern_sum_elements(arr) << "\n"; // ok, span ctor automatically infers the size

	vector<int> v{ 1,2,3,4 };
	modern_sum_elements(as_span(v));

	auto portion = as_span(v).subspan(0, 3); // [1,2,3]
	cout << modern_sum_elements(portion) << "\n";

	int* evil = nullptr;
	sum_elements(evil, 1); // inside sum_elements: undefined behavior

	modern_sum_elements({ evil, 2 }); // ok, span ctor Expects a valid ptr when size > 0

	/*	Note: magic does not exist:
			  passing misaligned (ptr, size) cannot be portably detected
			  for this reason, construct span *as soon as possible*
	*/ 
}

// Suppose we measured, and we cannot afford bounds-checking here:
void many_arrays_operation(const double* in1, const double* in2, double* out, int size)
{
	for (int i = 0; i < size; i++)
	{
		[[gsl::suppress(bounds.1)]] // critical operation
		{
			out[i] = in1[i] + in2[i]; 
		}

	}

	/*	A little improvement can consist in accepting parameters as span and
		work on raw pointers inside the for loop...
	*/
}

// Refactoring example

// Suppose we don't have control on this external C function
void Get_ApiC(double* out, const unsigned int* ids, int size)
{
	// this function will copy into *out* the values
	// corresponding to *ids*
}

struct Getter
{
	double Get(unsigned id)
	{
		double d = 0.0;
		Get_ApiC(&d, &id, 1);
		return d;
	}

	void Get(double* out, const unsigned int* ids, int size)
	{
		Get_ApiC(out, ids, size);
	}

	void Get(vector<double>& out, const unsigned int* ids)
	{
		Get_ApiC(out.data(), ids, out.size());
	}

	void Get(vector<double>& out, const vector<unsigned int>& ids)
	{
		Get_ApiC(out.data(), ids.data(), out.size());
	}

	// many overloads, basically duplicated code
};

struct Getter_GSL
{
	double Get(unsigned id)
	{
		double d = 0.0;
		Get_ApiC(&d, &id, 1);
		return d;
	}
	
	// only one function, many possible usages
	void Get(span<double> out, span<const unsigned> ids)
	{
		Expects(ids.size() == out.size());
		Get_ApiC(out.data(), ids.data(), out.size());
	}
	// what it's actually lost: out and ids MUST have the same size
	// I'd like expressing this on the interface (waiting for contracts)
	// Actually, with the C-style approach, clients know this requirement:

	// what people think looking at this code:
	// mmm...only one size...two "arrays"...ok, they must have the same size
	//
	// void Get(double* out, const unsigned int* ids, int size) 
};

ostream& operator<<(ostream& os, const vector<double>& d)
{
	copy(begin(d), end(d), ostream_iterator<double>(os, " "));
	return os;
}

int main()
{
	call_sum_elements();

	Getter getter;
	cout << "Got:" << getter.Get(1) << "\n";

	vector<double> dst(3); vector<unsigned> ids{ 1,2,3 };
	
	Getter_GSL getterGSL;
	getterGSL.Get(as_span(dst), as_span(ids));
	cout << "Got:" << dst << "\n";

	double out[3]{};
	getterGSL.Get(as_span(out), as_span(ids));
	cout << "Got:" << out[0] << " " << out[1] << " " << out[2] << "\n";

	getterGSL.Get(as_span(dst), as_span(ids));
	cout << "Got:" << dst << "\n";
}

// BONUS: Only index into arrays using constant expressions

#define SOME_MAGIC_CONSTANT 10

void legacy_function(double(&magicArray)[SOME_MAGIC_CONSTANT])
{
	for (int i = 0; i < 20; ++i)
		magicArray[i] = i * 2; // warning C26482: Only index into arrays using constant expressions. (bounds.2)
}

void print_magic_coefficient(double(&magicArray)[SOME_MAGIC_CONSTANT])
{
	std::cout << magicArray[20]; // warning C26483: Value 20 is outside the bounds (0, 9) of variable 'magicArray'. Only index into arrays using constant expressions that are within bounds of the array. (bounds.2)
}

void gsl_legacy_function(double(&magicArray)[SOME_MAGIC_CONSTANT])
{
	for (size_t i = 0u; i < SOME_MAGIC_CONSTANT; ++i)
		at(magicArray, i) = i * 2; // bounds-checked
}