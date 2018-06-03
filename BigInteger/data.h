//
// Created by vitalya on 02.06.18.
//

#ifndef BIGINT_DATA_H
#define BIGINT_DATA_H

#include <memory>
#include <utility>

size_t static const DEFAULT_CAPACITY = 10;
size_t static const DEFAULT_SIZE = 4;

class data {
public:

    data() = default;

    explicit data(size_t n);

    data(size_t n, unsigned int value);

    data(data const& other);

    data(data&& other) = default;

    data& operator=(data const& other);

    ~data();


    unsigned int* begin();

    unsigned int const* begin() const;

    unsigned int* end();

    unsigned int const* end() const;


    void push_back(unsigned int const& value);

    void pop_back();

    unsigned int& back();

    unsigned int const& back() const;


    unsigned int& operator[](size_t pos);

    unsigned int const& operator[](size_t pos) const;


    bool empty() const;

    size_t size() const;

    void assign(size_t n, unsigned int value);

    void swap(data& other);

    friend bool operator==(data const& a, data const& b);

private:
    bool is_array = true;
    size_t _size = 0;

    struct vector {
        size_t _capacity = DEFAULT_CAPACITY;
        std::shared_ptr<unsigned int> ptr;

        vector() = default;
        vector(size_t capacity, std::shared_ptr<unsigned int> pointer) :
                _capacity(capacity),
                ptr(std::move(pointer))
        {}
    };

    union united {
        unsigned int arr[DEFAULT_SIZE];
        vector vec;

        united() {}
        ~united() {}
    } _data;

    size_t make_capacity(size_t n);

    unsigned int* get_data() const;

    void make_unique();
};


#endif //BIGINT_DATA_H
