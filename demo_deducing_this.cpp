#include <cmath>
#include <string>
#include <iostream>

#define PARENS ()
#define EXPAND(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) __VA_ARGS__

#define FOR_EACH(macro, ...) __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...) macro(a1) __VA_OPT__(FOR_EACH_AGAIN PARENS (macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define TRAIT_INTERFACE_POINTER_I(name, ...) stl::trait_function_pointer_t<auto __VA_ARGS__> name##_;
#define TRAIT_INTERFACE_POINTER(...) TRAIT_INTERFACE_POINTER_I __VA_ARGS__

#define TRAIT_IMPL_CONSTRAINT_I(name, ...) static_cast<stl::trait_member_pointer_t<Impl, auto __VA_ARGS__>>(&Impl::name);
#define TRAIT_IMPL_CONSTRAINT(...) TRAIT_IMPL_CONSTRAINT_I __VA_ARGS__

#define TRAIT_INTERFACE_THUNK_I(name, ...) (stl::trait_function_pointer_t<auto __VA_ARGS__>) static_cast<stl::trait_member_pointer_t<Impl, auto __VA_ARGS__>>(&Impl::name),
#define TRAIT_INTERFACE_THUNK(...) TRAIT_INTERFACE_THUNK_I __VA_ARGS__

#define TRAIT_METHOD_I(name, ...)                                                                    \
auto name(this Self const& self, auto&&... args) -> stl::trait_function_result_t<auto __VA_ARGS__> { \
    return self.vptr->name##_(self.impl, static_cast<decltype(args)&&>(args)...);                    \
}
#define TRAIT_METHOD(...) TRAIT_METHOD_I __VA_ARGS__

#define TRAIT(Trait, ...)                                      \
struct Trait {                                                 \
    struct vtable {                                            \
        size_t size;                                           \
        size_t align;                                          \
        FOR_EACH(TRAIT_INTERFACE_POINTER, __VA_ARGS__)         \
    };                                                         \
    template<typename Impl>                                    \
    static constexpr auto implements = requires {              \
        FOR_EACH(TRAIT_IMPL_CONSTRAINT, __VA_ARGS__)           \
    };                                                         \
    template<typename Impl>                                    \
    inline static const auto vtable_for = vtable{              \
        sizeof(Impl),                                          \
        alignof(Impl),                                         \
        FOR_EACH(TRAIT_INTERFACE_THUNK, __VA_ARGS__)           \
    };                                                         \
};                                                             \
template<>                                                     \
struct stl::dyn<Trait&> {                                      \
    using Self = stl::dyn<Trait&>;                             \
    using vtable = Trait::vtable;                              \
                                                               \
    void*         impl = {};                                   \
    vtable const* vptr = {};                                   \
                                                               \
    template<typename Impl>                                    \
    constexpr dyn(Impl* impl) noexcept                         \
    : impl(impl)                                               \
    , vptr(&Trait::template vtable_for<Impl>) {}               \
                                                               \
    FOR_EACH(TRAIT_METHOD, __VA_ARGS__)                        \
};                                                             \
template<>                                                     \
struct stl::dyn<Trait const&> {                                \
    using Self = stl::dyn<Trait&>;                             \
    using vtable = Trait::vtable;                              \
                                                               \
    const void*   impl = {};                                   \
    vtable const* vptr = {};                                   \
                                                               \
    template<typename Impl>                                    \
    constexpr dyn(Impl const* impl) noexcept                   \
    : impl(impl)                                               \
    , vptr(&Trait::template vtable_for<Impl>) {}               \
                                                               \
    FOR_EACH(TRAIT_METHOD, __VA_ARGS__)                        \
};

namespace stl {
    template<typename Fn>
    struct trait_function_pointer;

    template<typename R, typename... Args>
    struct trait_function_pointer<R(Args...)> {
        using type = R(*)(void*, Args...);
    };

    template<typename R, typename... Args>
    struct trait_function_pointer<R(Args...) const> {
        using type = R(*)(const void*, Args...);
    };

    template<typename Fn>
    using trait_function_pointer_t = trait_function_pointer<Fn>::type;

    template<typename Self, typename Fn>
    struct trait_member_pointer;

    template<typename Self, typename R, typename... Args>
    struct trait_member_pointer<Self, R(Args...)> {
        using type = R(*)(Self&, Args...);
    };

    template<typename Self, typename R, typename... Args>
    struct trait_member_pointer<Self, R(Args...) const> {
        using type = R(*)(Self const&, Args...);
    };

    template<typename Self, typename Fn>
    using trait_member_pointer_t = trait_member_pointer<Self, Fn>::type;

    template<typename R, typename... Args>
    struct trait_function_result;

    template<typename R, typename... Args>
    struct trait_function_result<R(Args...)> {
        using type = R;
    };

    template<typename R, typename... Args>
    struct trait_function_result<R(Args...) const> {
        using type = R;
    };

    template<typename Fn>
    using trait_function_result_t = trait_function_result<Fn>::type;

    template <typename> struct dyn;
    template <typename Self, typename Trait>
    concept impl = Trait::template implements<Self>;
}

using i32 = int;
using f32 = float;

#define LPAREN (
#define RPAREN )
#define trait TRAIT LPAREN
#define fn LPAREN
#define AS_UNPACK(...) __VA_ARGS__ RPAREN
#define as , AS_UNPACK
#define cpp2(...) __VA_ARGS__

#define TYPE_UNPACK(...) __VA_ARGS__
#define TYPE(name, ...) struct name { using Self = name; __VA_ARGS__ };
#define type TYPE LPAREN

#define __this this Self const
#define __this_mut this Self

cpp2(
    // todo: reflection?

    trait Shape as (
        fn area as (() const -> f32),
        fn name as (() const -> std::string)
    )

    trait Debug as (
        fn name as (() const -> std::string)
    )

    type Circle as (
        f32 radius = {};

        auto area(__this &self) -> f32 {
            return f32(M_PI) * self.radius * self.radius;
        }
        auto name(__this &self) -> std::string {
            return "Circle";
        }
    )

    type Square as (
        f32 length = {};
        f32 height = {};

        auto area(__this &self) -> f32 {
            return self.length * self.height;
        }

        auto name(__this &self) -> std::string {
            return "Square";
        }
    )

    static_assert(Shape::implements<Circle>);
    static_assert(Shape::implements<Square>);
    static_assert(stl::impl<Circle, Shape>);
    static_assert(stl::impl<Square, Shape>);
)

void test_impl(stl::impl<Shape> auto const& shape) {
    std::cout << "test_impl: " << shape.name() << "'s area is " << shape.area() << std::endl;
}

void test_dyn(stl::dyn<Shape&> shape) {
    std::cout
        << "test_dyn: "
        << shape.name()
        << "'s area is "
        << shape.area() << std::endl;
}

#define impl_for(Trait, Type) template<> Trait::vtable Trait::vtable_for<Type>

//template<>
//struct Shape::__impl<std::vector<int>> : std::vector<int> {
//    auto area() const -> f32 {
//        return 0;
//    }
//    auto name() const -> std::string {
//        return "std::vector<int>";
//    }
//};

//impl_for(Shape, std::vector<int>) {
//    .size = sizeof(std::vector<int>),
//    .align = alignof(std::vector<int>),
//    .area_ = [](const void* __this) -> f32 {
//        struct __impl : std::vector<int> {
//
//        };
//        return 0;
//    },
//    .name_ = [](const void* __this) -> std::string {
//        return "std::vector<int>";
//    }
//};

auto main() -> i32 {
    // stl::impl is concept
    stl::impl<Shape> auto circle = Circle(2.0F);
    stl::impl<Shape> auto square = Square(10.0F, 20.0F);

    // stl::dyn uses type-erasure and stores reference to object
    // When 'circle' goes out of scope, 'shape' will point to dangling pointer
    stl::dyn<Shape&> shape = &circle;

    // stl::dyn satisfies stl::impl concept, so why not?
//    test_impl(shape);

    // stl::dyn can be reassigned
    shape = &square;
    test_impl(shape);
    test_impl(circle);

    // implicit cast to stl::dyn
    test_dyn(&circle);
    test_dyn(&square);


//    std::vector<int> points;
//    test_dyn(&points);

    return 0;
}
