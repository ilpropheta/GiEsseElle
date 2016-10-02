
# GiEsseElle

`GiEsseElle` is a small demo project which shows usages of **Microsoft's Guidelines Support Library** ([GSL](https://github.com/Microsoft/GSL/)). Some warnings and potential issues may be spotted by running static analysis, extended by **Microsoft's Guidelines Checker** - [CppCoreCheck](https://blogs.msdn.microsoft.com/vcblog/2015/12/03/c-core-guidelines-checkers-available-for-vs-2015-update-1/). 

This demo supports my talk at Microsoft's event [Future Decoded 2016](https://www.microsoft.com/italy/futuredecoded/):

> Let's change C++ with Microsoft GSL and Guidelines Checkers

Slides of my talk here:

## Set up

`GiEsseElle` consists in 5 small subprojects each showing a different aspect to check and fix:

- `GSL.Bounds`, on how to use `gsl::span` and bounds checking
- `GSL.Type`, on how to use `gsl::narrow` and type checking
- `GSL.Lifetime`, on how to use `gsl::final_act` and lifetime checking
- `GSL.Interfaces`, on how to use `gsl::not_null`
- `Cpp17.StringView`, on how to use `std::string_view`

`GiEsseElle.sln` has been developed on **Visual Studio 2015 Update 3**. Each project depends on a couple of NuGet packages:

- `Microsoft.GSL`
- `Microsoft.CppCoreCheck`

Actually, adding a dependency to the latter will automatically add a dependency to the former as well.

Each project is self-contained and can be compiled, analyzed and run separately from the others (this is valuable for showing demos quickly).

### Important note on the GSL

`GSL` is work in progress and thus it exhibits many bugs and omissions. The folder `GSL-edited.files` contains a couple of files that was missing a couple of things in the original GSL.

## Philosophy

For more details have a look at the [C++ Core Guidelines repository](https://github.com/isocpp/CppCoreGuidelines). 

Although `GSL` and `CppCoreCheck` are - in my opinion - not ready for being seamlessly employed in medium/large codebases, many ideas and constructs have been successfully used in C++ development for years. In particular, the C++ Core Guidelines are based on 4 basic concepts that *might be* valuable in professional development:

- Correct-by-Construction
- Fail-Fast
- Turning undefined behavior into well-known expectations
- Using types instead of pointers

### Correct-by-Construction

Making errors happen at compile-time instead of run-time implies that some issues simply do not compile. 

Not only is this achievable by adopting special types, but also by verifying programs with particular checkers that may spot dangerous scenarios.

The C++ Core Guidelines project aims to achieve this ambitious result and Microsoft's `CppCoreCheck` is an experimental extension on top of Visual Studio static analysis that - partially - supports some of the checkable guidelines.

An example of Correct-by-Construction methodology in the GSL consists in adopting `gsl::not_null` for passing around not-nullable pointers, because constructing an instance of `not_null` by simply passing`nullptr` is forbidden:

    int* ptr = nullptr;
    SomeFunction(*ptr); // undefined behavior
    
    gsl::not_null<int*> ptr = nullptr; // does not compile

### Fail-Fast

When a problem cannot simply be detected at compile-time, failing as soon as the problem appears is a common idiom.

Failing can result in different actions, like:

- throwing an exception
- aborting the program

The GSL supports also the "do-nothing" option, that is necessary especially for a **gradual adoption** of these concepts. Failing policies can be set with preprocessor macros:

- `GSL_TERMINATE_ON_CONTRACT_VIOLATION` (default): std::terminate will be called
- `GSL_THROW_ON_CONTRACT_VIOLATION`: `gsl::fail_fast` exception will be thrown
- `GSL_UNENFORCED_ON_CONTRACT_VIOLATION`: nothing happens

The former is also useful for **gradually** adopt of these ideas.

Fail-fast means also refusing to construct an object - while an object is being constructed - in case an invariant is broken.
Let's go back to the previous example. What happens when `not_null` is constructed with a pointer that is null? That is:

    int* ptr = nullptr;
    not_null<int*> nptr = ptr;
    SomeFunction(*nptr);

Does `nptr` make sense to exist? Not at all! For this reason, `not_null`'s constructor expects the pointer is valid, otherwise it fails according to the choosen policy.

### Turning undefined behavior into well-known expectations

Consider again the very first example:

    void SomeFunction(int& i);

    int* ptr = nullptr;
    SomeFunction(*ptr); // undefined behavior

Dereferencing a `nullptr` results in **undefined behavior**, that is basically "the compiler can do everything". Sometimes "everything" means it may go on with the execution and then call SomeFunction with a reference to a *broken* int value. Suppose now we turn the function's signature into:

    void SomeFunction(not_null<int*> i);

What's changed? In case `ptr` is `nullptr`, we have turned an undefined behavior into a well-defined policy violation that, for instance, results in throwing an exception. Another solution would have been checking the pointer. However this should be done everywhere `SomeFunction` is used.

Using a reference is the best option from both efficiency, simplicity and portability. However, a reference just **shouldn't** be null, instead `gsl::not_null` will never be.

When I see`not_null`, I see kind of a **barrier** in the sourcecode.

`not_null` is precious also to ensure that functions return valid pointer-like objects:

    not_null<shared_ptr<IService>> CreateService(...params...);

`CreateService` ensures the returned `shared_ptr` will be always valid, otherwise the invariant is broken and a failure is expected. In this particular case I have heard of different 
point of views about this in real code:

- some people just don't expect that `not_null`-returning functions throw exceptions;
- others think they can and expect a valid pointer or an exception.

Formally, returning `not_null` may result in violating the invariant and then in breaking the policy. So, whenever I see a function which returns not_null I assume it may throw an exception (because I usually set policy violation to throwing exceptions).

Another example of **Fail-Fast** is **Bounds-Checking**. What happens here?

    int arr[] = {1,2,3};
    int idx = 4;
    arr[idx] = 10; // undefined behavior

Again, we can turn undefined behavior into an exception (or something else) just by using a bounds-checked type. In the next section we'll learn what GSL proposes for doing that.

### Using types instead of pointers

In my talk "Great C++ comes with great responsibility" I pronounced for the first time the term I coined to describe pointers whose meaning is not clear because it depends on how they are actually used: "Factotum Pointers". Factotum pointers may actually represent different things, and to figure out the intended usage the poor programmer has to usually look into the code they affect. For example, what does the following function take as a parameter?

    void Function(int* p);

The intent of p is not clear:

- pointer to a single element
- pointer to a dynamically-allocated sequence of elements
- pointer to a statically-allocated sequence of elements
- a position that can be changed/incremented/subtracted

And also, we don't know if:

- p owns a resource that should be deleted after Function is called
- Function allows p to be null

So clients of `Function` have to figure out how `p` is used and what happens at the boundaries of the function call.

We know many guidelines to avoid such problems. For example, we know that using a raw pointer to pass around ownership is dangerous and unclear. Smart pointers, containers or custom wrappers are generally safer alternatives.

What about expressing that p may not be null? We have references and `gsl::not_null`, that we just met.

What about the difference between single element and sequence? Here we need something else. The C++ Core Guidelines propose:

- never use pointers to represent and pass arrays
- never do pointer arithmetic directly
- access static arrays only with constant indexes, or via bounds-checked functions (e.g. `gsl::at`)
- pass around naked ranges/sequences as `span`

The message is clear: pointers should only represents "nullable references to single objects".

`span` is the clearer and safer alternative to represent contiguous ranges. `span` is the safer and clearer alternative for C-style arrays.

The previous example becomes:

    int arr[] = {1,2,3};
    span<int> sp = arr;
    int idx = 4;
    sp[idx] = 10; // bounds-checked

GSL actually provides also:

    gsl::at(arr, idx); // bounds-checked

That is more succint and works also with `std::array,` `std::initializer_list` and containers providing both `operator[]` and `size()`. And it is declared `constexpr`.

## Checking the C++ Core Guidelines

An ambitious part of the Guidelines project aims to define kind of **standard static analysis**, that means showing exactly the same warning on every analysis tool that checks the same piece of code.

Checkable guidelines are subdivided into three **Safety Profiles**:

- Type
- Bounds
- Lifetime

The daring aim of the project is to guarantee that an aspect of a program is safe as far as each guideline of the corresponding profile is correctly observed.

**Type safety** guarantees no use of a location as a T that contains an unrelated U; **bounds safety** guarantees no accesses beyond the bounds of an allocation; finally, **lifetime safety** ensures no use of invalid or deallocated allocations.

`CppCoreCheck` is Microsoft's experimental extension of Visual Studio's static analysis, which enables Visual Studio to check these profiles.

Since `CppCoreCheck` is experimental, I have tried it on many small and non-critical activities, including:

- Training
- Experimenting on legacy code
- Developing little toy projects

Although - in my opinion - we are far away to use it on real-world medium/large codebases full of issues (e.g. C-style code and old-fashioned C++), I have found it useful as a tool that helps detect what the C++ Core Guidelines call the attention to. Whenever the checker complains with messages like:

> No array to pointer decay

You can evaluate if it makes sense - for instance - to replace the pointer parameter with `span` or to simply stop passing an array as a pointer.

This tool may be useful to train yourself or a team to embrace some/many ideas of the C++ Core Guidelines.

### GSL for annotating code

How many times do we cast types into others? Are we always on the safe side?

For example, casting a double into an int is an operation which may cause a loss of information. This behavior is sometimes expected, other times is not. On large codebases, it's extremely useful to quickly spot all the places where we perform such possibly lossy conversions.

GSL provides `gsl::narrow_cast`, that under the hood is just a `static_cast`. What I found useful of that function is its **annotation** power. In a ideal world, I would like seeing a warning whenever a narrowing conversion is required - regardless the cast is C-style or C++-style. Then I accept the warning and I make a decision: I can just "mark" the cast as acceptable (this will help me locate quickly every piece of code I requested a narrowing conversion that know it can fail), or - adopting the **Fail-Fast** pattern - I request a failure if the conversion loses information (this "strict" decision is quickly searchable as well).

GSL provides both alternatives:

    auto cmd = gsl::narrow_cast<int>(doubleParameter); // just annotated
    
    auto cmd = gsl::narrow<int>(doubleParameter); // throw gsl::narrowing_error if cast is lossy

`gsl::narrow` throws an exception (regardless of the failure policy) whenever the cast is really lossy. Instead, `gsl::narrow_cast` is only used to annotate an acceptable possibly lossy cast.

Both `gsl::narrow` and `gsl::narrow_cast` result useful in reviewing and refactoring code.

`not_null` is another example of both invariant checker and code marker.

### Manual Suppression

C++ Core Guidelines are intended for a **gradual adoption**. Generally it's not possible to embrace such a bunch of things in one go. Moreover, many companies may just consider unacceptable to adopt some guidelines.

In case we want to manually disable the check for a piece of code, we use `[[gsl::suppress]]` attribute. For example:

    void many_arrays_operation(const double* in1, const double* in2, double* out, int size)
    {
    	for (int i = 0; i <= size; i++)
    	{
    		[[gsl::suppress(bounds.1)]] // performance critical code (cannot do bounds-checking)
    		{
    			out[i] = in1[i] + in2[i]; 
    		}
    
    	}
    }

As you see, suppressing an issue affects a scope. `gsl::suppress` takes the name of the profile and, optionally, the number of the rule to disable.

## Exploring GiEsseElle

`GiEsseElle` contains a few examples of applying these ideas. Each project contains comments with more information about the code. `CppCoreCheck` can be used to verify the guidelines.

### Cpp17.StringView

This project shows a few examples of [std::string_view](http://en.cppreference.com/w/cpp/experimental/basic_string_view) (approved for C++17), a non-owning wrapper on a const sequence of characters, providing the same interface of `const` `std::string` plus some additions.

`string_view` enables C++ programmers to unify different string representations under a common, well-known, interface.

`Cpp17.StringView`'s `main.cpp` contains a `split` function that divides a string into tokens according to an array of delimiters. The first implementation is inconvenient because it returns a `vector<string>`, resulting in allocating substrings.

In the second implementation, instead, `string_view` kicks in. This time, rather than creating a `vector` of new strings, the `vector` just contains views on the original `string`, resulting in better memory usage.

Other examples show how `string_view` can be used to do logical operations on strings, like trimming. This can be combined with **C++14's heterogeneous lookup** to exploit *transparent comparators*:

    //					v--- this is a *transparent* comparator
    map<string, int, less<>> mm{{"a", 1}, {"b", 2}};
           
    auto entry = string_view("  a");
    // logical trim
    entry = entry.substr(entry.find_first_not_of(' '));
     //			     v--- this won't create a temporary std::string!
    cout << mm.find(entry)->second;

### GSL.Bounds

This project contains simple code that shows how to embrace the **Bounds Safety Profile**. This profile makes it easier to construct code that operates within the bounds of allocated blocks of memory. It does so by focusing on removing the primary sources of bounds violations: pointer arithmetic and array indexing. One of the core features of this profile is to **restrict pointers to only refer to single objects, not arrays**.

`span` is a bounds-checked, safe type for accessing arrays of data, that - according to the guidelines - should replace each usage of pointers that actually refer to arrays.

An example contained in `GSL.Bounds` is a simple function which adds a range of ints together:

    int sum_elements(int* arr, int size)
    {
    	int sum = 0; 
    	for (int i = 0; i < size; i++)
    	{
    		sum += arr[i]; //  warning C26481: Don't use pointer arithmetic. Use span instead. (bounds.1)
    	}
    	return sum;
    }

As you see, `CppCoreCheck` shows a warning because the pointer is accessed as an array. The corresponding profile guideline ([bounds.1](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#bounds1-dont-use-pointer-arithmetic-use-span-instead)) recommends to use `span` instead:

    int sum_elements(span<int> range)
    {
    	int sum = 0;
    	for (int i = 0; i < range.size(); i++)
    	{
    		sum += range[i]; // run-time checked
    	}
    	return sum;
    }

Actually, we can do even better, by following [another guideline](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Res-lib) that states: **Prefer the standard library to other libraries and to "handcrafted code"**:

    int modern_sum_elements(span<int> range)
    {
    	return accumulate(begin(range), end(range), 0);
    }

### GSL.Interfaces

This project is devoted to `not_null`.

A [guideline](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Ri-nullptr) from the Interface group states:

> Declare a pointer that must not be null as `not_null`.
> To help avoid dereferencing nullptr errors. To improve performance by
> avoiding redundant checks for nullptr.

Consider these functions:

    void UseService(Service* s) { s->Do(); }
    
    void UseService(Service& s) { s.Do(); }
    
    void UseService(gsl::not_null<Service*> s) { s->Do(); }

Which differences do they show from the client perspective?

In the first, it's not clear if Service* may be null or not. Looking at the implementation (that may be not feasible) we understand it may not be.

The second function expects a reference. Consider now what happens here:

    Service* s = nullptr; // or coming from somewhere
    UseService(*s);

What do you think it happens on the last line? A crash? An exception? Neither? The formal answer is **undefined behavior**, as we already discussed on a previous section of this document.

Now, let's consider the last function and how to call it:

    Service* s = nullptr;
    UseService(gsl::not_null<Service*>{s});

In this case the have turned undefined behavior into the failure policy we have chosen (e.g. throwing `gsl::fail_fast`). Remember also that this code is forbidden:

    gsl::not_null<Service*> s = nullptr;

`not_null` simply cannot hold `nullptr`. This exhibits the so-called **Correct-By-Construction** behavior.

Moreover, by stating the intent in source, implementers and tools can provide better diagnostics, such as finding some classes of errors through static analysis, and perform optimizations, such as removing branches and null tests.

### GSL.Lifetime

This project shows a simple example of dangerous code that `CppCoreCheck` catches as a lifetime issue, and also how to use `gsl:final_at` as a **portable destructor**:

    void LogThisFunctionExit()
    {
    	auto finalizer = gsl::finally([name = __FUNCTION__]{ 
    	    cout << "Done: " << name << endl; 
    	});
    	
    	cout << "This function is running\n";
    	cout << "Still running...";
    }

### GSL.Type

This project demonstrates how to use `gsl::narrow` and `gsl::narrow_cast`, and it shows uninitialization of member variables warnings.

As discussed here, some GSL's tools are intended also for **annotating** the code. Narrowing conversions are an example. Whenever you request a conversion from double to int, the cast can actually destroys information (the decimal part, if any). Such casts are called **narrowing**. You know, C++11's uniform initialization forbids narrowing:

    int i {1.0}; // does not compile

A quick-fix to this problem consists in using C-style cast:

    auto i = int(1.0); // ok...

If you manage lots of code, it's somehow useful to "mark" points in code where you perform such potentially lossy conversions. GSL offers a narrow operation for specifying that narrowing is acceptable and a narrow ("narrow if") that throws an exception if a narrowing would throw away information: 

    auto i = gsl::narrow_cast<int>(1.0); // marked as acceptable

Embracing the Fail-Fast methodology, we want to catch a failure at runtime. In this drastic scenarios we may use `gsl::narrow`:

    auto i = gsl::narrow_cast<int>(1.3); // will throw gsl::narrowing_error

## Caveats

### 2 or more `span` of the same length

It's common to declare functions taking many arrays of the same length and possibly of different types. For example:

    void twofft(float data1[], float data2[]. float fft1[], float fft2[], unsigned long len);

(also with different types).

According to the Guidelines, each array should be replaced with span. In this case, apart from copying the length for each span (that may be or not a performance penalty - and, when in doubt, should be measured), the worst problem is that the interface of the function loses the common size requirement.

Although the interface requirement is already poor because nothing really makes an enforcement, we should be pragmatic and admit that the current standard "de facto" for passing around sequences of the same length is such approach, well-known by C and C++ programmers.

In an ideal world I would express the requirement through contracts, but we don't have them yet into the language.

We may set up an expectation (e.g. using GSL's `Expects`). However, this is still an implementation detail and not really an interface contract (indeed, it's stated in the body of the function).

You can code your own "multiple spans, sharing the same length" type but actually I did it and these are the worst feedbacks I got from people co-working with me on a project involving such experimental type: "we have lost the name of the parameters" and "declaring such tied_span is too complicated with more than 2 arrays of different type". 

And worse, many times they just ignored the tied_span and fell back to the old pointers + length approach.

So, in some cases, span alone is simply not enough.

### `not_null<unique_ptr<T>>`

It's not supported. What do you expect to be into a moved-from not_null?!

Conversely, [Dropbox's nn](https://github.com/dropbox/nn) supports `unique_ptr`.

### `span` vs array static size safety

More details [here](https://github.com/Microsoft/GSL/issues/214).

In short, the compiler can automatically refuse to compile if you try to decay a pointer into a reference to an array:

    void foo(int(&)[3]);
    
    // this will fail to compile:
    //
    int* values = new int[2];
    foo(values);

The same happens if you try to pass an array of wrong size:

    // this will fail to compile:
    //
    int values[2]
    foo(values)

On the other hand, this safety is not maintained with `span`:

    // void foo(gsl::span<int, 3>);
    //
    span<int> values;
    foo(values); // compile-ok, runtime fail
    //
    int* values...
    foo({values, 2}); // compile-ok, runtime fail
