#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <iomanip>
#include "vector.h"

struct big_integer {
    int sign;
    vector<unsigned int> digits;

    big_integer();
    big_integer(int);
    explicit big_integer(const std::string&);
    big_integer(big_integer &&) = default;
    big_integer(const big_integer&) = default;
    ~big_integer() = default;

    big_integer& operator=(const big_integer&) = default;
    big_integer& operator+=(const big_integer&);
    big_integer& operator-=(const big_integer&);
    big_integer& operator*=(const big_integer&);
    big_integer& operator/=(const big_integer&);
    big_integer& operator%=(const big_integer&);
    big_integer& operator&=(const big_integer&);

    big_integer& operator|=(const big_integer&);
    big_integer& operator^=(const big_integer&);
    big_integer& operator<<=(int);

    big_integer& operator>>=(int);
    big_integer operator+() const;

    big_integer operator-() const;
    big_integer operator~() const;
    big_integer& operator++();

    big_integer& operator--();
    friend bool operator==(const big_integer&, const big_integer&);

    friend bool operator!=(const big_integer&, const big_integer&);
    friend bool operator<(const big_integer&, const big_integer&);
    friend bool operator>(const big_integer&, const big_integer&);
    friend bool operator<=(const big_integer&, const big_integer&);
    friend bool operator>=(const big_integer&, const big_integer&);

    friend big_integer operator+(const big_integer&, const big_integer&);
    friend big_integer operator-(const big_integer&, const big_integer&);
    friend big_integer operator*(const big_integer&, const big_integer&);
    friend big_integer operator/(const big_integer&, const big_integer&);
    friend big_integer operator%(const big_integer&, const big_integer&);

    friend big_integer operator&(const big_integer&, const big_integer&);
    friend big_integer operator|(const big_integer&, const big_integer&);
    friend big_integer operator^(const big_integer&, const big_integer&);

    friend big_integer operator<<(const big_integer&, int);
    friend big_integer operator>>(const big_integer&, int);

    std::string to_string() const;
private:
    void normalize();
    bool is_zero() const;
    big_integer reverse_bits();
    big_integer to_signed();
    big_integer to_unsigned();
    friend big_integer bitwise_operator(const big_integer&, const big_integer&,
                                 const std::function<unsigned int(unsigned int, unsigned int)>&);
    bool less_than(const big_integer &) const;
    void mul(const unsigned int& number);
    void add(const unsigned int& number);
    unsigned int div(const unsigned int& number);
    void sub(const big_integer& b);
};

bool operator==(const big_integer&, const big_integer&);
bool operator!=(const big_integer&, const big_integer&);
bool operator<(const big_integer&, const big_integer&);
bool operator>(const big_integer&, const big_integer&);
bool operator<=(const big_integer&, const big_integer&);
bool operator>=(const big_integer&, const big_integer&);

std::string to_string(big_integer const& a);

std::ostream& operator<<(std::ostream&, big_integer&);
std::istream& operator>>(std::istream&, big_integer&);