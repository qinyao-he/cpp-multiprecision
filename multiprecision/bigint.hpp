//
// Created by 何钦尧 on 6/7/16.
//

#ifndef BIGINT_BIGINT_HPP
#define BIGINT_BIGINT_HPP

#include <cstdint>
#include <string>

#include "forward_declare.hpp"

namespace hqythu {

namespace bigint {

template <int N>
class BigInt {
    static_assert(N % (sizeof(uint32_t) * 8) == 0, "Bit length N_bits must be multiple of machine word (uint32_t)");
private:
    bool positive;
    uint32_t* data;
    int len;

    void normalize();

public:
    static const int N_bits = N;
    static const int N_limbs;
    static const int limb_bits = 32;
    typedef uint32_t limb_type;
    typedef uint64_t double_limb_type;

    BigInt();
    ~BigInt();
    BigInt(int v);
    BigInt(const std::string& v);
    template <int M>
    BigInt(const BigInt<M>& v);

    BigInt(const BigInt& op);
    BigInt(BigInt&& op);

    bool get_sign() const { return positive; }
    int get_len() const { return len; }
    const limb_type* get_data() const { return data; }

    static int compare(const BigInt& a, const BigInt& b);
    static int compare_unsigned(const BigInt& a, const BigInt& b);

    static void add(const BigInt& a, const BigInt& b, BigInt& result);
    static void sub(const BigInt& a, const BigInt& b, BigInt& result);
    static void mul(const BigInt& a, const BigInt& b, BigInt& result);
    static void div(const BigInt& a, const BigInt& b, BigInt& result, BigInt& residue);

    BigInt& operator = (const BigInt& op);
    BigInt& operator = (BigInt&& op);

    template <int N1, int N2>
    friend typename std::conditional<N1 >= N2, BigInt<N1>, BigInt<N2>>::type operator + (const BigInt<N1>& a, const BigInt<N2>& b);
    template <int N1, int N2>
    friend typename std::conditional<N1 >= N2, BigInt<N1>, BigInt<N2>>::type operator - (const BigInt<N1>& a, const BigInt<N2>& b);
    template <int N1, int N2>
    friend typename std::conditional<N1 >= N2, BigInt<N1>, BigInt<N2>>::type operator * (const BigInt<N1>& a, const BigInt<N2>& b);
    template <int N1, int N2>
    friend typename std::conditional<N1 >= N2, BigInt<N1>, BigInt<N2>>::type operator / (const BigInt<N1>& a, const BigInt<N2>& b);
    template <int N1, int N2>
    friend typename std::conditional<N1 >= N2, BigInt<N1>, BigInt<N2>>::type operator % (const BigInt<N1>& a, const BigInt<N2>& b);

    template <int N1, int N2>
    friend bool operator == (const BigInt<N1>& a, const BigInt<N2>& b);
    template <int N1, int N2>
    friend bool operator != (const BigInt<N1>& a, const BigInt<N2>& b);
    template <int N1, int N2>
    friend bool operator < (const BigInt<N1>& a, const BigInt<N2>& b);
    template <int N1, int N2>
    friend bool operator > (const BigInt<N1>& a, const BigInt<N2>& b);
    template <int N1, int N2>
    friend bool operator <= (const BigInt<N1>& a, const BigInt<N2>& b);
    template <int N1, int N2>
    friend bool operator >= (const BigInt<N1>& a, const BigInt<N2>& b);

    void from_int(int v);
    void from_hex_string(std::string v);
    int to_int() const;
    std::string to_hex_string() const;

    static BigInt random();
};

template <int N>
const int BigInt<N>::N_limbs = N / (sizeof(uint32_t) * 8);

template <int N>
BigInt<N>::BigInt() {
    data = new limb_type[N_limbs];
    std::fill(data, data + N_limbs, 0);
    positive = true;
    len = 1;
}

template <int N>
BigInt<N>::~BigInt() {
    delete[] data;
}

template <int N>
BigInt<N>::BigInt(int v) : BigInt() {
    from_int(v);
}

template <int N>
BigInt<N>::BigInt(const std::string& v) : BigInt() {
    BigInt();
    from_hex_string(v);
}

template <int N>
template <int M>
BigInt<N>::BigInt(const BigInt<M>& v) :BigInt() {
    BigInt();
    this->positive = v.get_sign();
    this->len = std::min(v.get_len(), N_limbs);
    std::copy(v.get_data(), v.get_data() + len, data);
}

template <int N>
BigInt<N>::BigInt(const BigInt &op) {
    positive = op.positive;
    len = op.len;
    data = new limb_type[N_limbs];
    std::copy(op.data, op.data + len, data);
}

template <int N>
BigInt<N>::BigInt(BigInt &&op) {
    positive = op.positive;
    len = op.len;
    data = op.data;
    op.data = nullptr;
}

template <int N>
BigInt<N>& BigInt<N>::operator = (const BigInt &op) {
    positive = op.positive;
    len = op.len;
    std::copy(op.data, op.data + len, data);
    return *this;
}

template <int N>
BigInt<N>& BigInt<N>::operator = (BigInt &&op) {
    positive = op.positive;
    len = op.len;
    delete[] data;
    data = op.data;
    op.data = nullptr;
    return *this;
}

template <int N, int M>
typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type operator + (const BigInt<N>& a, const BigInt<M>& b) {
    typedef typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type result_type;
    result_type res;
    result_type::add(a, b, res);
    return res;
}

template <int N, int M>
typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type operator - (const BigInt<N>& a, const BigInt<M>& b) {
    typedef typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type result_type;
    result_type res;
    result_type::sub(a, b, res);
    return res;
}

template <int N, int M>
typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type operator * (const BigInt<N>& a, const BigInt<M>& b) {
    typedef typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type result_type;
    result_type res;
    result_type::mul(a, b, res);
    return res;
}

template <int N, int M>
typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type operator / (const BigInt<N>& a, const BigInt<M>& b) {
    typedef typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type result_type;
    result_type q, r;
    result_type::div(a, b, q, r);
    return q;
}

template <int N, int M>
typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type operator % (const BigInt<N>& a, const BigInt<M>& b) {
    typedef typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type result_type;
    result_type q, r;
    result_type::div(a, b, q, r);
    return r;
}

template <int N, int M>
bool operator == (const BigInt<N>& a, const BigInt<M>& b) {
    typedef typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type higher_type;
    return higher_type::compare(a, b) == 0;
}

template <int N, int M>
bool operator != (const BigInt<N>& a, const BigInt<M>& b) {
    typedef typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type higher_type;
    return higher_type::compare(a, b) != 0;
}

template <int N, int M>
bool operator < (const BigInt<N>& a, const BigInt<M>& b) {
    typedef typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type higher_type;
    return higher_type::compare(a, b) < 0;
}

template <int N, int M>
bool operator > (const BigInt<N>& a, const BigInt<M>& b) {
    typedef typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type higher_type;
    return higher_type::compare(a, b) > 0;
}

template <int N, int M>
bool operator <= (const BigInt<N>& a, const BigInt<M>& b) {
    typedef typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type higher_type;
    return higher_type::compare(a, b) <= 0;
}

template <int N, int M>
bool operator >= (const BigInt<N>& a, const BigInt<M>& b) {
    typedef typename std::conditional<N >= M, BigInt<N>, BigInt<M>>::type higher_type;
    return higher_type::compare(a, b) >= 0;
}

}

}

#include "arithmetic.hpp"
#include "conversion.hpp"
#include "numeric.hpp"
#include "number_theory.hpp"

#endif //BIGINT_BIGINT_HPP
