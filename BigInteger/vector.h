//
// Created by vitalya on 12.05.18.
//

#ifndef VECTOR_VECTOR_H
#define VECTOR_VECTOR_H

#include <cstdio>
#include <cstring>
#include <cassert>
#include <iostream>

template<typename T>
class vector {
public:
    typedef T* iterator;
    typedef T const* const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    vector() noexcept {
        _size = 0;
        _capacity = DEFAULT_CAPACITY;
        _data = static_cast<T*>(operator new(_capacity * sizeof(T)));
    }

    explicit vector(size_t n) {
        _size = n;
        _capacity = make_capacity(n);
        _data = static_cast<T*>(operator new(_capacity * sizeof(T)));
    }

    vector(size_t n, T value) {
        _size = n;
        _capacity = make_capacity(n);
        _data = static_cast<T*>(operator new(_capacity * sizeof(T)));
        fill(_data, _data + _size, value);
    }

    vector(const_iterator first, const_iterator last) {
        _size = last - first;
        _capacity = make_capacity(_size);
        _data = static_cast<T*>(operator new(_capacity * sizeof(T)));
        iterator pos = _data;
        for (auto it = first; it != last; ++it) {
            *(pos++) = *it;
        }
    }

    vector(vector<T> const& other) {
        _size = other.size();
        _capacity = other.capacity();
        _data = static_cast<T*>(operator new(_capacity * sizeof(T)));
        memcpy(_data, other.data(), _size * sizeof(T));
    }

    vector(vector<T> && other) noexcept {
        _size = other.size();
        _capacity = other.capacity();
        _data = static_cast<T*>(operator new(_capacity * sizeof(T)));
        for (size_t i = 0; i < _size; ++i)
            _data[i] = std::move(other._data[i]);
    }

    ~vector() noexcept {
        for (size_t i = 0; i < _size; ++i)
            _data[i].~T();
        operator delete (_data);
    }

    vector& operator=(vector const& other) {
        if (this == &other)
            return *this;
        destroy();
        _size = other.size();
        _capacity = other.capacity();
        _data = static_cast<T*>(operator new(_capacity * sizeof(T)));
        memcpy(_data, other.data(), _size * sizeof(T));
        return (*this);
    }


    iterator begin() {
        return _data;
    }

    iterator end() {
        return _data + _size;
    }

    const_iterator begin() const {
        return _data;
    }

    const_iterator end() const {
        return _data + _size;
    }

    reverse_iterator rbegin() {
        return reverse_iterator(_data + _size);
    }

    reverse_iterator rend() {
        return reverse_iterator(_data);
    }


    void push_back(T const& value) {
        if (_size == _capacity) {
            reallocate(_size);
        }
        new (_data + _size) T (value);
        _size++;
    }

    void pop_back() {
        assert(_size > 0);
        _data[--_size].~T();
    }

    T& back() {
        assert(_size > 0);
        return *(_data + _size - 1);
    }

    T const& back() const {
        assert (_size > 0);
        return *(_data + _size - 1);
    }

    T& front() {
        assert(_size > 0);
        return *_data;
    }

    T const& front() const {
        assert(_size > 0);
        return *_data;
    }


    bool empty() const {
        return !_size;
    }

    size_t size() const {
        return _size;
    }

    size_t capacity() const {
        return _capacity;
    }


    T* data() {
        return _data;
    }

    T const* data() const {
        return _data;
    }

    T& operator[](size_t pos) {
        if (!(0 <= pos && pos < _size))
            std::cout << pos << " " << _size << std::endl;
        assert(0 <= pos && pos < _size);
        return *(_data + pos);
    }

    T const& operator[](size_t pos) const {
        assert(0 <= pos && pos < _size);
        return *(_data + pos);
    }


    void reserve(size_t n) {
        if (_capacity < n) {
            reallocate(n);
        }
    }

    void resize(size_t n, T value = T()) {
        if (_capacity < n) {
            reallocate(n);
        }
        for (size_t i = _size; i < n; ++i)
            new (_data + i) T (value);
        for (size_t i = n; i < _size; ++i)
            _data[i].~T();
        _size = n;
    }

    void assign(size_t n, T value = T()) {
        if (_capacity < n) {
            _capacity = make_capacity(n);
            T* tmp = static_cast<T*>(operator new(_capacity * sizeof(T)));
            destroy();
            _data = tmp;
        }
        _size = n;
        fill(begin(), end(), value);
    }

    void shrink_to_fit() {
        _capacity = _size;
        T* tmp = operator new(_capacity * sizeof(T));
        memcpy(tmp, _data, _size * sizeof(T));
        destroy();
        _data = tmp;
    }

    void clear() {
        for (size_t i = 0; i < _size; ++i)
            _data[i].~T();
        _size = 0;
    }


    iterator insert(const_iterator pos, T const& value) {
        assert(begin() <= pos && pos < end());
        reserve(_size + 1);
        memmove(pos + 1, pos, (_size - (pos - _data)) * sizeof(T));
        new (pos) T (value);
        ++_size;
        return pos;
    }

    iterator insert(const_iterator pos, const_iterator first, const_iterator last) {
        assert(begin() <= pos && pos < end());
        iterator it = const_cast<iterator>(pos);
        size_t cnt = last - first;
        if (!cnt)
            return it;
        reserve(_size + cnt);
        memmove(it + cnt, it, (_size - (it - _data)) * sizeof(T));
        for (auto tmp_it = it; first != last; ++first, ++tmp_it) {
            *tmp_it = *first;
        }
        _size += cnt;
        return it;
    }

    iterator erase(const_iterator pos) {
        assert(begin() <= pos && pos < end());
        iterator it = const_cast<iterator>(pos);
        (*it).~T();
        memmove(it, it + 1, (_size - (it + 1 - _data)) * sizeof(T));
        --_size;
        return it;
    }

    iterator erase(const_iterator first, const_iterator last) {
        assert(begin() <= first && first < end());
        assert(begin() < last && last <= end());
        iterator it = const_cast<iterator>(first);
        if (first == last) return it;
        for ( ; first != last; ++first)
            (*first).~T();
        memmove(it, last, (_size - (last - _data)) * sizeof(T));
        _data -= last - first;
        return it;
    }


    friend bool operator==(vector<T> const& a, vector<T> const& b)  {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i)
            if (a[i] != b[i])
                return false;
        return true;
    }

    friend void swap(vector<T>& a, vector<T>& b) {
        std::swap(a._data, b._data);
        std::swap(a._size, b._size);
        std::swap(a._capacity, b._capacity);
    }

private:
    size_t const DEFAULT_CAPACITY = 10;
    T* _data;
    size_t _size;
    size_t _capacity;

    size_t make_capacity(size_t n) {
        if (n < DEFAULT_CAPACITY) {
            return DEFAULT_CAPACITY;
        } else {
            return (n * 3 >> 1u);
        }
    }

    inline void destroy() {
        this->~vector();
    }

    inline void reallocate(size_t n) {
        _capacity = make_capacity(n);
        auto tmp = static_cast<T*>(operator new( _capacity * sizeof(T)));
        memcpy(tmp, _data, _size * sizeof(T));
        destroy();
        _data = tmp;
    }

    void fill(iterator begin, iterator end, T const& value) {
        while (begin != end) {
            new (begin) T(value);
            begin++;
        }
    };
};

#endif //VECTOR_VECTOR_H
