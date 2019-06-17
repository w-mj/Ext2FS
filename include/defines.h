#pragma once

// 获得特定类型的空引用
template <typename T> 
constexpr inline T& null_ref() {
    return *((T*)nullptr);
}

// 判断一个引用是否为空
template <typename T>
inline bool null_ref(const T& t) {
    return (&t) == 0;
}