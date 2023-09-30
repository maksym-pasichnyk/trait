#include <iostream>
#include <string>
#include <cmath>

#define PARENS ()
#define EXPAND(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) __VA_ARGS__

#define FOR_EACH(macro, ...) __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...) macro(a1) __VA_OPT__(FOR_EACH_AGAIN PARENS (macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define TRAIT_INTERFACE_POINTER_I(name, ...) stl::trait_function_pointer_t<__VA_ARGS__> name;
#define TRAIT_INTERFACE_POINTER(...) TRAIT_INTERFACE_POINTER_I __VA_ARGS__

#define TRAIT_INTERFACE_FUNCTION_I(name, ...)                                                                       \
template<typename Self>                                                                                             \
static auto name##_interface_function(void* self_ptr, auto... args) -> stl::trait_function_result_t<__VA_ARGS__> {  \
    return static_cast<Self*>(self_ptr)->name(static_cast<decltype(args)&&>(args)...);                              \
}

#define TRAIT_INTERFACE_FUNCTION(...) TRAIT_INTERFACE_FUNCTION_I __VA_ARGS__

#define TRAIT_IMPL_CONSTRAINT_I(name, ...) static_cast<stl::trait_member_pointer_t<Self, __VA_ARGS__>>(&Self::name);
#define TRAIT_IMPL_CONSTRAINT(...) TRAIT_IMPL_CONSTRAINT_I __VA_ARGS__

#define TRAIT_INTERFACE_THUNK_I(name, ...) .name = &name##_interface_function<Self>,
#define TRAIT_INTERFACE_THUNK(...) TRAIT_INTERFACE_THUNK_I __VA_ARGS__

#define TRAIT_METHOD_I(name, ...)                                           \
auto name(auto&&... args) const -> trait_function_result_t<__VA_ARGS__> {   \
    return vtable->name(object, std::forward<decltype(args)>(args)...);     \
}
#define TRAIT_METHOD(...) TRAIT_METHOD_I __VA_ARGS__

#define trait(name, ...)                                    \
struct name;                                                    \
template<>                                                      \
struct stl::trait<name> {                                       \
    FOR_EACH(TRAIT_INTERFACE_FUNCTION, __VA_ARGS__)             \
                                                                \
    struct interface {                                          \
        FOR_EACH(TRAIT_INTERFACE_POINTER, __VA_ARGS__)          \
    };                                                          \
    template<typename Self>                                     \
    static constexpr auto impl = requires {                     \
        FOR_EACH(TRAIT_IMPL_CONSTRAINT, __VA_ARGS__)            \
    };                                                          \
    template<typename Self> requires impl<Self>                 \
    static constexpr auto interface_for = interface{            \
        FOR_EACH(TRAIT_INTERFACE_THUNK, __VA_ARGS__)            \
    };                                                          \
    struct dyn {                                                \
        interface const* vtable = {};                           \
        void*            object = {};                           \
                                                                \
        template<typename Self>                                 \
        constexpr dyn(Self& self) noexcept                      \
        : vtable(std::addressof(interface_for<Self>))           \
        , object(std::addressof(self)) {}                       \
                                                                \
        FOR_EACH(TRAIT_METHOD, __VA_ARGS__)                     \
    };                                                          \
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
        using type = R(*)(void*, Args...);
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

    template <typename>
    struct trait;

    template <typename Trait>
    using dyn = trait<Trait>::dyn;

    template <typename Self, typename Trait>
    concept impl = trait<Trait>::template impl<Self>;
}

using f32 = float;

// declare Shape trait
trait(Shape,
    (area, auto() const -> f32),
    (name, auto() const -> std::string)
);

struct Circle {
    f32 radius = {};

    auto area() const -> f32 {
        return 2.0F * f32(M_PI) * radius;
    }

    auto name() const -> std::string {
        return "Circle";
    }
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
    std::cout 
        << __FUNCTION__ 
        << ": "
        << shape.name() 
        << "'s radius is " 
        << shape.area()
        << std::endl;
}

void test_dyn(stl::dyn<Shape> const& shape) {
    std::cout 
        << __FUNCTION__ 
        << ": "
        << shape.name() 
        << "'s radius is " 
        << shape.area()
        << std::endl;
}

int main() {
    // stl::impl is concept
    stl::impl<Shape> auto circle = Circle(2.0F);
    stl::impl<Shape> auto square = Square(10.0F, 20.0F);

    // stl::dyn uses type-erasure and stores reference to object
    // When 'c' goes out of scope, 's' will point to dangling pointer
    stl::dyn<Shape> shape = circle;

    // stl::dyn satisfies stl::impl concept, so why not?
    test_impl(shape);

    // stl::dyn can be reassigned
    shape = square;
    test_dyn(shape);

    test_impl(circle);
    test_impl(square);

    // implicit cast to stl::dyn
    test_dyn(circle);
    test_dyn(square);
}
