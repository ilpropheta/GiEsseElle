#pragma once
#include <yvals.h>
#include <algorithm>

// __str_find
template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
inline _SizeT 
	__str_find(const _CharT *__p, _SizeT __sz,
	_CharT __c, _SizeT __pos) _NOEXCEPT
{
	if (__pos >= __sz)
	return __npos;
	const _CharT* __r = _Traits::find(__p + __pos, __sz - __pos, __c);
	if (__r == 0)
		return __npos;
	return static_cast<_SizeT>(__r - __p);
}

	template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
inline _SizeT 
	__str_find(const _CharT *__p, _SizeT __sz,
	const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
{
	if (__pos > __sz || __sz - __pos < __n)
	return __npos;
	if (__n == 0)
		return __pos;
	const _CharT* __r = std::search(__p + __pos, __p + __sz, __s, __s + __n);
	if (__r == __p + __sz)
		return __npos;
	return static_cast<_SizeT>(__r - __p);
}


	// __str_rfind

	template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
inline _SizeT 
	__str_rfind(const _CharT *__p, _SizeT __sz,
	_CharT __c, _SizeT __pos) _NOEXCEPT
{
	if (__sz < 1)
	return __npos;
	if (__pos < __sz)
		++__pos;
	else
		__pos = __sz;
	for (const _CharT* __ps = __p + __pos; __ps != __p;)
	{
		if (_Traits::eq(*--__ps, __c))
			return static_cast<_SizeT>(__ps - __p);
	}
	return __npos;
}

	template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
inline _SizeT 
	__str_rfind(const _CharT *__p, _SizeT __sz,
	const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
{
	__pos = std::min(__pos, __sz);
	if (__n < __sz - __pos)
		__pos += __n;
	else
		__pos = __sz;
	const _CharT* __r = std::find_end(__p, __p + __pos, __s, __s + __n);
	if (__n > 0 && __r == __p + __pos)
		return __npos;
	return static_cast<_SizeT>(__r - __p);
}

	// __str_find_first_of
	template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
inline _SizeT 
	__str_find_first_of(const _CharT *__p, _SizeT __sz,
	const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
{
	if (__pos >= __sz || __n == 0)
	return __npos;
	[[gsl::suppress(bounds.1)]]
	{
		const _CharT* __r = std::find_first_of(__p + __pos, __p + __sz, __s, __s + __n);
		if (__r == __p + __sz)
			return __npos;
		return static_cast<_SizeT>(__r - __p);
	}
}


	// __str_find_last_of
	template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
inline _SizeT 
	__str_find_last_of(const _CharT *__p, _SizeT __sz,
	const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
{
	if (__n != 0)
	{
		if (__pos < __sz)
			++__pos;
		else
			__pos = __sz;
		for (const _CharT* __ps = __p + __pos; __ps != __p;)
		{
			const _CharT* __r = _Traits::find(__s, __n, *--__ps);
			if (__r)
				return static_cast<_SizeT>(__ps - __p);
		}
	}
	return __npos;
}


	// __str_find_first_not_of
	template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
inline _SizeT 
	__str_find_first_not_of(const _CharT *__p, _SizeT __sz,
	const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
{
	if (__pos < __sz)
	{
		const _CharT* __pe = __p + __sz;
		for (const _CharT* __ps = __p + __pos; __ps != __pe; ++__ps)
			if (_Traits::find(__s, __n, *__ps) == 0)
				return static_cast<_SizeT>(__ps - __p);
	}
	return __npos;
}


	template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
inline _SizeT 
	__str_find_first_not_of(const _CharT *__p, _SizeT __sz,
	_CharT __c, _SizeT __pos) _NOEXCEPT
{
	if (__pos < __sz)
	{
		[[gsl::suppress(bounds)]]
		{
			const _CharT* __pe = __p + __sz;
			for (const _CharT* __ps = __p + __pos; __ps != __pe; ++__ps)
				[[gsl::suppress(lifetime)]]
				{
					if (!_Traits::eq(*__ps, __c))
						return static_cast<_SizeT>(__ps - __p);
				}
		}
	}
	return __npos;
}


	// __str_find_last_not_of
	template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
inline _SizeT 
	__str_find_last_not_of(const _CharT *__p, _SizeT __sz,
	const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
{
	if (__pos < __sz)
	++__pos;
	else
		__pos = __sz;
	for (const _CharT* __ps = __p + __pos; __ps != __p;)
		if (_Traits::find(__s, __n, *--__ps) == 0)
			return static_cast<_SizeT>(__ps - __p);
	return __npos;
}


	template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
inline _SizeT 
	__str_find_last_not_of(const _CharT *__p, _SizeT __sz,
	_CharT __c, _SizeT __pos) _NOEXCEPT
{
	if (__pos < __sz)
	++__pos;
	else
		__pos = __sz;
	for (const _CharT* __ps = __p + __pos; __ps != __p;)
		if (!_Traits::eq(*--__ps, __c))
			return static_cast<_SizeT>(__ps - __p);
	return __npos;
}
