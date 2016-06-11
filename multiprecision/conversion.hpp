//
// Created by 何钦尧 on 6/7/16.
//

#ifndef BIGINT_CONVERSION_H
#define BIGINT_CONVERSION_H

#include <cmath>
#include <sstream>
#include <string>

#include "forward_declare.hpp"
#include "bigint.hpp"

namespace hqythu {

namespace bigint {

using std::string;

template <int N>
void BigInt<N>::from_int(int v) {
    len = 1;
    data[0] = std::abs(v);
    positive = (v >= 0);
}

template <int N>
void BigInt<N>::from_hex_string(string v) {
    if (v.find("-") == 0) {
        positive = false;
        v = v.substr(1);
    }
    string inv(v.rbegin(), v.rend());
    int i = 0;
    int limb_len = sizeof(limb_type) * 2;
    while (i < N_limbs && i * limb_len < v.length()) {
        std::stringstream ss;
        string substr = inv.substr(i * limb_len, limb_len);
        substr = string(substr.rbegin(), substr.rend());
        ss << std::hex << substr;
        ss >> data[i];
        i++;
    }
    len = i;
}

template <int N>
int BigInt<N>::to_int() const {
    return data[0];
}

template <int N>
string BigInt<N>::to_hex_string() const {
    string str;
    for (int i = 0; i < len; i++) {
        std::stringstream ss;
        string substr;
        ss << std::hex << data[i];
        ss >> substr;
        if (substr.length() < sizeof(limb_type) * 2) {
            substr.insert(0, string(sizeof(limb_type) * 2 - substr.length(), '0'));
        }
        str.insert(0, substr);
    }
    if (!positive) {
        str.insert(0, "-");
    }
    return str;
}

}

}

#endif //BIGINT_CONVERSION_H
