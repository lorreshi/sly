//
// Created by sly on 7/3/23.
// 字节序操作函数(大端/小端)

#ifndef SLY_ENDIAN_H
#define SLY_ENDIAN_H

#define SYLAR_LITTLE_ENDIAN 1
#define SYLAR_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>

namespace sylar {

/**
 * @brief 8字节类型的字节序转化
 */
    template<class T>
    // std::enable_if 模板，用于在编译时进行条件判断。它的作用是只有当类型 T 的大小等于 uint64_t
    // 的大小时，才定义这个模板函数。如果条件不满足，编译器将选择其他重载或模板特化。
    typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
    byteswap(T value) {
        return (T)bswap_64((uint64_t)value);
    }

/**
 * @brief 4字节类型的字节序转化
 */
    template<class T>
    typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
    byteswap(T value) {
        return (T)bswap_32((uint32_t)value);
    }

/**
 * @brief 2字节类型的字节序转化
 */
    template<class T>
    typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
    byteswap(T value) {
        return (T)bswap_16((uint16_t)value);
    }

#if BYTE_ORDER == BIG_ENDIAN
#define SYLAR_BYTE_ORDER SYLAR_BIG_ENDIAN
#else
#define SYLAR_BYTE_ORDER SYLAR_LITTLE_ENDIAN
#endif

#if SYLAR_BYTE_ORDER == SYLAR_BIG_ENDIAN

/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 */
template<class T>
T byteswapOnLittleEndian(T t) {
    return t;
}

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 */
template<class T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}

#else

/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 */
    template<class T>
    T byteswapOnLittleEndian(T t) {
        return byteswap(t);
    }

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 */
    template<class T>
    T byteswapOnBigEndian(T t) {
        return t;
    }
#endif

}


#endif //SLY_ENDIAN_H
