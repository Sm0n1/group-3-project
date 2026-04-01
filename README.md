
### Google Docs folder link 

https://drive.google.com/drive/folders/1sVGTf26g0Y38iN2YYh_g9bAhXBtHn969?usp=share_link

Includes report + presentation + meeting notes. 
Easier for everyone to access it with the link :) 

### C++ Coding Guidelines

The coding guidelines presented here are subject to change for the time being.

#### Naming Conventions

All names, except template parameters, use `snake_case`. Template parameter names use `PascalCase`. Variable names are never prefixed with `m_`, `g_`, etc, and never contain type information.

#### Namespaces

Do not circumvent namespaces with `using namespace`. Always wrap file contents in a namespace.

#### Header Files

All header files should contain a header guard of the following form:

```cpp
#ifndef CLAYBORNE_FILE_NAME_HPP
#define CLAYBORNE_FILE_NAME_HPP

...

#endif  // CLAYBORNE_FILE_NAME_HPP
```

#### Variable Initialization

Prefer brace-initialization over initializing with `=`.

```cpp
int x{ 0 };
```

#### Enums

```cpp
enum class color {
    red,
    green,
    blue,
    somewhat_yellow,
};
```

Note the use of an enum class and the trailing comma.

#### Branches and Loops

```cpp
if (should_return) {
    return;
}

for (int i = 0; i < n; i += 1) {
    sum += arr[i];
}
```

Always use curly braces for bodies, even if they only contain a single statement.

#### Functions

```cpp
constexpr void foo_bar(int arg1, int arg2) {
    return 0;
}
```

If possible, mark functions as `constexpr`.

#### Polymorhism

We will be using EnTT to handle most of the game state polymorphically. For this reason there is little reason for using the polymorphism features of C++. The following section covers the cases when EnTT is not applicable.

Always use templates and concepts over inheritence. Avoid explicit function/operator overloading.

#### Arrays

Prefer `std::array` for static sizes, `std::vector`  for dynamic sizes, and `std::span` for arguments if applicable.

#### Constants

Avoid macros if possible. Use `constexpr` instead.

#### Error Handling

Avoid exceptions if possible. Use `std::optional` and `std::expecting` instead. Assert liberally.

# Building

If you want to use a vendored SDL3, use the following command:
```sh
cmake .. -D CLAYBORNE_VENDORED=ON
```