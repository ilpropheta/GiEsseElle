#include "gsl.h"
#include <map>
#include <random>
#include <vector>
#include <iostream>

using namespace std;
using namespace gsl;

// what happens if str is nullptr?
string BuildString(const char* str)
{
	return str;
}

// I.12: Declare a pointer that must not be null as not_null

string BuildString(gsl::not_null<const char*> str) // GSL declares also gsl::czstring<> = const char*
{
	return str.get();
}

// Factory example

class IDataSource
{
public:
	virtual ~IDataSource() = default;
	virtual double Get(const string& key) = 0;
};

class MemoryDataSource : public IDataSource
{
	const map<string, double> data{
		{ "param1"s, 1.0 }
	};

public:
	double Get(const string & key) override
	{
		return data.at(key);
	}
};

class EmptyDataSource : public IDataSource
{
public:
	double Get(const string & key) override
	{
		return 0.0;
	}
};

// intent: CreateDataSource always returns a valid shared_ptr
// note: formally this function can also throw an exception trying to
// create a not_null from a nullptr
gsl::not_null<std::shared_ptr<IDataSource>> CreateDataSource(const std::string& cmdLine)
{
	if (cmdLine.find_first_of("memory") == 0)
	{
		return make_shared<MemoryDataSource>();
	}
	return make_shared<EmptyDataSource>();
}

// not_null as *barrier*

struct Service
{
	void Do()
	{
		cout << "Service used\n";
	}
};

Service* GetService()
{
	// suppose it performs a connection to something
	// so it can return nullptr...
	static Service s;
	std::random_device rd;
	uniform_int_distribution<int> success(0,5);
	if (success(rd)) // connection granted?
	{
		return &s;
	}
	return nullptr;
}

// it's not clear if the service can be nullptr
void Legacy_UseService(Service* service) 
{
	// actually it shouldn't, 
	// but we had to look at the implementation to understand it
	service->Do(); 
}

// clear: cannot handle null service
void Safe_UseService(not_null<Service*> service) 
{
	// let's wrap around legacy code
	// (actually, we can even hide the legacy function from the client code)
	Legacy_UseService(service); 
}

// *shouldn't* handle nullptr service...(see main)
void Safe_UseService(Service& service)
{
	service.Do();
}

int main()
{
	Service* s = nullptr;
	Safe_UseService(*s); // undefined behavior

	// not_null<Service*> s = nullptr; // cannot compile
	not_null<Service*> ns = GetService(); // run-time check
	// kind of a *barrier*

	Safe_UseService(*ns); // if we get here, s.get() is not null
	
	// not_null works with pointer-like types
	// that is: everything that can be checked for validity:

	std::function<void()> empty;
	not_null<std::function<void()>> sf(empty); // invariant broken
}