# prindex
A simple, lightweight wrapper around C++'s `type_info` for consistent, cross-compiler usage.

Get the same `name()` and `hash_code()` on every (supported) compiler and standard library.
## Installation
Prindex is a single-header library. You can download the latest release [here](https://github.com/andreasxp/prindex/releases).
Using prindex requires support of C++11.
## Usage
Prindex was designed to be as similar in usage as `type_info` and `type_index`. Here is a table of different uses and their alternatives in prindex:

| C++ Standard                                    | prindex                                         |
| ----------------------------------------------- | ----------------------------------------------- |
| `typeid(type)`                                  | `zh::prid<type>() or PRID(type)`                |
| `typeid(expr)`                                  | `zh::prid(expr) or PRID(expr)`                  |
| `const std::type_info& ti = typeid(...)`        | `const zh::prinfo& ti = PRID(...)`              |
| `std::type_index(typeid(...))`                  | `zh::prindex(zh::prid(...))` or `PRIDX(...)`    |

If you would like to take advantage of the *true keyword experience*, prindex includes helpful macros, so you can use `PRID(type)` instead of `zh::prid<type>()` (same for `pridx`).

All interfaces of included classes are identical to the corresponding ones in the C++ Standard.
## Documentation
All classes and functions described are in `namespace zh`.
### `class prinfo`
The class `prinfo` holds implementation-independent information about a type, including the name of the type and means to compare two types for equality or collating order. This is the class returned by the `prid` operator.

The `prinfo` class is neither CopyConstructible nor CopyAssignable.
#### Member functions
* (constructor) [deleted]  
has neither default nor copy constructors
* (destructor) [virtual]  
derived objects are safe to delete through pointer-to-base 
* `operator=` [deleted]  
can not be copy-assigned 
* `operator==`
* `operator!=`  
checks whether the objects refer to the same type 
* `before`  
checks whether the referred type precedes referred type of another `prinfo` object in the implementation defined order, i.e. orders the referred types 
* `hash_code`  
returns a value which is identical for the same types 
* `name`  
implementation-independent name of the type

### `class prindex`
The `prindex` class is a wrapper class around a `prinfo` object, that can be used as index in associative and unordered associative containers. The relationship with `prinfo` object is maintained through a pointer, therefore `prindex` is CopyConstructible and CopyAssignable.

#### Member functions
* (constructor)  
constructs the object  
* (destructor)  
destroys the prindex object
* `operator=`  
assigns a prindex object
* `operator==`
* `operator!=`
* `operator<`
* `operator<=`
* `operator>`
* `operator>=`  
compares the underlying `prinfo` objects
* `hash_code`  
returns hashed code 
* `name`  
returns implementation-independent name of the type,
associated with underlying `prinfo` object 

#### Helper classes
`std::hash<prindex>`  
hash support for `prindex`

### Function/macro `prid`
```C++
template <class T>
inline const prinfo& prid();

template <class T>
inline const prinfo& prid(T&& obj);
```
```C++
#define PRID(...) \
zh::detail::get_prinfo(typeid(__VA_ARGS__))
```
The `prid` function returns a const reference to a `prid` object, holding implementation-independent information about a type or expression passed into it. `PRID` is a macro that does the same thing.

### Function/macro `pridx`
```C++
template <class T>
inline prindex pridx();

template <class T>
inline prindex pridx(T&& obj);
```
```C++
#define PRIDX(...) \
zh::prindex(zh::detail::get_prinfo(typeid(__VA_ARGS__)))
```
The `pridx` function returns a `prindex` object, holding implementation-independent information about a type or expression passed into it. `PRIDX` is a macro that does the same thing.

## License
This project is licenced under the MIT licence. It is free for personal and commercial use.
