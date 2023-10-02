# trait
Rust-like traits implementation in C++

Implementation that uses macros```demo.cpp``` [https://godbolt.org](https://godbolt.org/z/Psnrqc3Y3)
```c++
trait(Shape,
    (area, auto() const -> f32),
    (name, auto() const -> std::string)
);

struct Circle {
    f32 radius = {};

    auto area() const -> f32 { return f32(M_PI) * radius * radius; }
    auto name() const -> std::string { return "Circle"; }
};

auto circle = Circle{.radius = 10};
stl::dyn<Shape> shape = circle;
```

Implementation that uses p1240r1 ```demo_p1240r1.cpp``` [https://godbolt.org](https://godbolt.org/z/9Y3hxvnPa)
```c++
struct Shape {
    auto area() const -> f32;
    auto name() const -> std::string;
};

struct Circle {
    f32 radius = {};

    auto area() const -> f32' { return f32(M_PI) * radius * radius; }
    auto name() const -> std::string { return "Circle"; }
};

auto circle = Circle{.radius = 10};
stl::dyn<Shape> shape = circle;
```

[p1240r1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1240r1.pdf)
