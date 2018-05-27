#include "big_integer.h"


typedef unsigned int ui;
typedef unsigned long long ull;

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
    big_integer ans;
    if (a.sign == b.sign) {
        ui propagate = 0;
        ans.digits.resize(std::max(a.digits.size(), b.digits.size()), 0);
        for (size_t i = 0; i < ans.digits.size(); i++) {
            ull result = 0;
            if (i >= b.digits.size()) {
                result = (ull) a.digits[i] + propagate;
            } else if (i >= a.digits.size()) {
                result = (ull) b.digits[i] + propagate;
            } else {
                result = (ull) b.digits[i] + a.digits[i] + propagate;
            }
            propagate = (ui) (result >> 32u);
            ans.digits[i] = (ui) result;
        }
        if (propagate != 0)
            ans.digits.push_back(propagate);
    } else if (b.sign == -1) {
        big_integer copy_b(b);
        copy_b.sign = 1;
        ans = a - copy_b;
    } else {
        big_integer copy_a(a);
        copy_a.sign = 1;
        ans = b - copy_a;
    }
    ans.normalize();
    return ans;
}

big_integer operator-(const big_integer& a, const big_integer& b) {
    big_integer ans;
    if (a.sign == b.sign) {
        if (b.less_than(a)) {
            ans = a;
            ui propagate = 0;
            for (size_t i = 0; i < b.digits.size(); i++) {
                ull result = (ull) a.digits[i] -
                                            (ull) b.digits[i] - propagate;
                propagate = (ui) (a.digits[i] < (ull) b.digits[i] + propagate);
                ans.digits[i] = (ui) result;
            }
            if (propagate != 0) {
                ans.digits[b.digits.size()] -= propagate;
            }
        } else if (a == b) {
            return big_integer(0);
        } else {
            ans = b - a;
            ans.sign = a.sign * (-1);
        }
    } else if (b.sign == -1) {
        big_integer copy_b(b);
        copy_b.sign = 1;
        ans = a + copy_b;
    } else {
        big_integer copy_b(b);
        copy_b.sign = -1;
        ans = a + copy_b;
    }
    ans.normalize();
    return ans;
}

big_integer operator*(const big_integer& a, const big_integer& b) {
    big_integer ans(0);
    ans.digits.resize(a.digits.size() * b.digits.size(), 0);
    ans.sign = a.sign * b.sign;
    ull result;
    ui pointer = 0;
    ui pt_copy = 0;
    for (ui digit : a.digits) {
        ui propagate = 0;
        for (ui digit1 : b.digits) {
            result = (ull) digit * (ull) digit1
                     + ans.digits[pointer] + propagate;
            ans.digits[pointer] = (ui) result;
            propagate = (ui) (result >> 32u);
            ++pointer;
        }
        if (ans.digits.size() <= pointer) {
            ans.digits.push_back(0);
        }
        ans.digits[pointer] += propagate;
        pointer = (++pt_copy);
    }
    ans.normalize();
    return ans;
}

big_integer operator/(const big_integer& a, const big_integer& b) {
    assert(!b.is_zero());
    unsigned long long const MX = 4294967296u; // max unsigned int + 1
    big_integer ans(0);
    big_integer copy_a(a);
    big_integer copy_b(b);
    if (b.digits.size() == 1) {
        ans = a;
        ans.div(b.digits[0]);
        ans.sign = a.sign * b.sign;
        return ans;
    }
    copy_a.sign = copy_b.sign = 1;
    if (copy_a < copy_b) {
        ans = big_integer(0);
        return ans;
    } else if (copy_a == copy_b) {
        ans = big_integer(1);
        return ans;
    }
    auto d = (ui)(MX / (b.digits.back() + 1));
    copy_a.mul(d);
    copy_b.mul(d);
    copy_a.normalize();
    copy_b.normalize();
    size_t sz_a = copy_a.digits.size(), sz_b = copy_b.digits.size();
    ans.digits.resize(sz_a - sz_b + 1);
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
        auto tmp = (ui)((p1 + p2) / copy_b.digits.back());
        divider = copy_b;
        divider.mul(tmp);
        while (dividend.less_than(divider)) {
            divider = copy_b;
            divider.mul(--tmp);
        }
        dividend.sub(divider);
        for (size_t j = sz_b; j--;) {
            dividend.digits[j + 1] = dividend.digits[j];
        }
        ans.digits[pos] = tmp;
    }
    ans.sign = a.sign * b.sign;
    ans.normalize();
    return ans;
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
    big_integer copy_a(a);
    if (copy_a.sign < 0) {
        copy_a = ~copy_a;
        copy_a = (copy_a << b) + 1;
        copy_a.sign = -1;
        return copy_a;
    }
    ull propagate = 0;
    ull result = 0;
    vector<unsigned int> tmp((ui)b / 32, 0);
    copy_a.digits.insert(copy_a.digits.begin(), tmp.begin(), tmp.end());
    if (b > 0) {
        for (ui &digit : copy_a.digits) {
            result = ((ull) digit << (ui)b);
            digit = (ui)(result + propagate);
            propagate = (ui)(result >> 32u);
        }
    }
    if (propagate != 0) {
        copy_a.digits.push_back((ui)propagate);
    }
    copy_a.normalize();
    return copy_a;
}

big_integer operator>>(const big_integer& a, int b) {
    assert(b >= 0);
    big_integer copy_a(a);
    if (a.sign < 0) {
        copy_a = ~copy_a;
        copy_a = (copy_a >> b) + 1;
        copy_a.sign = -1;
        return copy_a;
    }
    ull propagate = 0;
    ull result = 0;
    size_t cnt = (ui)b / 32;
    copy_a.digits = vector<unsigned int>(copy_a.digits.begin() + cnt, copy_a.digits.end());
    b %= 32;
    if (b > 0) {
        for (size_t i = copy_a.digits.size(); i--;) {
            ui cur = copy_a.digits[i] & ((1u << (ui)b) - 1);
            result = (copy_a.digits[i] >> (ui)b);
            copy_a.digits[i] = (ui)(result + (propagate << (ui)(32 - b)));
            propagate = cur;
        }
    }
    copy_a.normalize();
    return copy_a;
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
    big_integer a(*this);
    a.sign *= -1;
    a.normalize();
    return a;
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
    return digits.size() == 1 && digits.front() == 0;
}

std::string big_integer::to_string() const {
    if (digits.empty() || this->is_zero()) {
        return "0";
    }
    bool flag = this->sign== -1;
    big_integer copy(*this);
    std::string str;
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
    big_integer tmp(b);
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
        assert(i < digits.size());
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
