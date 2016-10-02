// Freely adapted from libcxx - https://llvm.org/svn/llvm-project/libcxx/trunk/include/experimental/string_view (June 24th, 2016)
#pragma once

#include <string>
#include <algorithm>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <iomanip>
#include "string_find_utils.h"

namespace experimental
{
    template<class _CharT, class _Traits = std::char_traits<_CharT> >
    class basic_string_view 
	{
    public:
        // types
        typedef _Traits                                    traits_type;
        typedef _CharT                                     value_type;
        typedef const _CharT*                              pointer;
        typedef const _CharT*                              const_pointer;
        typedef const _CharT&                              reference;
        typedef const _CharT&                              const_reference;
        typedef const_pointer                              const_iterator; // See [string.view.iterators]
        typedef const_iterator                             iterator;
        typedef std::reverse_iterator<const_iterator>	   const_reverse_iterator;
        typedef const_reverse_iterator                     reverse_iterator;
        typedef size_t                                     size_type;
        typedef ptrdiff_t                                  difference_type;
        static  const size_type npos = size_type(-1);

        // [string.view.cons], construct/copy
        
        basic_string_view() _NOEXCEPT : __data (nullptr), __size(0) {}

        basic_string_view(const basic_string_view&) = default;
        
        basic_string_view& operator=(const basic_string_view&) = default;

        template<class _Allocator>
        
        basic_string_view(const std::basic_string<_CharT, _Traits, _Allocator>& __str) _NOEXCEPT
            : __data (__str.data()), __size(__str.size()) {}

        
        basic_string_view(const _CharT* __s, size_type __len)
            : __data(__s), __size(__len)
        {
        }

        
        basic_string_view(const _CharT* __s)
            : __data(__s), __size(_Traits::length(__s)) {}

        // [string.view.iterators], iterators
        
        const_iterator begin()  const _NOEXCEPT { return cbegin(); }

        
        const_iterator end()    const _NOEXCEPT { return cend(); }

        
        const_iterator cbegin() const _NOEXCEPT { return __data; }

        
		const_iterator cend()   const _NOEXCEPT { [[gsl::suppress(bounds.1)]]{ return __data + __size; } }

        
        const_reverse_iterator rbegin()   const _NOEXCEPT { return const_reverse_iterator(cend()); }

        
        const_reverse_iterator rend()     const _NOEXCEPT { return const_reverse_iterator(cbegin()); }

        
        const_reverse_iterator crbegin()  const _NOEXCEPT { return const_reverse_iterator(cend()); }

        
        const_reverse_iterator crend()    const _NOEXCEPT { return const_reverse_iterator(cbegin()); }

        // [string.view.capacity], capacity
        
        size_type size()     const _NOEXCEPT { return __size; }

        
        size_type length()   const _NOEXCEPT { return __size; }

        
        size_type max_size() const _NOEXCEPT { return std::numeric_limits<size_type>::max(); }

         bool 
        empty()         const _NOEXCEPT { return __size == 0; }

        // [string.view.access], element access
        
        const_reference operator[](size_type __pos) const { return __data[__pos]; }

        
        const_reference at(size_type __pos) const
        {
            return __pos >= size()
                ? throw(out_of_range("string_view::at"))
                : __data[__pos];
        }

        
        const_reference front() const
        {
            return __data[0];
        }

        
        const_reference back() const
        {
            return __data[__size-1];
        }

        
        const_pointer data() const _NOEXCEPT { return __data; }

        // [string.view.modifiers], modifiers:
         
        void clear() _NOEXCEPT
        {
            __data = nullptr;
            __size = 0;
        }

         
        void remove_prefix(size_type __n) _NOEXCEPT
        {
            __data += __n;
            __size -= __n;
        }

         
        void remove_suffix(size_type __n) _NOEXCEPT
        {
            __size -= __n;
        }

         
        void swap(basic_string_view& __other) _NOEXCEPT
        {
            const value_type *__p = __data;
            __data = __other.__data;
            __other.__data = __p;

            size_type __sz = __size;
            __size = __other.__size;
            __other.__size = __sz;
//             std::swap( __data, __other.__data );
//             std::swap( __size, __other.__size );
        }

        // [string.view.ops], string operations:
        template<class _Allocator>
        
		explicit operator std::basic_string<_CharT, _Traits, _Allocator>() const
        { return basic_string<_CharT, _Traits, _Allocator>( begin(), end()); }

        template<class _Allocator = allocator<_CharT> >
        
		std::basic_string<_CharT, _Traits, _Allocator>
        to_string( const _Allocator& __a = _Allocator()) const
		{
			return std::basic_string<_CharT, _Traits, _Allocator>(begin(), end(), __a);
		}

        size_type copy(_CharT* __s, size_type __n, size_type __pos = 0) const
        {
            if ( __pos > size())
                throw(out_of_range("string_view::copy"));
            size_type __rlen = std::min( __n, size() - __pos );
			memcpy_s(__s, __rlen, begin() + __pos, __rlen);
			//std::copy_n(begin() + __pos, __rlen, __s ); // this causes infamous C4996 warning
            return __rlen;
        }
        
        basic_string_view substr(size_type __pos = 0, size_type __n = npos) const
        {
			[[gsl::suppress(bounds.1)]]
			{
				return __pos > size()
					? throw((out_of_range("string_view::substr")))
					: basic_string_view(data() + __pos, std::min(__n, size() - __pos));
			}
        }

         int compare(basic_string_view __sv) const _NOEXCEPT
        {
            size_type __rlen = std::min( size(), __sv.size());
            int __retval = _Traits::compare(data(), __sv.data(), __rlen);
            if ( __retval == 0 ) // first __rlen chars matched
                __retval = size() == __sv.size() ? 0 : ( size() < __sv.size() ? -1 : 1 );
            return __retval;
        }

         
        int compare(size_type __pos1, size_type __n1, basic_string_view __sv) const
        {
            return substr(__pos1, __n1).compare(__sv);
        }

         
        int compare(                       size_type __pos1, size_type __n1, 
                    basic_string_view _sv, size_type __pos2, size_type __n2) const
        {
            return substr(__pos1, __n1).compare(_sv.substr(__pos2, __n2));
        }

         
        int compare(const _CharT* __s) const
        {
            return compare(basic_string_view(__s));
        }

         
        int compare(size_type __pos1, size_type __n1, const _CharT* __s) const
        {
            return substr(__pos1, __n1).compare(basic_string_view(__s));
        }

         
        int compare(size_type __pos1, size_type __n1, const _CharT* __s, size_type __n2) const
        {
            return substr(__pos1, __n1).compare(basic_string_view(__s, __n2));
        }

        // find
         
        size_type find(basic_string_view __s, size_type __pos = 0) const _NOEXCEPT
        {
            return __str_find<value_type, size_type, traits_type, npos>
                (data(), size(), __s.data(), __pos, __s.size());
        }
		         
        size_type find(_CharT __c, size_type __pos = 0) const _NOEXCEPT
        {
			return __str_find<value_type, size_type, traits_type, npos>
                (data(), size(), __c, __pos);
        }

         
        size_type find(const _CharT* __s, size_type __pos, size_type __n) const
        {
            return __str_find<value_type, size_type, traits_type, npos>
                (data(), size(), __s, __pos, __n);
        }

         
        size_type find(const _CharT* __s, size_type __pos = 0) const
        {
            return __str_find<value_type, size_type, traits_type, npos>
                (data(), size(), __s, __pos, traits_type::length(__s));
        }

        // rfind
         
        size_type rfind(basic_string_view __s, size_type __pos = npos) const _NOEXCEPT
        {
            return __str_rfind<value_type, size_type, traits_type, npos>
                (data(), size(), __s.data(), __pos, __s.size());
        }

         
        size_type rfind(_CharT __c, size_type __pos = npos) const _NOEXCEPT
        {
            return __str_rfind<value_type, size_type, traits_type, npos>
                (data(), size(), __c, __pos);
        }

         
        size_type rfind(const _CharT* __s, size_type __pos, size_type __n) const
        {
            return __str_rfind<value_type, size_type, traits_type, npos>
                (data(), size(), __s, __pos, __n);
        }

         
        size_type rfind(const _CharT* __s, size_type __pos=npos) const
        {
            return __str_rfind<value_type, size_type, traits_type, npos>
                (data(), size(), __s, __pos, traits_type::length(__s));
        }

        // find_first_of
         
        size_type find_first_of(basic_string_view __s, size_type __pos = 0) const _NOEXCEPT
        {
            return __str_find_first_of<value_type, size_type, traits_type, npos>
                (data(), size(), __s.data(), __pos, __s.size());
        }

         
        size_type find_first_of(_CharT __c, size_type __pos = 0) const _NOEXCEPT
        { return find(__c, __pos); }

         
        size_type find_first_of(const _CharT* __s, size_type __pos, size_type __n) const
        {
            return __str_find_first_of<value_type, size_type, traits_type, npos>
                (data(), size(), __s, __pos, __n);
        }

         
        size_type find_first_of(const _CharT* __s, size_type __pos=0) const
        {
            return __str_find_first_of<value_type, size_type, traits_type, npos>
                (data(), size(), __s, __pos, traits_type::length(__s));
        }

        // find_last_of
         
        size_type find_last_of(basic_string_view __s, size_type __pos=npos) const _NOEXCEPT
        {
            return __str_find_last_of<value_type, size_type, traits_type, npos>
                (data(), size(), __s.data(), __pos, __s.size());
        }

         
        size_type find_last_of(_CharT __c, size_type __pos = npos) const _NOEXCEPT
        { return rfind(__c, __pos); }

         
        size_type find_last_of(const _CharT* __s, size_type __pos, size_type __n) const
        {
            return __str_find_last_of<value_type, size_type, traits_type, npos>
                (data(), size(), __s, __pos, __n);
        }

         
        size_type find_last_of(const _CharT* __s, size_type __pos=npos) const
        {
            return __str_find_last_of<value_type, size_type, traits_type, npos>
                (data(), size(), __s, __pos, traits_type::length(__s));
        }

        // find_first_not_of
         
        size_type find_first_not_of(basic_string_view __s, size_type __pos=0) const _NOEXCEPT
        {
            return __str_find_first_not_of<value_type, size_type, traits_type, npos>
                (data(), size(), __s.data(), __pos, __s.size());
        }

         
        size_type find_first_not_of(_CharT __c, size_type __pos=0) const _NOEXCEPT
        {
            return __str_find_first_not_of<value_type, size_type, traits_type, npos>
                (data(), size(), __c, __pos);
        }

         
        size_type find_first_not_of(const _CharT* __s, size_type __pos, size_type __n) const
        {
            return __str_find_first_not_of<value_type, size_type, traits_type, npos>
                (data(), size(), __s, __pos, __n);
        }

         
        size_type find_first_not_of(const _CharT* __s, size_type __pos=0) const
        {
            return __str_find_first_not_of<value_type, size_type, traits_type, npos>
                (data(), size(), __s, __pos, traits_type::length(__s));
        }

        // find_last_not_of
         
        size_type find_last_not_of(basic_string_view __s, size_type __pos=npos) const _NOEXCEPT
        {
            return __str_find_last_not_of<value_type, size_type, traits_type, npos>
                (data(), size(), __s.data(), __pos, __s.size());
        }

         
        size_type find_last_not_of(_CharT __c, size_type __pos=npos) const _NOEXCEPT
        {
            return __str_find_last_not_of<value_type, size_type, traits_type, npos>
                (data(), size(), __c, __pos);
        }

         
        size_type find_last_not_of(const _CharT* __s, size_type __pos, size_type __n) const
        {
            return __str_find_last_not_of<value_type, size_type, traits_type, npos>
                (data(), size(), __s, __pos, __n);
        }

         
        size_type find_last_not_of(const _CharT* __s, size_type __pos=npos) const
        {
            return __str_find_last_not_of<value_type, size_type, traits_type, npos>
                (data(), size(), __s, __pos, traits_type::length(__s));
        }

    private:
        const   value_type* __data;
        size_type           __size;
    };


    // [string.view.comparison]
    // operator ==
    template<class _CharT, class _Traits>
    bool operator==(basic_string_view<_CharT, _Traits> __lhs,
                    basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
    {
        if ( __lhs.size() != __rhs.size()) return false;
        return __lhs.compare(__rhs) == 0;
    }

    template<class _CharT, class _Traits>
    bool operator==(basic_string_view<_CharT, _Traits> __lhs,
                    typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
    {
        if ( __lhs.size() != __rhs.size()) return false;
        return __lhs.compare(__rhs) == 0;
    }

    template<class _CharT, class _Traits>
    bool operator==(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs, 
                    basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
    {
        if ( __lhs.size() != __rhs.size()) return false;
        return __lhs.compare(__rhs) == 0;
    }


    // operator !=
    template<class _CharT, class _Traits>
    bool operator!=(basic_string_view<_CharT, _Traits> __lhs, basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
    {
        if ( __lhs.size() != __rhs.size())
            return true;
        return __lhs.compare(__rhs) != 0;
    }

    template<class _CharT, class _Traits>
    bool operator!=(basic_string_view<_CharT, _Traits> __lhs,
                    typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
    {
        if ( __lhs.size() != __rhs.size())
            return true;
        return __lhs.compare(__rhs) != 0;
    }

    template<class _CharT, class _Traits>
    bool operator!=(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs, 
                    basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
    {
        if ( __lhs.size() != __rhs.size())
            return true;
        return __lhs.compare(__rhs) != 0;
    }


    // operator <
    template<class _CharT, class _Traits>
    bool operator<(basic_string_view<_CharT, _Traits> __lhs, basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
    {
        return __lhs.compare(__rhs) < 0;
    }

    template<class _CharT, class _Traits>
    bool operator<(basic_string_view<_CharT, _Traits> __lhs,
                    typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
    {
        return __lhs.compare(__rhs) < 0;
    }

    template<class _CharT, class _Traits>
    bool operator<(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs, 
                    basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
    {
        return __lhs.compare(__rhs) < 0;
    }


    // operator >
    template<class _CharT, class _Traits>
    bool operator> (basic_string_view<_CharT, _Traits> __lhs, basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
    {
        return __lhs.compare(__rhs) > 0;
    }

    template<class _CharT, class _Traits>
    bool operator>(basic_string_view<_CharT, _Traits> __lhs,
                    typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
    {
        return __lhs.compare(__rhs) > 0;
    }

    template<class _CharT, class _Traits>
    bool operator>(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs, 
                    basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
    {
        return __lhs.compare(__rhs) > 0;
    }


    // operator <=
    template<class _CharT, class _Traits>
    bool operator<=(basic_string_view<_CharT, _Traits> __lhs, basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
    {
        return __lhs.compare(__rhs) <= 0;
    }

    template<class _CharT, class _Traits>
    bool operator<=(basic_string_view<_CharT, _Traits> __lhs,
                    typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
    {
        return __lhs.compare(__rhs) <= 0;
    }

    template<class _CharT, class _Traits>
    bool operator<=(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs, 
                    basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
    {
        return __lhs.compare(__rhs) <= 0;
    }


    // operator >=
    template<class _CharT, class _Traits>
    bool operator>=(basic_string_view<_CharT, _Traits> __lhs, basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
    {
        return __lhs.compare(__rhs) >= 0;
    }


    template<class _CharT, class _Traits>
    bool operator>=(basic_string_view<_CharT, _Traits> __lhs,
                    typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
    {
        return __lhs.compare(__rhs) >= 0;
    }

    template<class _CharT, class _Traits>
    bool operator>=(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs, 
                    basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
    {
        return __lhs.compare(__rhs) >= 0;
    }

    // [string.view.io]
    template<class _CharT, class _Traits>
	std::basic_ostream<_CharT, _Traits>&
    operator<<(std::basic_ostream<_CharT, _Traits>& __os, basic_string_view<_CharT, _Traits> __sv)
    {
        return __os.write (__sv.data(), __sv.size());
    }

  typedef basic_string_view<char>     string_view;
  typedef basic_string_view<char16_t> u16string_view;
  typedef basic_string_view<char32_t> u32string_view;
  typedef basic_string_view<wchar_t>  wstring_view;
}
  
namespace std
{
	// [string.view.hash]
	template<class _CharT, class _Traits>
	struct hash<experimental::basic_string_view<_CharT, _Traits> >
		: public unary_function < experimental::basic_string_view<_CharT, _Traits>, size_t >
	{
		size_t operator()(const experimental::basic_string_view<_CharT, _Traits>& __val) const _NOEXCEPT;
	};

	template<class _CharT, class _Traits>
	size_t hash<experimental::basic_string_view<_CharT, _Traits> >::operator()(const experimental::basic_string_view<_CharT, _Traits>& __val) const _NOEXCEPT
	{
		return std::hash<const _CharT*>()(__val.data());
	}

	template<class _CharT, class _Traits>
	std::string to_string(const experimental::basic_string_view<_CharT, _Traits>& __val)
	{
		return{ __val.data(), __val.size() };
	}
}