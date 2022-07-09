#pragma once

namespace parselib { namespace constants {

template <typename Type> inline const Type& empty() {
    static const Type zero{};
    return zero;
}

template <> inline const bool& empty() {
    static const bool zero{false};
    return zero;
}

template <typename Type> inline bool isEmpty(const Type& obj) {
    return empty<Type>() == obj;
}

}}
