# trait
Rust-like traits implementation in C++

Implementation that uses macros ```demo.cpp``` [https://godbolt.org](https://t.co/fCHMRVkvJN)


```c++
cpp2(
    trait Shape as (
        fn area as (() const -> f32),
        fn name as (() const -> std::string)
    )
);

struct Circle {
    f32 radius = {};

    auto area() const -> f32 { return f32(M_PI) * radius * radius; }
    auto name() const -> std::string { return "Circle"; }
};

stl::impl<Shape> auto circle = Circle{.radius = 10};
stl::dyn<Shape&> shape = &circle;
```

Implementation that uses deducing this and macros ```demo_deducing_this.cpp```
```c++
cpp2(
    trait Shape as (
        fn area as (() const -> f32),
        fn name as (() const -> std::string)
    )

    type Circle as (
        f32 radius = {};

        auto area(__this &self) -> f32 {
            return return f32(M_PI) * self.radius * self.radius;
        }
        auto name(__this &self) -> std::string {
            return "Circle";
        }
    )
);
stl::impl<Shape> auto circle = Circle{.radius = 10};
stl::dyn<Shape&> shape = &circle;
```

Implementation that uses p1240r1 ```demo_p1240r1.cpp``` [https://godbolt.org](https://godbolt.org/z/9Y3hxvnPa)
```c++
struct Shape {
    auto area() const -> f32;
    auto name() const -> std::string;
};

struct Circle {
    f32 radius = {};

    auto area() const -> f32 { return f32(M_PI) * radius * radius; }
    auto name() const -> std::string { return "Circle"; }
};

auto circle = Circle{.radius = 10};
stl::dyn<Shape> shape = circle;
```

[p1240r1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1240r1.pdf)
