#include "gsl.h"
#include "span.h"
#include "gsl_util.h"
#include <iostream>
#include <vector>

using namespace std;

// we want to log when this function terminates
void LogThisFunctionExit()
{
	// gsl::final_act is like BOOST_SCOPE_EXIT
	// or ScopeGuard by Loki
	// it's kind of a *portable destructor*
	auto finalizer = gsl::finally([name = __FUNCTION__]{ 
		cout << "Done: " << name << endl;
	});
	
	cout << "This function is running\n";
	cout << "Still running...";
}

// Another useful example: using external APIs quickly

// imagine that this bunch of function is completely our of your control
// and also that they use the classical "token pattern"
// to maintain the state across several calls:

void* Initialize() { return{}; } // returns the opaque state
void Finalize(void* token) { delete static_cast<int*>(token); } // destroy the state
void Use(void* token) {} // use the state

// this is your code
void UseApi()
{
	auto token = Initialize();
	auto finalizer = gsl::finally([=] {Finalize(token); });
	Use(token);

	/* Note: this approach is quick and should be used only when the
			 usage is really localized and not repeated.
			 Conversely, a proper RAII wrapper is preferred.
	*/
}

// An example of the Lifetime Checker in action:
void Invalidate()
{
	vector<int> vv;
	vv.push_back(10);
	auto first = &vv[0];
	vv.push_back(20); // Do not dereference an invalid pointer (lifetimes rule 1)
	cout << *first;
}

int main()
{
	LogThisFunctionExit();
}