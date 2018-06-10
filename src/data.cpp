//
// Created by vitalya on 02.06.18.
//

#include "data.h"
#include <cstring>
#include <cassert>

data::data(size_t n) : data(n, 0) {}

data::data(size_t n, unsigned int value) : _size(n) {
    if (n > DEFAULT_SIZE) {
        size_t capacity = make_capacity(_size);
        auto tmp = new unsigned int[capacity];
        new (&_data.vec) vector(capacity, std::shared_ptr<unsigned int>(tmp, std::default_delete<unsigned int[]>()));
        std::fill(_data.vec.ptr.get(), _data.vec.ptr.get() + _size, value);
        is_array = false;
    } else {
        std::fill(_data.arr, _data.arr + _size, value);
        is_array = true;
    }
}

data::data(data const& other) : _size(other._size), is_array(other.is_array) {
    if (is_array) {
        memcpy(_data.arr, other._data.arr, _size * sizeof(unsigned int));
    } else {
        new (&_data.vec) vector(other._data.vec);
    }
}

data::~data() {
    if (!is_array) {
        _data.vec.~vector();
    }
}

data& data::operator=(data other) {
    swap(other);
    return *this;
}

void data::assign(size_t n, unsigned int value) {
    make_unique();
    *this = data(n, value);
}


unsigned int* data::begin() {
    make_unique();
    return get_data();
}

unsigned int const* data::begin() const {
    return get_data();
}

unsigned int* data::end() {
    make_unique();
    return get_data() + _size;
}


unsigned int const* data::end() const {
    return get_data() + _size;
}

unsigned int& data::back() {
    make_unique();
    return *(get_data() + _size - 1);
}


unsigned int const& data::back() const {
    return *(get_data() + _size - 1);
}

void data::push_back(unsigned int value) {
    make_unique();
    if (is_array && _size == DEFAULT_SIZE) {
        size_t capacity = make_capacity(DEFAULT_SIZE + 1);
        auto* tmp = new unsigned int[capacity];
        memcpy(tmp, _data.arr, DEFAULT_SIZE * sizeof(unsigned int));
        new (&_data.vec) vector(capacity, std::shared_ptr<unsigned int>(tmp, std::default_delete<unsigned int[]>()));
        is_array = false;
    }
    if (!is_array && _size == _data.vec._capacity) {
        size_t capacity = make_capacity(_size + 1);
        auto* tmp = new unsigned int[capacity];
        memcpy(tmp, _data.vec.ptr.get(), _size * sizeof(unsigned int));
        _data.vec.ptr.reset(tmp, std::default_delete<unsigned int[]>());
        _data.vec._capacity = capacity;
    }
    (*this)[_size++] = value;
}


void data::pop_back() {
    assert(_size > 0);
    make_unique();
    --_size;
}

unsigned int& data::operator[](size_t pos) {
    assert(0 <= pos && pos < _size);
    make_unique();
    return *(get_data() + pos);
}


unsigned int const& data::operator[](size_t pos) const {
    assert(0 <= pos && pos < _size);
    return *(get_data() + pos);
}

size_t data::size() const {
    return _size;
}

bool data::empty() const {
    return _size == 0;
}

void data::swap(data& other) {
    std::swap(_size, other._size);
    std::swap(is_array, other.is_array);

    char copy[sizeof(united)];
    memcpy(copy, &_data, sizeof(united));
    memcpy(&_data, &other._data, sizeof(united));
    memcpy(&other._data, copy, sizeof(united));
}

bool operator==(data const& a, data const& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}


size_t data::make_capacity(size_t n) {
    if (n < DEFAULT_CAPACITY) {
        return DEFAULT_CAPACITY;
    } else {
        return (n * 3 >> 1u);
    }
}

unsigned int* data::get_data() const {
    return is_array ? const_cast<unsigned int*>(_data.arr) : _data.vec.ptr.get();
}

void data::make_unique() {
    if (is_array || _data.vec.ptr.unique()) {
        return;
    }
    auto tmp = new unsigned int[_data.vec._capacity];
    memcpy(tmp, _data.vec.ptr.get(), _data.vec._capacity * sizeof(unsigned int));
    _data.vec.ptr.reset(tmp, std::default_delete<unsigned int[]>());
}
