
### Google Docs folder link 

https://drive.google.com/drive/folders/1sVGTf26g0Y38iN2YYh_g9bAhXBtHn969?usp=share_link

Includes report + presentation + meeting notes. 
Easier for everyone to access it with the link :) 


### C++ Coding Guidelines

The coding guidelines presented here are subject to change for the time being.

#### Naming Conventions

All names, except template parameters, use `snake_case`. Template parameter names use `UpperPascalCase`. Variable names are never prefixed with `m_`, `g_`, etc, and never contain type information.

#### Namespaces

Do not circumvent namespaces with `using namespace`.

#### Header Files

All header files should contain a header guard of the following form:

```cpp
#ifndef FOO_BAR_BAZ_H_
#define FOO_BAR_BAZ_H_

...

#endif  // FOO_BAR_BAZ_H_
```

#### Enums

```cpp
enum class Color {
    red,
    green,
    blue,
    somewhat_yellow,
};
```

Note the use of an enum class and the trailing comma.

#### Functions

```cpp
constexpr void foo_bar(int arg1, int arg2) {
    return 0;
}
```

#### Polymorhism

Always use templates and concepts over inheritence. Avoid explicit function/operator overloading. In general, avoid polymorphism.

#### C Arrays

Prefer std::Array for static sizes, std::Vector for dynamic sizes, and std::Span for arguments.

If possible, mark functions as `constexpr`.

#### Constants

Avoid macros if possible. Use `constexpr` instead.

#### Error Handling

Avoid exceptions if possible. Use `std::optional` and `std::expecting` instead. Assert liberally.
