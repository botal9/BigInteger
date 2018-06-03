#include "big_integer.h"
#include <utility>
#include <cassert>

typedef unsigned int ui;
typedef unsigned long long ull;

typedef data uint_array;

big_integer::big_integer() {
    sign = 1;
    digits.push_back(0);
}

big_integer::big_integer(int x) {
    if (x < 0) {
        sign = -1;
    } else {
        sign = 1;
    }
    digits.push_back((ui) std::abs((long long)x));
}

big_integer::big_integer(char sign, uint_array const& digits) : sign(sign), digits(digits) {
    this->normalize();
}

big_integer::big_integer(const std::string& s) {
    size_t start = 0;
    if (s[0] == '-') {
        sign = -1;
        start = 1;
    } else {
        sign = 1;
    }
    for (size_t i = start; i < s.size(); i++) {
        mul(10);
        add((ui) (s[i] - '0'));
    }
    normalize();
}

big_integer operator+(const big_integer& a, const big_integer& b) {
    if (a.sign == b.sign) {
        ui propagate = 0;
        uint_array digits(std::max(a.digits.size(), b.digits.size()), 0);
        for (size_t i = 0; i < digits.size(); i++) {
            ull result = 0;
            if (i >= b.digits.size()) {
                result = (ull) a.digits[i] + propagate;
            } else if (i >= a.digits.size()) {
                result = (ull) b.digits[i] + propagate;
            } else {
                result = (ull) b.digits[i] + a.digits[i] + propagate;
            }
            propagate = (ui) (result >> 32u);
            digits[i] = (ui) result;
        }
        if (propagate != 0)
            digits.push_back(propagate);
        return big_integer(a.sign, digits);
    } else if (b.sign == -1) {
        return a - big_integer(1, b.digits);
    } else {
        return b - big_integer(1, a.digits);
    }
}

big_integer operator-(const big_integer& a, const big_integer& b) {
    if (a.sign == b.sign) {
        if (b.less_than(a)) {
            uint_array digits(a.digits);
            ui propagate = 0;
            for (size_t i = 0; i < b.digits.size(); i++) {
                ull result = (ull) a.digits[i] -
                             (ull) b.digits[i] - propagate;
                propagate = (ui) (a.digits[i] < (ull) b.digits[i] + propagate);
                digits[i] = (ui) result;
            }
            if (propagate != 0) {
                digits[b.digits.size()] -= propagate;
            }
            return big_integer(a.sign, digits);
        } else if (a == b) {
            return big_integer(0);
        } else {
            big_integer ans = b - a;
            ans.sign = a.sign * (char)-1;
            return ans;
        }
    } else if (b.sign == -1) {
        return a + big_integer(1, b.digits);
    } else {
        return a + big_integer(-1, b.digits);
    }
}

big_integer operator*(const big_integer& a, const big_integer& b) {
    uint_array digits(a.digits.size() * b.digits.size(), 0);
    ull result;
    ui pointer = 0;
    ui pt_copy = 0;
    for (ui digit : a.digits) {
        ui propagate = 0;
        for (ui digit1 : b.digits) {
            result = (ull) digit * (ull) digit1
                     + digits[pointer] + propagate;
            digits[pointer] = (ui) result;
            propagate = (ui) (result >> 32u);
            ++pointer;
        }
        if (digits.size() <= pointer) {
            digits.push_back(0);
        }
        digits[pointer] += propagate;
        pointer = (++pt_copy);
    }
    return big_integer(a.sign * b.sign, digits);
}

big_integer operator/(const big_integer& a, const big_integer& b) {
    assert(!b.is_zero());
    unsigned long long const MX = 4294967296u; // max unsigned int + 1
    big_integer copy_a(a);
    big_integer copy_b(b);
    if (b.digits.size() == 1) {
        big_integer ans(a);
        ans.div(b.digits[0]);
        ans.sign = a.sign * b.sign;
        return ans;
    }
    copy_a.sign = copy_b.sign = 1;
    if (copy_a < copy_b) {
        return big_integer(0);
    } else if (copy_a == copy_b) {
        return big_integer(1);
    }
    auto d = (ui)(MX / (b.digits.back() + 1));
    copy_a.mul(d);
    copy_b.mul(d);
    copy_a.normalize();
    copy_b.normalize();
    size_t sz_a = copy_a.digits.size(), sz_b = copy_b.digits.size();
    uint_array digits(sz_a - sz_b + 1, 0);
    big_integer dividend, divider;
    dividend.digits.assign(sz_b + 1, 0);
    for (size_t i = 0; i < sz_b; i++) {
        dividend.digits[i] = copy_a.digits[sz_a - sz_b + i];
    }
    dividend.digits[sz_b] = 0;
    for (size_t i = 0; i < sz_a - sz_b + 1; i++) {
        size_t pos = sz_a - sz_b - i;
        dividend.digits[0] = copy_a.digits[pos];
        ull p1 = MX * (sz_b < dividend.digits.size() ? dividend.digits[sz_b] : 0);
        ull p2 = (sz_b - 1 < dividend.digits.size() ? dividend.digits[sz_b - 1] : 0);
        auto tmp = (ui)std::min(((p1 + p2) / copy_b.digits.back()), MX - 1);
        divider = copy_b;
        divider.mul(tmp);
        while (dividend.less_than(divider)) {
            --tmp;
            divider -= copy_b;
        }
        dividend.sub(divider);
        for (size_t j = sz_b; j--;) {
            dividend.digits[j + 1] = dividend.digits[j];
        }
        digits[pos] = tmp;
    }
    return big_integer(a.sign * b.sign, digits);
}

big_integer operator%(const big_integer& a, const big_integer& b) {
    big_integer ans = a - a / b * b;
    ans.normalize();
    return ans;
}

big_integer operator&(const big_integer& a, const big_integer& b) {
    return bitwise_operator(a, b, [](ui x, ui y) {return x & y;});
}

big_integer operator|(const big_integer& a, const big_integer& b) {
    return bitwise_operator(a, b, [](ui x, ui y) {return x | y;});
}

big_integer operator^(const big_integer& a, const big_integer& b) {
    return bitwise_operator(a, b, [](ui x, ui y) {return x ^ y;});
}

big_integer operator<<(const big_integer& a, int b) {
    assert(b >= 0);
    if (b == 0) {
        return a;
    }
    size_t cnt = (ui)b / 32;
    uint_array digits(a.digits.size() + cnt, 0);
    for (size_t i = 0; i < a.digits.size(); ++i) {
        digits[i + cnt] = a.digits[i];
    }
    b %= 32;
    if (b > 0) {
        ull propagate = 0;
        ull result = 0;
        for (size_t i = cnt; i < digits.size(); ++i) {
            result = ((ull) digits[i] << (ui)b) + propagate;
            digits[i] = (ui)result;
            propagate = (ui)(result >> 32u);
        }
        if (propagate != 0) {
            digits.push_back((ui)propagate);
        }
    }
    return big_integer(a.sign, digits);
}

big_integer operator>>(const big_integer& a, int b) {
    assert(b >= 0);
    if (b == 0) {
        return a;
    }
    if (a.sign < 0) {
        big_integer copy_a = ~a;
        copy_a = (copy_a >> b) + 1;
        copy_a.sign = -1;
        return copy_a;
    }
    size_t cnt = (ui)b / 32;
    uint_array digits(a.digits.size() - cnt);
    for (size_t i = cnt; i < a.digits.size(); ++i) {
        digits[i - cnt] = a.digits[i];
    }
    b %= 32;
    if (b > 0) {
        ull propagate = 0;
        ull result = 0;
        for (size_t i = digits.size(); i--;) {
            ui tmp = digits[i] & ((1u << (ui)b) - 1);
            result = (digits[i] >> (ui)b);
            digits[i] = (ui)(result + (propagate << (ui)(32 - b)));
            propagate = tmp;
        }
    }
    big_integer ans(1, digits);
    return ans;
}

big_integer& big_integer::operator+=(const big_integer& b) {
    *this = *this + b;
    return *this;
}


big_integer &big_integer::operator-=(const big_integer& b) {
    *this = *this - b;
    return *this;
}

big_integer& big_integer::operator*=(const big_integer& b) {
    *this = *this * b;
    return *this;
}

big_integer &big_integer::operator/=(const big_integer& b) {
    *this = *this / b;
    return *this;
}

big_integer &big_integer::operator%=(const big_integer& b) {
    *this = *this % b;
    return *this;
}

big_integer &big_integer::operator&=(const big_integer& b) {
    *this = *this & b;
    return *this;
}


big_integer &big_integer::operator|=(const big_integer& b) {
    *this = *this | b;
    return *this;
}

big_integer &big_integer::operator^=(const big_integer& b) {
    *this = *this ^ b;
    return *this;
}

big_integer &big_integer::operator<<=(int b) {
    *this = (*this << b);
    return *this;
}


big_integer &big_integer::operator>>=(int b) {
    *this = (*this >> b);
    return *this;
}

std::ostream& operator<<(std::ostream& stream, big_integer& b) {
    stream << b.to_string() << std::endl;
    return stream;
}


std::istream& operator>>(std::istream& stream, big_integer& b) {
    std::string s;
    stream >> s;
    b = big_integer(s);
    return stream;
}

bool operator==(const big_integer& a, const big_integer& b) {
    return a.sign == b.sign && a.digits == b.digits;
}


bool operator<(const big_integer& a, const big_integer& b) {
    return a.less_than(b);
}

bool operator>(const big_integer& a, const big_integer& b) {
    return b.less_than(a);
}

bool operator<=(const big_integer& a, const big_integer& b) {
    return !b.less_than(a);
}

bool operator>=(const big_integer& a, const big_integer& b) {
    return !a.less_than(b);
}

bool operator!=(const big_integer& a, const big_integer& b) {
    return !(a == b);
}

big_integer big_integer::operator-() const  {
    return big_integer(this->sign * (char)-1, this->digits);
}


big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator~() const {
    return -(*this) - 1;
}

big_integer& big_integer::operator++() {
    *this += 1;
    return *this;
}


big_integer& big_integer::operator--() {
    *this -= 1;
    return *this;
}

big_integer big_integer::to_signed() {
    digits.push_back(0);
    if (sign < 0) {
        reverse_bits();
        *this -= 1;
    }
    sign = 0;
    return *this;
}


big_integer big_integer::to_unsigned() {
    if ((this->digits.back() >> 31u) & 1u) {
        sign = -1;
        *this += 1;
        reverse_bits();
    } else {
        sign = 1;
    }
    return *this;
}

big_integer big_integer::reverse_bits() {
    for (ui& digit : digits) {
        digit = ~digit;
    }
    return *this;
}

big_integer bitwise_operator(const big_integer& a, const big_integer& b,
                             const std::function<ui(ui, ui)>& op) {
    big_integer copy_b(b);
    big_integer copy_a(a);
    while (copy_a.digits.size() < copy_b.digits.size())
        copy_a.digits.push_back(0);
    while (copy_b.digits.size() < copy_a.digits.size())
        copy_b.digits.push_back(0);
    copy_a.to_signed();
    copy_b.to_signed();
    for (size_t i = 0; i < copy_a.digits.size(); ++i) {
        copy_a.digits[i] = op(copy_a.digits[i], copy_b.digits[i]);
    }
    copy_a.to_unsigned();
    copy_a.normalize();
    return copy_a;
}

bool big_integer::is_zero() const {
    return digits.size() == 1 && digits[0] == 0;
}

std::string big_integer::to_string() const {
    if (digits.empty() || this->is_zero()) {
        return "0";
    }
    bool flag = this->sign== -1;
    big_integer copy(*this);
    std::string str = "";
    while (!(copy.is_zero())) {
        ui dig = copy.div(10);
        str += std::to_string(dig);
    }
    if (flag) {
        str += '-';
    }
    return std::string(str.rbegin(), str.rend());
}

void big_integer::normalize() {
    while (digits.size() > 1 && digits.back() == 0) {
        digits.pop_back();
    }
    if (is_zero())
        this->sign = 1;
}

ui big_integer::div(const ui& b) {
    ull propagate = 0;
    for (size_t i = digits.size(); i--;) {
        ull temp = digits[i] + (propagate << 32u);
        digits[i] = (ui) (temp / b);
        propagate = temp % b;
    }
    normalize();
    return (ui) propagate;
}

void big_integer::mul(const ui& b) {
    ui propagate = 0;
    for (ui& digit : digits) {
        ull result = (ull) digit * b + propagate;
        digit = (ui) result;
        propagate = (ui) (result >> 32u);
    }
    if (propagate > 0) {
        digits.push_back(propagate);
    }
}

void big_integer::add(const ui& b) {
    ui propagate = b;
    for (ui& digit : digits) {
        ull result = (ull) digit + propagate;
        digit = (ui) result;
        propagate = (ui) (result >> 32u);
        if (propagate == 0) {
            break;
        }
    }
    if (propagate > 0) {
        digits.push_back(propagate);
    }
}

void big_integer::sub(const big_integer& b) {
    ui propagate = 0;
    size_t i = 0;
    for (; i < b.digits.size(); ++i) {
        ull tmp = (ull) propagate + b.digits[i];
        propagate = (ui) (digits[i] < tmp);
        digits[i] -= tmp;
    }
    for (; propagate; ++i) {
        propagate = (ui) (digits[i] == 0);
        digits[i] -= 1;
    }
}

std::string to_string(const big_integer& a) {
    return a.to_string();
}

bool big_integer::less_than(const big_integer &b) const {
    if (digits.size() < b.digits.size()) {
        return true;
    }
    if (digits.size() == b.digits.size()) {
        for (size_t i = digits.size(); i--;) {
            if (digits[i] < b.digits[i]) {
                return true;
            } else if (digits[i] > b.digits[i]) {
                return false;
            }
        }
        return false;
    }
    return false;
}