# C++ Code Style Standards

### What type of indention do we use?

We use 4 spaces for code indention.

---

### How do we place braces?

We put braces on separate lines, like this:

```cpp
void foo(int a, int b) const
{
    /* ... */
}
```

---

### What naming convention do we use?

* For local variables we use: `snake_case`.
* For constants we use: `UPPER_SNAKE`.
* For classes we use: `PascalCase`.
* For class fields we use: `snake_case`.
* For functions and methods we use: `snake_case`.

---

### Do we put a space before conditional statements?

Yes, we do it like this: `if ()`.

---

### How to we write expressions and functions parameters?

We put spaces between operators and no spaces inside the brackets, like this: `(x + (y * z))` or `foo(a, b, c)`.

---

### Can we declare more than one variable in one line?

Usually no, but if there's good reason for doing so, it's ok.

---

### Where do we put the asterisk/ampersand when declaring the pointer/reference?

Next to the name of the type, like this:

```cpp
int* foo;
std::string& bar;
```

---

### What file extensions do we use?

We use `.cpp` for source files, `.hpp` for header files and `.tpp` for templated headers.

---

### Where do we put templates?

We keep template code in `.tpp` files, which names are the same as the class they are defining.

---

### How do we write oneline conditional statements?

We don't.

---

### How should the files structure look like?

It should look like this:

```
├───example
│   └─── Source code of the game
├───core
│   └─── Source code of the engine
└───thirdparty
    └─── Source code of thirdparty libraries

```

---

### What include guards do we use?

We use `#pragma once`.

---

### Do we use forward declarations?

Yes, if you can use forward declarations instead of including a header, do so.

---

### What is the order of includes?

1. Corresponding header file (if in `.cpp` file)
2. C++ system files
3. Other libraries `.hpp` files
4. Engine `.hpp` files
5. Forward declarations

Here's the example:

```cpp
#include "corresponding_header.hpp"

#include <iostream>
#include <ctime>
#include <string>

#include <GL/glew.h>

#include "another_project_header.hpp"

class GLFWWindow;
```

---

### What about namespaces?

Namespaces subdivide the global scope into distinct, named scopes, and so are useful for preventing name collisions in the global scope.

We use `Aporia` namespace for our engine.

When we want to use more than one namespace, we do it like this:

```cpp
namespace Aporia::Editor
{
    /* ... */
}
```

---

### What about `using` declaration?

We do **not** use `using namespace` nor e.g. `using std::string` in the global scope, **never**! If we need to, we can use it in other scopes e.g. in a class or a function.

---

### How do we describe functions and it's parameters?

We do it in header files using Doxygen notation, like this:

```cpp
/**
 *  Does foo
 *
 *  @param angle ...
 *  @param index ...
 *
 *  @return Returns foo
 */
void Foo(float angle, int index);
```

---

### What about colons in classes and constructors?

Put spaces before and after a colon, like this:

```cpp
class DerivedClass : public BaseClass
{
public:
    DerivedClass(int x)
        : _x(x) {}

private:
    int _x;
};
```

---

### What is the order of methods and fields?

- static constexpr variables
- using declarations

* friendship declarations

- public methods
- public fields

* protected methods
* protected fields

- private methods
- private fields

---

### Is there any convention of naming private and protected fields/methods?

Yes, there is. Add a *underscore* in front of the name of the private and protected field/method.

```cpp
protected:
    int _protected_bar();

private:
    int _private_foo;
```

---

### How do we format constructor initializer lists?

We put a colon in the new line after the indentation, like this:

```cpp
Foo(int x, int y, std::string name)
    : _x(x), _y(y), _name( std::move(name) )
{
    /* ... */
}
```

---


## Tips

### Use `final` specifier with classes that should not be inherited

```cpp
class DerivedClass final : public BaseClass {/* ... */}
```

---

### Use `override` specifier with methods that are inherited.

```cpp
struct BaseClass
{
    virtual void foo();
    /* ... */
}

struct DerivedClass : public BaseClass
{
    void foo() override;
    /* ... */
}
```

---

### Use `const` keyword with functions and methods that do not change the value of attributes

```cpp
Vector2<int> get_window_position() const;
```

---

### Use `const T&` if the passed objects are read-only and to avoid copying it

```cpp
template<typename T>
T distance_to(const Vector2<T>& pos);
```

---

### Use `enum class` instead of `enum` to scope it's values

```cpp
enum class Imagetype : int
{
    Standard = 0,
    Nice = 1,
    NotNice = -1
};
/* ... */
Imagetype foo;
/* ... */
foo = Imagetype::Nice;
/* ... */
auto bar = static_cast<std::underlying_type_t<Imagetype>>(foo);
```

---

### Don't define a default constructor that only initializes data members; use in-class member initializers instead

#### Reason

Using in-class member initializers lets the compiler generate the function for you. The compiler-generated function can be more efficient.

#### Example, bad

```cpp
class Foo
{
public:
    X1()
        :s{"default"}, i{1} {}
    /* ... */
private:
    /* BAD: doesn't use member initializers */
    std::string s;
    int i;
};
```

#### Example

```cpp
class Bar
{
public:
    /* use compiler-generated default constructor */
    /* ... */
private:
    std::string s = "default";
    int i = 1;
};
```

#### Enforcement

(Simple) A default constructor should do more than just initialize member variables with constants.

##### Source: [C++ Core Guidelines – C.45](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Rc-default)

---

### Define and initialize member variables in the order of member declaration

#### Reason

To minimize confusion and errors. That is the order in which the initialization happens (independent of the order of member initializers).

#### Example, bad

```cpp
class Foo
{
public:
    Foo(int x)
        : m2{x}, m1{++x} {}   /* BAD: misleading initializer order */
    /* ... */
private:
    int m1;
    int m2;
};

Foo x(1); /* surprise: x.m1 == x.m2 == 2 */
```

#### Enforcement

(Simple) A member initializer list should mention the members in the same order they are declared.

##### Source: [C++ Core Guidelines – C.47](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Rc-order)

---

### Where there is a choice, prefer default arguments over overloading

#### Reason

Default arguments simply provide alternative interfaces to a single implementation. There is no guarantee that a set of overloaded functions all implement the same semantics. The use of default arguments can avoid code replication.

#### Note

There is a choice between using default argument and overloading when the alternatives are from a set of arguments of the same types. For example:

```cpp
void print(const std::string& s, format f = {});
```

as opposed to

```cpp
void print(const std::string& s);  /* use default format */
void print(const std::string& s, format f);
```

There is not a choice when a set of functions are used to do a semantically equivalent operation to a set of types. For example:

```cpp
void print(const char&);
void print(int);
```

##### Source: [C++ Core Guidelines – F.51](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Rf-default-args)

---

### Keep the number of function arguments low

#### Reason

Having many arguments opens opportunities for confusion. Passing lots of arguments is often costly compared to alternatives.

#### Discussion

The two most common reasons why functions have too many parameters are:

 * Missing an *abstraction*. There is an abstraction missing, so that a compound value is being
 passed as individual elements instead of as a single object that enforces an invariant. This not only expands the parameter list, but it leads to errors because the component values are no longer protected by an enforced invariant.

 * Violating *"one function, one responsibility."* The function is trying to do more than one job and should probably be refactored.

##### Source: [C++ Core Guidelines – I.23](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Ri-nargs)

---

### Never transfer ownership by a raw pointer (T*) or reference (T&)

#### Reason

If there is any doubt whether the caller or the callee owns an object, leaks or premature destruction will occur.

#### Example

Consider:

```cpp
X* compute(args)    /* don't */
{
    X* res = new X{};
    /* ... */
    return res;
}
```

Who deletes the returned X? The problem would be harder to spot if compute returned a reference. Consider returning the result by value (use move semantics if the result is large):

```cpp
std::vector<double> compute(args)  /* good */
{
    std::vector<double> res(10000);
    /* ... */
    return res;
}
```

**Alternative:** Pass ownership using a *"smart pointer"*, such as *unique_ptr* (for exclusive ownership) and *shared_ptr* (for shared ownership). However, that is less elegant and often less efficient than returning the object itself, so use smart pointers only if reference semantics are needed.

##### Source: [C++ Core Guidelines – I.11](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Ri-raw)
