/*
Copyright (c) 2017 Andrey Zhukov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
//Now hosted on github!!!

#ifndef PRETTY_INDEX_HPP
#define PRETTY_INDEX_HPP

//==============================================================================
//IMPORTANT MACROS =============================================================

/*!
\brief   If set to 1, removes spaces between angle brackets '''<>'''
\details Example of what this macro does:\n
         If macro **is** set to 1, '''prettyid(std::string).name()''' returns
		 '''std::basic_string<char, std::char_traits<char>, 
		 std::allocator<char>>'''
		 If macro **is not** set to 1, '''prettyid(std::string).name()'''
		 returns '''std::basic_string<char, std::char_traits<char>, 
		 std::allocator<char> >'''
\see     pretty_index::name
*/
#define PRETTY_INDEX_GROUP_ANGLE_BRACKETS 1

/*!
\brief   If set to 1, cuts non-standard inline namespaces in llvm
\details Example of what this macro does:\n 
         If macro **is** set to 1, '''prettyid(std::string).name()''' returns
		 '''std::basic_string<char, std::char_traits<char>, 
		 std::allocator<char> >'''
		 If macro **is not** set to 1, '''prettyid(std::string).name()''' 
		 returns '''std::__1::basic_string<char, std::char_traits<char>, 
		 std::allocator<char> >'''
\see     pretty_index::name
*/
#define PRETTY_INDEX_LIBCPP_CUT_INLINE_NAMESPACES 1

/*!
\brief   If set to 1, unwraps typedef classes to their original types in gcc
\details Example of what this macro does:\n
         If macro **is** set to 1, '''prettyid(std::string).name()''' returns
         '''std::basic_string<char, std::char_traits<char>, 
		 std::allocator<char> >'''
         If macro **is not** set to 1, '''prettyid(std::string).name()'''
         returns '''std::string'''
\see     pretty_index::name
*/
#define PRETTY_INDEX_LIBSTDCPP_UNWRAP_TYPEDEFS 1

/*!
\brief   If set to 1, enables a macro called "prettyid", which behaves like
         the typeid keyword
\details prettyid macro is using variadic macro arguments. If your compiler
         does not support them, prettyid will not work with template classes.
\see     prettyid
*/
#define PRETTY_INDEX_ENABLE_PRETTYID_MACRO 1

//==============================================================================

#include <cstdlib> // std::realloc, std::free
#include <cstring> // strlen, strstr, strchr, std::memmove, strncpy_s
#include <typeinfo> // std::type_info
#include <typeindex> // std::type_index

#include <ciso646>
#ifdef _LIBCPP_VERSION
#	include <cxxabi.h> // abi::__cxa_demangle
//	Using LLVM libc++
#elif __GLIBCXX__ // Note: only version 6.1 or newer define this in ciso646
#	include <cxxabi.h> // abi::__cxa_demangle
//	Using GNU libstdc++
#elif _CPPLIB_VER // Note: used by Visual Studio
//	Using Dinkumware STL
#else
#	error pretty_index: unsupported standerd library detected. Supported libraries are: LLVM libc++, GNU libstdc++, Dinkumware STL
#endif

///Namespace for all functions and classes, to not pollute global namespace
namespace zhukov {

///Namespace for internal-use-only functions
namespace details {

char * str_shrink_to_fit(char*& str) {
	str = static_cast<char*>(realloc(str, strlen(str) + 1));
	return str;
}

char * str_cut(char*& str, const char* string_to_cut) {
	std::size_t string_to_cut_len = strlen(string_to_cut);

	char* it;
	while ((it = strstr(str, string_to_cut)) != nullptr) {
		std::memmove(it, it + string_to_cut_len, strlen(it + string_to_cut_len) + 1);
	}

	str = static_cast<char*>(std::realloc(str, strlen(str)+1));
	return str;
}

//Disable C4996 for strspy, strncpy
#if defined (_MSC_VER)
#pragma warning(disable:4996)
#endif

char * str_dup(const char * str) {
	std::size_t len = 1 + strlen(str);
	char* p = static_cast<char*>(malloc(len));

	return p ? static_cast<char*>(memcpy(p, str, len)) : nullptr;
}

char* str_rep(char*& str, const char* original, const char* updated) {
	std::size_t original_len = strlen(original);
	std::size_t updated_len = strlen(updated);
	
	//How much memory is needed for the new std::string
	std::size_t mem_needed = strlen(str) + 1;

	//By how much does the array need to grow/shrink
	int diff = updated_len - original_len;
	
	//Calculate required memory
	for (char* it = strstr(str, original); it != nullptr; it = strstr(++it, original)) mem_needed += diff;
	
	char* it;
	if (diff >= 0) {
		//If std::string grows, reallocate memoty at the beginning
		if (!(str = static_cast<char*>(std::realloc(str, mem_needed)))) return nullptr;

		it = str;
		while ((it = strstr(it, original)) != nullptr) {
			//Free space for new std::string
			std::memmove(it + updated_len, it + original_len, strlen(it + original_len) + 1);
			//Insert new std::string
			strncpy(it, updated, updated_len);

			it += updated_len;
		}
	}
	else {
		//If std::string shrinks, reallocate memoty at the end
		it = str;
		while ((it = strstr(it, original)) != nullptr) {
			//Free space for new std::string
			std::memmove(it + updated_len, it + original_len, strlen(it + original_len) + 1);
			//Insert new std::string
			strncpy(it, updated, updated_len);

			it += updated_len;
		}

		str = static_cast<char*>(std::realloc(str, mem_needed));
	}
	
	return str;

}

//Disable C4996 for strncpy
#if defined (_MSC_VER)
#pragma warning(default:4996)
#endif

char* demangle(const char* name) {
	#if defined (_CPPLIB_VER)

	//Normal version
	char* str = str_dup(name);
	if (!str_cut(str, "class ")) throw std::bad_cast();
	if (!str_cut(str, "struct ")) throw std::bad_cast();
	if (!str_rep(str, " ,", ",")) throw std::bad_cast();
	if (!str_rep(str, ",", ", ")) throw std::bad_cast();
	
	//Optimised-by-hand version
	/*std::size_t mem_needed = strlen(name) + 1;
	for (const char* it = strchr(name, ','); it != nullptr; it = strchr(++it, ',')) mem_needed++;
	char* str = (char*)std::malloc(mem_needed);
	std::memcpy(str, name, mem_needed);

	const char substr_1[] = "class ";
	const char substr_2[] = "struct ";
	char* it;

	//Length of "struct " = 6
	while ((it = strstr(str, substr_1)) != nullptr) {
		memmove(it, it + 6, strlen(it + 6) + 1);
	}

	//Length of "struct " = 7
	while ((it = strstr(str, substr_2)) != nullptr) {
		memmove(it, it + 7, strlen(it + 7) + 1);
	}
	
	for (it = strchr(str, ','); it != nullptr; it = strchr(++it, ',')) {
		if (*(it + 1) != ' ') {
			memmove(it + 2, it + 1, strlen(it + 1) + 1);
			*(it + 1) = ' ';
		}
		if (*(it - 1) == ' ') {
			memmove(it - 1, it, strlen(it) + 1);
		}
	}*/

	#elif defined(_LIBCPP_VERSION)
	
	int status = 0;
	const char* demangled_name = abi::__cxa_demangle(name, nullptr, nullptr, &status);
	if (status != 0) {
		std::free(const_cast<char*>(demangled_name));
		throw std::runtime_error((std::string(name) + " - demangling failed").c_str());
	}
	char* str = str_dup(demangled_name);
	std::free(const_cast<char*>(demangled_name));

	#if PRETTY_INDEX_LIBCPP_CUT_INLINE_NAMESPACES == 1

	const char substr[] = "std::__";
	const char colons[] = "::";
	char* it;

	//Length of "std::__" = 7
	//Double colons "::" begin at pos 3
	while ((it = strstr(str, substr)) != nullptr) {
		memmove(it + 3, strstr(it+7, colons), strlen(strstr(it + 7, colons)) + 1);
	}
	str_shrink_to_fit(str);

	#endif // PRETTY_INDEX_LIBCPP_CUT_INLINE_NAMESPACES == 1
	#elif defined (__GLIBCXX__)

	int status = 0;
	const char* demangled_name = abi::__cxa_demangle(name, nullptr, nullptr, &status);
	if (status != 0) {
		std::free(const_cast<char*>(demangled_name));
		throw std::runtime_error((std::string(name) + " - demangling failed").c_str());
	}
	char* str = str_dup(demangled_name);
	std::free(const_cast<char*>(demangled_name));

	#if PRETTY_INDEX_LIBSTDCPP_UNWRAP_TYPEDEFS == 1
	if (!str_rep(str, "std::string", "std::basic_std::string<char, std::char_traits<char>, std::allocator<char> >")) throw std::bad_cast();
	#endif // PRETTY_INDEX_LIBSTDCPP_UNWRAP_TYPEDEFS == 1
	
	#endif

	#if PRETTY_INDEX_GROUP_ANGLE_BRACKETS == 1

	if (!str_rep(str, " >", ">")) throw std::bad_cast();

	#endif // PRETTY_INDEX_GROUP_ANGLE_BRACKETS == 1

	return str;
}

/*!
\brief   MurmurHashNeutral2, by Austin Appleby
\details Produces a hash of a byte array. Same as MurmurHash2, but endian- and alignment-neutral.
         Site: https://sites.google.com/site/murmurhash/
\param   key Poointer to an array of bytes
\param   len Length of the array
\param   seed Seed for hashing algorithm
\see     pretty_index::hash_code
*/
unsigned int MurmurHashNeutral2(const void * key, int len, unsigned int seed) {
	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	unsigned int h = seed ^ len;

	const unsigned char * data = (const unsigned char *)key;

	while (len >= 4) {
		unsigned int k;

		k = data[0];
		k |= data[1] << 8;
		k |= data[2] << 16;
		k |= data[3] << 24;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	switch (len) {
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
		h *= m;
	};

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

} // namespace details

class pretty_index {
private:
	char* data;

public:
	const char* name() const;
	std::size_t hash_code() const;

	bool operator==(const pretty_index& rhs) const;
	bool operator!=(const pretty_index& rhs) const;
	bool operator< (const pretty_index& rhs) const;
	bool operator<=(const pretty_index& rhs) const;
	bool operator> (const pretty_index& rhs) const;
	bool operator>=(const pretty_index& rhs) const;

	pretty_index(const std::type_info& info); // Construct from std::type_info
	pretty_index(const std::type_index& info); // Construct from std::type_index

	pretty_index() = delete; // Default constructor (deleted)
	pretty_index(const pretty_index& other) = default; // Copy constructor
	pretty_index(pretty_index&& other) = default; // Move constructor
	~pretty_index(); // Default destructor
};

inline const char* pretty_index::name() const {
	return data;
}

inline std::size_t pretty_index::hash_code() const {
	return details::MurmurHashNeutral2(data, static_cast<int>(strlen(data)), 0);
}

inline bool pretty_index::operator==(const pretty_index & rhs) const {
	return strcmp(data, rhs.data) == 0;
}

inline bool pretty_index::operator!=(const pretty_index & rhs) const {
	return !operator==(rhs);
}

inline bool pretty_index::operator<(const pretty_index & rhs) const {
	return strcmp(data, rhs.data) < 0;
}

inline bool pretty_index::operator<=(const pretty_index & rhs) const {
	return !rhs.operator<(*this);
}

inline bool pretty_index::operator>(const pretty_index & rhs) const {
	return rhs.operator<(*this);
}

inline bool pretty_index::operator>=(const pretty_index & rhs) const {
	return !operator<(rhs);
}

inline pretty_index::pretty_index(const std::type_info & info) :
	data (details::demangle(info.name()))
{}

inline pretty_index::pretty_index(const std::type_index & info) :
	data(details::demangle(info.name()))
{}

inline pretty_index::~pretty_index() {
	std::free(data);
}

} // namespace zhukov

namespace std {

template<> struct hash<zhukov::pretty_index> {
	std::size_t operator()(const zhukov::pretty_index& obj) const {
		return obj.hash_code();
	}
};

} // namespace std

//Note: your compiler must support variadic templates for this macro to work.
#if PRETTY_INDEX_ENABLE_PRETTYID_MACRO == 1
#	define prettyid(...) zhukov::pretty_index(typeid(__VA_ARGS__))
#endif // PRETTY_INDEX_ENABLE_PRETTYID_MACRO == 1

#endif // PRETTY_INDEX_HPP