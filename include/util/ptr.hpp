#pragma once

#include <memory>

template<auto Fn>
struct deleter_type {
    template <typename T>
    constexpr void operator()(T *ptr) const
    {
        Fn(const_cast<std::remove_const_t<T> *>(ptr));
    }
};

template <typename T, auto Fn>
using c_unique_ptr = std::unique_ptr<T, deleter_type<Fn>>;