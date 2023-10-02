
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <memory>
#include <utility>
#include <type_traits>
#include <experimental/meta>

namespace meta = std::experimental::meta;

namespace stl {
    template<typename T, typename U> 
    using like_t = std::conditional_t<std::is_const_v<U>, T const, T>;


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
    using trait_member_pointer_t = typename trait_member_pointer<Self, Fn>::type;

    template<typename>
    struct has_const_qualifier;

    template<typename R, typename Self, typename... Args>
    struct has_const_qualifier<R(Self::*)(Args...)> : std::false_type {};

    template<typename R, typename Self, typename... Args>
    struct has_const_qualifier<R(Self::*)(Args...) const> : std::true_type {};

    template <typename Trait>
    struct vtable_type_t {
        consteval {
            for (auto func : meta::member_fn_range(reflexpr(Trait))) {
                auto params = meta::param_range(func);
                auto return_type = meta::return_type_of(func);
                -> fragment struct {
                    typename(%{return_type})(*unqualid(%{func}))(void*, -> %{params}) = nullptr;
                };
            }
        }
    };

    template <typename Trait, typename Self>
    struct vtable_impl_t {
        vtable_type_t<Trait> vtable;

        constexpr vtable_impl_t() {
            consteval {
                for (auto func : meta::member_fn_range(reflexpr(Trait))) {
                    auto params = meta::param_range(func);
                    auto return_type = meta::return_type_of(func);

                    -> fragment this {
                        this->vtable.unqualid(%{func}) = [](void* self_ptr, -> %{params}) -> typename(%{return_type}) {
                            return static_cast<Self*>(self_ptr)->unqualid(%{func})(unqualid(...%{params}));
                        };
                    };
                }
            }
        }
    };

    template<typename Self, typename Trait>
    consteval auto implements() -> bool {
        template for (constexpr auto func : meta::member_fn_range(reflexpr(Trait))) {
            constexpr auto function_type = meta::type_of(func);

            using Fn = trait_member_pointer_t<Self, typename(function_type)>;

            if constexpr (not requires { static_cast<Fn>(&Self::unqualid(func)); }) {
                return false;
            }
        }
        return true;
    }

    template <typename Trait, typename Self>
    static constexpr auto vtable_for = vtable_impl_t<Trait, Self>().vtable;

    template<typename Self, typename Trait>
    concept impl = implements<Self, Trait>();

    template <typename Trait>
    struct dyn {
        void*                       object;
        vtable_type_t<Trait> const* vtable;

        template<impl<Trait> Self>
        dyn(Self& self) 
        : object(&self)
        , vtable(&vtable_for<Trait, Self>) {}

        consteval {
            for (auto func : meta::member_fn_range(reflexpr(Trait))) {
                auto params = meta::param_range(func);
                auto return_type = meta::return_type_of(func);
                -> fragment struct {
                    auto unqualid(%{func})(-> %{params}) const -> typename(%{return_type}) {
                        return this->vtable->unqualid(%{func})(this->object, unqualid(...%{params}));
                    }
                };
            }
        }
    };
}

using i32 = int;
using f32 = float;

struct Shape {
    auto area() const -> float;
    auto name() const -> std::string;
    void scale_by(float factor);
};

struct Circle {
    f32 radius = {};

    auto area() const -> f32 { return f32(M_PI) * radius * radius; }
    auto name() const -> std::string { return "Circle"; }
    void scale_by(float factor) {}
};

struct Square {
    f32 length = {};
    f32 height = {};

    auto area() const -> f32 { return length * height; }
    auto name() const -> std::string { return "Square"; }
    void scale_by(float factor) {}
};

void test_impl(stl::impl<Shape> auto const& shape) {
    std::cout << "test_impl: " << shape.name() << "'s radius is " << shape.area() << std::endl;
}

void test_dyn(stl::dyn<Shape> const& shape) {
    std::cout << "test_dyn: " << shape.name() << "'s radius is " << shape.area() << std::endl;
}

auto main() -> i32 {
    // stl::impl is concept
    stl::impl<Shape> auto circle = Circle{2.0F};
    stl::impl<Shape> auto square = Square{10.0F, 20.0F};

    // stl::dyn uses type-erasure and stores reference to object
    // When 'circle' goes out of scope, 'shape' will point to dangling pointer
    stl::dyn<Shape> shape = circle;

    // stl::dyn satisfies stl::impl concept, so why not?
    // test_impl(shape);

    // stl::dyn can be reassigned
    shape = square;
    test_dyn(shape);

    test_impl(circle);
    test_impl(square);

    // implicit cast to stl::dyn
    test_dyn(circle);
    test_dyn(square);
    return 0;
}
