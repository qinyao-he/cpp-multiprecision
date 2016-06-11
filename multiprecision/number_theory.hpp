//
// Created by 何钦尧 on 6/9/16.
//

#ifndef BIGINT_NUMBER_THEORY_HPP
#define BIGINT_NUMBER_THEORY_HPP

#include <utility>

#include "forward_declare.hpp"
#include "bigint.hpp"

namespace hqythu {

namespace bigint {

template <class T>
T gcd(const T& a, const T& b) {
    if (b == T(0)) {
        return a;
    } else {
        return gcd(b, a % b);
    }
}

template <class T>
std::tuple<T, T, T> ext_gcd(const T& a, const T& b) {
    if (b == T(0)) {
        return std::make_tuple(T(1), T(0), a);
    } else {
        T x, y, q;
        std::tie(x, y, q) = ext_gcd(b, a % b);
        std::tie(x, y) = std::make_pair(y, x - (a / b) * y);
        return std::make_tuple(x, y, q);
    }
}

template <class T>
bool miller_rabin_test(const T& p, const T& a) {
    T p_ = p - T(1);
    T q = p_;
    int k = 0;
    while (q % T(2) == T(0)) {
        q = q / T(2);
        k++;
    }
    T t = pow(a, q, p);
    if (t == T(1)) {
        return true;
    }
    if (t == p_) {
        return true;
    }
    while (k > 0) {
        t = (t * t) % p;
        if (t == p_) {
            return true;
        }
        k--;
    }
    return false;
}

template <class T>
bool is_prime_miller_rabin(const T& p) {
    if (p == T(2)) {
        return true;
    }
    if (p % T(2) == T(0)) {
        return false;
    }
    T p_ = p - T(1);
    int iter_time = 10;
    while (iter_time) {
        T a = T::random() % p_;
        if (a == T(0)) {
            continue;
        }
        assert(a < p_);
        bool flag = miller_rabin_test(p, a);
        if (!flag) {
            return false;
        }
        iter_time--;
    }
    return true;
}

}

}

#endif //BIGINT_NUMBER_THEORY_HPP
