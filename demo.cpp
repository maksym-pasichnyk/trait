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

#define TRAIT_INTERFACE_FUNCTION_I(name, ...)                                                   \
template<typename Impl>                                                                         \
static auto name(auto __this, auto... args) -> stl::trait_function_result_t<auto __VA_ARGS__> { \
    if constexpr (std::is_const_v<std::remove_pointer_t<decltype(__this)>>) {                   \
        return static_cast<const Impl*>(__this)->name(static_cast<decltype(args)&&>(args)...);  \
    } else {                                                                                    \
        return static_cast<Impl*>(__this)->name(static_cast<decltype(args)&&>(args)...);        \
    }                                                                                           \
}

#define TRAIT_INTERFACE_FUNCTION(...) TRAIT_INTERFACE_FUNCTION_I __VA_ARGS__

#define TRAIT_IMPL_CONSTRAINT_I(name, ...) static_cast<stl::trait_member_pointer_t<Impl, auto __VA_ARGS__>>(&Impl::name);
#define TRAIT_IMPL_CONSTRAINT(...) TRAIT_IMPL_CONSTRAINT_I __VA_ARGS__

#define TRAIT_INTERFACE_THUNK_I(name, ...) &functions::template name<Impl>,
#define TRAIT_INTERFACE_THUNK(...) TRAIT_INTERFACE_THUNK_I __VA_ARGS__

#define TRAIT_METHOD_I(name, ...)                                                   \
auto name(auto&&... args) const -> stl::trait_function_result_t<auto __VA_ARGS__> { \
    return vptr->name##_(impl, static_cast<decltype(args)&&>(args)...);             \
}
#define TRAIT_METHOD(...) TRAIT_METHOD_I __VA_ARGS__

#define TRAIT(Trait, ...)                                      \
struct Trait {                                                 \
    struct functions {                                         \
        FOR_EACH(TRAIT_INTERFACE_FUNCTION, __VA_ARGS__)        \
    };                                                         \
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
    static constexpr auto vtable_for = vtable{                 \
        sizeof(Impl),                                          \
        alignof(Impl),                                         \
        FOR_EACH(TRAIT_INTERFACE_THUNK, __VA_ARGS__)           \
    };                                                         \
};                                                             \
template<>                                                     \
struct stl::dyn<Trait&> {                                      \
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
    constexpr dyn(dyn& other) = default;                       \
    constexpr dyn(dyn const& other) = default;                 \
    constexpr dyn(dyn&& other) noexcept = default;             \
                                                               \
    FOR_EACH(TRAIT_METHOD, __VA_ARGS__)                        \
};                                                             \
template<>                                                     \
struct stl::dyn<Trait const&> {                                \
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
    constexpr dyn(dyn& other) = default;                       \
    constexpr dyn(dyn const& other) = default;                 \
    constexpr dyn(dyn&& other) noexcept = default;             \
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
        using type = R(Self::*)(Args...);
    };

    template<typename Self, typename R, typename... Args>
    struct trait_member_pointer<Self, R(Args...) const> {
        using type = R(Self::*)(Args...) const;
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

cpp2(
    // cpp2 macro provides custom syntax for declaring traits!

    trait Shape as (
        fn area as (() const -> f32),
        fn name as (() const -> std::string)
    )

    trait Debug as (
        fn name as (() const -> std::string)
    )
);

struct Circle {
    f32 radius = {};

    auto area() const -> f32 { return f32(M_PI) * radius * radius; }
    auto name() const -> std::string { return "Circle"; }
};

struct Square {
    f32 length = {};
    f32 height = {};

    auto area() const -> f32 {
        return length * height;
    }
    auto name() const -> std::string {
        return "Square";
    }
};

void test_impl(stl::impl<Shape> auto const& shape) {
    std::cout << "test_impl: " << shape.name() << "'s area is " << shape.area() << std::endl;
}

void test_dyn(stl::dyn<Shape&> shape) {
    std::cout << "test_dyn: " << shape.name() << "'s area is " << shape.area() << std::endl;
}

auto main() -> i32 {
    // stl::impl is concept
    stl::impl<Shape> auto circle = Circle(2.0F);
    stl::impl<Shape> auto square = Square(10.0F, 20.0F);

    // stl::dyn uses type-erasure and stores reference to object
    // When 'circle' goes out of scope, 'shape' will point to dangling pointer
    stl::dyn<Shape&> shape = &circle;

    // stl::dyn satisfies stl::impl concept, so why not?
    test_impl(shape);

    // stl::dyn can be reassigned
//    shape = &square;
//    test_dyn(shape);

    test_impl(circle);
    test_impl(square);

    // implicit cast to stl::dyn
    test_dyn(&circle);
    test_dyn(&square);

    return 0;
}
