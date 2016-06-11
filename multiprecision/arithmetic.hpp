//
// Created by 何钦尧 on 6/7/16.
//

#ifndef BIGINT_ARITHMETIC_HPP
#define BIGINT_ARITHMETIC_HPP

#include <cassert>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <functional>
#include <iostream>
#include <utility>

#include "forward_declare.hpp"
#include "bigint.hpp"

namespace {

using std::pair;

template <class limb_type>
int compare_unsigned(const limb_type* a, int a_len, const limb_type* b, int b_len) {
    if (a_len > b_len) {
        return 1;
    }
    if (a_len < b_len) {
        return -1;
    }
    assert(a_len == b_len);
    int pos_a = a_len - 1, pos_b = b_len - 1;
    while (pos_a > 0 && pos_b > 0 && a[pos_a] == b[pos_b]) {
        pos_a--, pos_b--;
    }
    if (a[pos_a] > b[pos_b]) {
        return 1;
    } else if (a[pos_a] < b[pos_b]) {
        return -1;
    } else {
        return 0;
    }
}

template <class limb_type>
int bit_length(const limb_type* a, int a_len) {
    int res = (a_len - 1) * (sizeof(limb_type) * 8);
    limb_type high = a[a_len - 1];
    while (high) {
        res++;
        high >>= 1;
    }
    return res;
}

template <class limb_type>
int normalize(const limb_type* a, int a_len) {
    while (a_len > 1 && a[a_len - 1] == 0) {
        a_len--;
    }
    return a_len;
}

template <class limb_type, class double_limb_type>
int raw_add(const limb_type* a, int a_len, const limb_type* b, int b_len, limb_type* result, int max_len) {
    const int limb_bits = sizeof(limb_type) * 8;

    double_limb_type carry = 0;

    int res_len = std::max(a_len, b_len);
    int overlap_len = std::min(a_len, b_len);

    const limb_type *pa = a;
    const limb_type *pb = b;
    limb_type *pr = result;

    if (a_len < b_len) {
        std::swap(pa, pb);
    }

    limb_type *pr_end = pr + overlap_len;
    while (pr < pr_end) {
        carry += static_cast<double_limb_type>(*pa) + static_cast<double_limb_type>(*pb);
        *pr = static_cast<limb_type>(carry);
        carry >>= limb_bits;
        pa++, pb++, pr++;
    }
    pr_end += res_len - overlap_len;
    while (pr < pr_end) {
        if (!carry) {
            if (pa != pr) {
                std::copy(pa, pa + (pr_end - pr), pr);
            }
            break;
        }
        carry += static_cast<double_limb_type>(*pa);
        *pr = static_cast<limb_type>(carry);
        carry >>= limb_bits;
        pa++, pr++;
    }
    if (carry && res_len < max_len) {
        res_len++;
        *pr = static_cast<limb_type>(carry);
    }
    res_len = normalize(result, res_len);
    return res_len;
}


template <class limb_type, class double_limb_type>
int raw_sub(const limb_type* a, int a_len, const limb_type* b, int b_len, limb_type* result, int max_len) {
    const int limb_bits = sizeof(limb_type) * 8;

    double_limb_type borrow = 0;

    int res_len = std::max(a_len, b_len);
    int overlap_len = std::min(a_len, b_len);

    int c = compare_unsigned(a, a_len, b, b_len);

    const limb_type *pa = a;
    const limb_type *pb = b;
    limb_type *pr = result;

    if (c < 0) {
        std::swap(pa, pb);
    } else if (c == 0) {
        // TODO: init to 0
    }

    limb_type *pr_end = pr + overlap_len;
    while (pr < pr_end) {
        borrow = static_cast<double_limb_type>(*pa) - static_cast<double_limb_type>(*pb) - borrow;
        *pr = static_cast<limb_type>(borrow);
        borrow = (borrow >> limb_bits) & 1u;
        pa++, pb++, pr++;
    }
    pr_end += res_len - overlap_len;
    while (pr < pr_end) {
        if (!borrow) {
            if (pa != pr) {
                std::copy(pa, pa + (pr_end - pr), pr);
            }
            break;
        }
        borrow = static_cast<double_limb_type>(*pa) - borrow;
        *pr = static_cast<limb_type>(borrow);
        borrow = (borrow >> limb_bits) & 1u;
        pa++, pr++;
    }
    res_len = normalize(result, res_len);
    return res_len;
};

template <class limb_type, class double_limb_type>
int raw_mul(const limb_type* a, int a_len, const limb_type* b, int b_len, limb_type* result, int max_len) {
    const int limb_bits = sizeof(limb_type) * 8;

    std::fill(result, result + max_len, 0);
    int res_len = a_len + b_len;
    res_len = std::min(max_len, res_len);

    double_limb_type carry = 0;

    for (int i = 0; i < a_len; i++) {
        carry = 0;
        for (int j = 0; j < b_len; j++) {
            if (i + j >= res_len) {
                break;
            }
            double_limb_type multi;
            multi = static_cast<double_limb_type>(a[i]) * static_cast<double_limb_type>(b[j]);
            carry += static_cast<limb_type>(multi) + static_cast<double_limb_type>(result[i + j]);
            result[i + j] = static_cast<limb_type>(carry);
            carry >>= limb_bits;
            carry += multi >> limb_bits;
        }
        int j = b_len;
        while (carry) {
            if (i + j < res_len) {
                carry += result[i + j];
                result[i + j] = static_cast<limb_type>(carry);
                carry >>= limb_bits;
            } else {
                break;
            }
            carry += result[i + j];
            result[i + j] = static_cast<limb_type>(carry);
            carry >>= limb_bits;
        }
    }
    res_len = normalize(result, res_len);
    return res_len;
}

template <class limb_type, class double_limb_type>
int raw_mul_karatsuba(const limb_type* a, int a_len, const limb_type* b, int b_len, limb_type* result, int max_len) {
    const int limb_bits = sizeof(limb_type) * 8;

    if (a_len == 0 || b_len == 0) {
        result[0] = 0;
        return 1;
    }

    int m = 1 << static_cast<int>(std::log2(std::max(a_len - 1, b_len - 1)));

    if (a_len <= 1 && b_len <= 1) {
        double_limb_type carry = static_cast<double_limb_type>(*a) * static_cast<double_limb_type>(*b);
        result[0] = static_cast<limb_type>(carry);
        if (max_len > 1) {
            result[1] = static_cast<limb_type>(carry >> limb_bits);
            if (result[1] == 0) {
                return 1;
            } else {
                return 2;
            }
        } else {
            return 1;
        }
    }

    const limb_type* a1 = a + m;
    const limb_type* a0 = a;
    const limb_type* b1 = b + m;
    const limb_type* b0 = b;

    int a0_len = std::min(a_len, m);
    int a1_len = std::max(0, a_len - m);
    int b0_len = std::min(b_len, m);
    int b1_len = std::max(0, b_len - m);

    limb_type* c2 = new limb_type[2 * m];
    std::fill(c2, c2 + 2 * m, 0);
    limb_type* c1 = new limb_type[2 * m + 2];
    std::fill(c1, c1 + 2 * m + 2, 0);
    limb_type* c0 = new limb_type[2 * m];
    std::fill(c0, c0 + 2 * m, 0);
    int c0_len, c1_len, c2_len;

    c2_len = raw_mul_karatsuba<limb_type, double_limb_type>(a1, a1_len, b1, b1_len, c2, 2 * m);
    c0_len = raw_mul_karatsuba<limb_type, double_limb_type>(a0, a0_len, b0, b0_len, c0, 2 * m);

    limb_type* temp1 = new limb_type[m + 1];
    limb_type* temp2 = new limb_type[m + 1];
    std::fill(temp1, temp1 + m + 1, 0);
    std::fill(temp2, temp2 + m + 1, 0);
    int temp1_len, temp2_len;

    temp1_len = raw_add<limb_type, double_limb_type>(a1, a1_len, a0, a0_len, temp1, m + 1);
    temp2_len = raw_add<limb_type, double_limb_type>(b1, b1_len, b0, b0_len, temp2, m + 1);

    limb_type* mul = new limb_type[2 * m + 2];
    std::fill(mul, mul + 2 * m + 2, 0);
    int mul_len = raw_mul_karatsuba<limb_type, double_limb_type>(temp1, temp1_len, temp2, temp2_len, mul, 2 * m + 2);

    mul_len = raw_sub<limb_type, double_limb_type>(mul, mul_len, c2, c2_len, mul, 2 * m + 2);
    c1_len = raw_sub<limb_type, double_limb_type>(mul, mul_len, c0, c0_len, c1, 2 * m + 2);

    limb_type* temp = new limb_type[max_len];
    std::fill(temp, temp + max_len, 0);
    int temp_len;

    int res_len;
    std::fill(result, result + max_len, 0);

    res_len = std::min(max_len, c0_len);
    std::copy(c0, c0 + res_len, result);

    std::copy(c1, c1 + std::min(c1_len, max_len - m), temp + m);
    temp_len = std::min(c1_len, max_len - m) + m;
    res_len = raw_add<limb_type, double_limb_type>(result, res_len, temp, temp_len, result, max_len);

    std::fill(temp, temp + max_len, 0);
    std::copy(c2, c2 + std::min(c2_len, max_len - 2 * m), temp + 2 * m);
    temp_len = std::min(c2_len, max_len - 2 * m) + 2 * m;
    res_len = raw_add<limb_type, double_limb_type>(result, res_len, temp, temp_len, result, max_len);

    delete[] c0;
    delete[] c1;
    delete[] c2;
    delete[] mul;
    delete[] temp;

    return res_len;
}

template <class limb_type, class double_limb_type>
pair<int, int> raw_div(const limb_type* a, int a_len, const limb_type* b, int b_len, limb_type* result, limb_type* residue, int max_len) {
    if (b_len == 1 && b[0] == 1) {
        std::fill(residue, residue + max_len, 0);
        std::copy(a, a + a_len, result);
        return std::make_pair(a_len, 1);
    }
    const int N = max_len * sizeof(limb_type) * 8;
    const int extend_len = max_len * 4;
    int k = 2 * N;
    limb_type* X = new limb_type[extend_len];
    limb_type* D = new limb_type[extend_len];
    std::fill(X, X + extend_len, 0);
    std::fill(D, D + extend_len, 0);
    int X_len, D_len;
    int k_b = bit_length(b, b_len);
    int k_x = k - k_b + 1;
    X[k_x / (sizeof(limb_type) * 8)] = static_cast<limb_type>(1) << (k_x % (sizeof(limb_type) * 8));
    X_len = k_x / (sizeof(limb_type) * 8) + 1;
    std::copy(b, b + b_len, D);
    D_len = b_len;

    limb_type* temp1 = new limb_type[extend_len];
    limb_type* temp2 = new limb_type[extend_len];
    std::fill(temp1, temp1 + extend_len, 0);
    std::fill(temp2, temp2 + extend_len, 0);
    int temp1_len, temp2_len;
    int k_temp2 = 0;
    while (true) {
        temp1_len = raw_mul<limb_type, double_limb_type>(D, D_len, X, X_len, temp1, extend_len);
        std::fill(temp2, temp2 + extend_len, 0);
        k_temp2 = k + 1;
        temp2[k_temp2 / (sizeof(limb_type) * 8)] = static_cast<limb_type>(1) << (k_temp2 % (sizeof(limb_type) * 8));
        temp2_len = k_temp2 / (sizeof(limb_type) * 8) + 1;
        temp1_len = raw_sub<limb_type, double_limb_type>(temp2, temp2_len, temp1, temp1_len, temp1, extend_len);
        temp2_len = raw_mul<limb_type, double_limb_type>(X, X_len, temp1, temp1_len, temp2, extend_len);
        assert(temp2_len > k / (sizeof(limb_type) * 8));
        int equal = std::inner_product(temp2 + k / (sizeof(limb_type) * 8), temp2 + temp2_len, X,
                                       0, std::plus<limb_type>(), std::not_equal_to<limb_type>());
        std::copy(temp2 + k / (sizeof(limb_type) * 8), temp2 + temp2_len, X);
        X_len = temp2_len - k / (sizeof(limb_type) * 8);
        if (equal == 0) {
            break;
        }
    }

    limb_type* temp = new limb_type[extend_len];
    std::fill(temp, temp + extend_len, 0);
    temp1_len = raw_mul<limb_type, double_limb_type>(a, a_len, X, X_len, temp1, extend_len);
    std::copy(temp1 + k / (sizeof(limb_type) * 8), temp1 + k / (sizeof(limb_type) * 8) + max_len, result);
    int result_len, residue_len;
    result_len = temp1_len - k / (sizeof(limb_type) * 8);
    result_len = std::min(result_len, max_len);
    if (result_len == 0) {
        result_len = 1;
    }
    int temp_len = raw_mul<limb_type, double_limb_type>(b, b_len, result, result_len, temp, max_len);
    residue_len = raw_sub<limb_type, double_limb_type>(a, a_len, temp, temp_len, residue, max_len);
    if (compare_unsigned(residue, residue_len, b, b_len) >= 0) {
        limb_type i = 1;
        residue_len = raw_sub<limb_type, double_limb_type>(residue, residue_len, b, b_len, residue, max_len);
        result_len = raw_add<limb_type, double_limb_type>(result, result_len, &i, 1, result, max_len);
    }

    delete[] X;
    delete[] D;
    delete[] temp;
    delete[] temp1;
    delete[] temp2;

    return std::make_pair(result_len, residue_len);
};
}

namespace hqythu {

namespace bigint {

template <int N>
int BigInt<N>::compare_unsigned(const BigInt& a, const BigInt& b) {
    if (a.len > b.len) {
        return 1;
    }
    if (a.len < b.len) {
        return -1;
    }
    assert(a.len == b.len);
    int pos_a = a.len - 1, pos_b = b.len - 1;
    while (pos_a > 0 && pos_b > 0 && a.data[pos_a] == b.data[pos_b]) {
        pos_a--, pos_b--;
    }
    if (a.data[pos_a] > b.data[pos_b]) {
        return 1;
    } else if (a.data[pos_a] < b.data[pos_b]) {
        return -1;
    } else {
        return 0;
    }
}

template <int N>
int BigInt<N>::compare(const BigInt &a, const BigInt &b) {
    if (a.positive && b.positive) {
        return compare_unsigned(a, b);
    }
    if (a.positive && !b.positive) {
        return 1;
    }
    if (!a.positive && b.positive) {
        return -1;
    }
    if (!a.positive && !b.positive) {
        return -compare_unsigned(a, b);
    }
    return 0;
}

template <int N>
void BigInt<N>::normalize() {
    while (len > 1 && data[len - 1] == 0) {
        len--;
    }
    if (len == 1) {
        if (data[0] == 0) {
            positive = true;
        }
    }
    if (len == 0) {
        len = 1;
    }
}

template <int N>
void BigInt<N>::add(const BigInt& a, const BigInt& b, BigInt& result) {
    if (a.positive != b.positive) {
        result.len = raw_sub<limb_type, double_limb_type>(a.data, a.len, b.data, b.len, result.data, N_limbs);
        result.positive = (compare(a, b) >= 0);
    } else {
        result.len = raw_add<limb_type, double_limb_type>(a.data, a.len, b.data, b.len, result.data, N_limbs);
        result.positive = a.positive;
    }
    result.normalize();
}

template <int N>
void BigInt<N>::sub(const BigInt& a, const BigInt& b, BigInt& result) {
    if (a.positive != b.positive) {
        result.len = raw_add<limb_type, double_limb_type>(a.data, a.len, b.data, b.len, result.data, N_limbs);
        result.positive = a.positive;
    } else {
        result.len = raw_sub<limb_type, double_limb_type>(a.data, a.len, b.data, b.len, result.data, N_limbs);
        result.positive = compare(a, b) >= 0;
    }
    result.normalize();
}

template <int N>
void BigInt<N>::mul(const BigInt& a, const BigInt& b, BigInt& result) {
    result.len = raw_mul<limb_type, double_limb_type>(a.data, a.len, b.data, b.len, result.data, N_limbs);
    result.positive = (a.positive == b.positive);
    result.normalize();
}

template <int N>
void BigInt<N>::div(const BigInt& a, const BigInt& b, BigInt& result, BigInt& residue) {
    auto res = raw_div<limb_type, double_limb_type>(a.data, a.len, b.data, b.len, result.data, residue.data, N_limbs);
    result.len = res.first;
    residue.len = res.second;
    result.positive = true;
    residue.positive = true;
    if (!a.positive && b.positive) {
        add(result, BigInt(1), result);
        result.positive = false;
        sub(b, residue, residue);
        residue.positive = true;
    } else if (a.positive && !b.positive) {
        add(result, BigInt(1), result);
        result.positive = false;
        add(b, residue, residue);
        sub(BigInt(0), residue, residue);
        residue.positive = false;
    } else if (!a.positive && !b.positive){
        residue.positive = false;
    }
    result.normalize();
    residue.normalize();
}

}

}

#endif //BIGINT_ARITHMETIC_HPP
