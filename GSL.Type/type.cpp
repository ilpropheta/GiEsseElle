#include "gsl_util.h"
#include <iostream>
#include <vector>

using namespace std;

struct SimulationParams
{
	double coeff1, coeff2, coeff3;
};

double Simulate(const SimulationParams& params)
{
	// this warning is licit but the explanation does not make sense
	// https://github.com/Microsoft/GSL/issues/307

	int cmd = static_cast<int>(params.coeff1);
	//int cmd = gsl::narrow_cast<int>(params.coeff1); // annotate only
	//int cmd = gsl::narrow<int>(params.coeff1);	  // fail in case of loss
	// 
	// ^____ play with these lines and run the code
	switch (cmd)
	{
	case 1:
		return params.coeff1 + params.coeff2;
		break;
	case 2:
		return params.coeff1 * params.coeff2;
		break;
	default:
		return numeric_limits<double>::quiet_NaN();
		break;
	}
}

struct LegacyClass
{
	LegacyClass() : i(10), j(20)
	{

	}

	LegacyClass(const LegacyClass& other) : i(other.i) // j should be initialized as well
	{

	}

	/* 	If at some point we manage to add another member variable,
		the warning reminds that each special operator should be updated accordingly.
		
		Note: this example is trivial and can be turned into:
		
			struct Pippo { int i, j; }; // rule of zero
		
		Rule of zero should be applied as much as possible. However, sometimes it's not
		the case (e.g. you cannot easily change/refactor legacy code),
		thus the warning is helpful.
	*/
		
	int i, j; 
	  //   ^___ this is new
};

int main()
{	
	LegacyClass lc;
	auto lc2 = lc;
	cout << lc2.i << ", " << lc2.j << endl;
	
	try
	{
		cout << Simulate({ 1.1, 2.0, 3.0 }) << endl;
	}
	catch (const gsl::narrowing_error&)
	{
		cout << "cmd is not integer\n";
	}
}
