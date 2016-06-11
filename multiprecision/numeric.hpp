//
// Created by 何钦尧 on 6/9/16.
//

#ifndef BIGINT_NUMERIC_HPP
#define BIGINT_NUMERIC_HPP

#include <random>
#include <chrono>

namespace hqythu {

namespace bigint {

template <int N>
BigInt<N> BigInt<N>::random() {
    // use current unix time epoch as random seed.
    static std::default_random_engine engine(
            std::chrono::system_clock::now().time_since_epoch().count());
    static std::uniform_int_distribution<limb_type> distribution;

    BigInt<N> r;
    r.len = N_limbs;
    for (int i = 0; i < r.len; i++) {
        r.data[i] = distribution(engine);
    }
    r.positive = true;
    r.normalize();
    return r;
}

template <class T>
T pow(T a, T d, T n) {
    if (d == T(0)) {
        return T(1);
    } else if (d == T(1)) {
        return a % n;
    }
    T t = pow(a, d / T(2), n);
    if (d % T(2) == T(0)) {
        return (t * t) % n;
    } else {
        return (((t * t) % n) * a) % n;
    }
}

}

}

#endif //BIGINT_NUMERIC_HPP
