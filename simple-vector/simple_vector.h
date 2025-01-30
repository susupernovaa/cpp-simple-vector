#pragma once

#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <algorithm>

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve) 
        : capacity_to_reserve_(capacity_to_reserve)
    {
    }
 
    size_t GetCapacity() {
        return capacity_to_reserve_;
    }
private:
    size_t capacity_to_reserve_ = 0;
};
 
ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;
 
    SimpleVector() noexcept = default;
 
    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) 
        : items_(size), size_(size), capacity_(size)
    {
        std::fill(items_.Get(), items_.Get() + size_, Type());
    }
 
    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) 
        : items_(size), size_(size), capacity_(size)
    {
        std::fill(items_.Get(), items_.Get() + size_, value);
    }
 
    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) 
        : items_(init.size()), size_(init.size()), capacity_(init.size())
    {
        std::copy(init.begin(), init.end(), items_.Get());
    }
 
    SimpleVector(const SimpleVector& other) 
        : items_(other.GetSize()), size_(other.GetSize()), capacity_(other.GetCapacity())
    {
        std::copy(other.begin(), other.end(), items_.Get());
    }
 
    SimpleVector(SimpleVector&& other) 
        : items_(std::move(other.items_)), 
        size_(std::exchange(other.size_, 0)), 
        capacity_(std::exchange(other.capacity_, 0))
    {
    }
 
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            SimpleVector tmp(rhs);
            swap(tmp);
        } 
        return *this;
    }
 
    SimpleVector(ReserveProxyObj obj) 
        : capacity_(obj.GetCapacity())
    {
    }
 
    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }
 
    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }
 
    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }
 
    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return *(items_.Get() + index);
    }
 
    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return *(items_.Get() + index);
    }
 
    // Возвращает ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Out of range");
        }
        return *(items_.Get() + index);
    }
 
    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Out of range");
        }
        return *(items_.Get() + index);
    }
 
    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        Resize(0);
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        } else if (new_size <= capacity_) {
            std::generate(items_.Get() + size_, items_.Get() + new_size, []() {return Type();});
            size_ = new_size;
        } else {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> tmp(new_capacity);
            std::copy(std::make_move_iterator(items_.Get()), std::make_move_iterator(items_.Get() + size_), tmp.Get());
            std::generate(tmp.Get() + size_, tmp.Get() + new_capacity, []() {return Type();});
            items_.swap(tmp);
            size_ = new_size;
            capacity_ = new_capacity;
        }
    }
 
    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }
 
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.Get() + size_;
    }
 
    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.Get();
    }
 
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }
 
    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }
 
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }
 
    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ < capacity_) {
            items_[size_] = item;
            ++size_;
        } else {
            size_t new_capacity = (capacity_ == 0 ? 1 : capacity_ * 2);
            Reserve(new_capacity);
            std::swap(items_[size_], item);
            ++size_;  
        }
    }
 
    void PushBack(Type&& item) {
        if (size_ < capacity_) {
            std::swap(items_[size_], item);
            ++size_;
        } else {
            size_t new_capacity = (capacity_ == 0 ? 1 : capacity_ * 2);
            Reserve(new_capacity);
            std::swap(items_[size_], item);
            ++size_;
        }
    }
 
    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        size_t index = static_cast<size_t>(pos - items_.Get());
        if (size_ >= capacity_) {
            size_t new_capacity = (capacity_ == 0 ? 1 : capacity_ * 2);
            Reserve(new_capacity);
            capacity_ = new_capacity; 
        }
        std::copy_backward(items_.Get() + index, items_.Get() + size_, items_.Get() + size_ + 1);
        items_[index] = value;
        ++size_;
        return items_.Get() + index;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        size_t index = static_cast<size_t>(pos - items_.Get());
        if (size_ >= capacity_) {
            size_t new_capacity = (capacity_ == 0 ? 1 : capacity_ * 2);
            Reserve(new_capacity);
            capacity_ = new_capacity; 
        }
        std::copy_backward(std::make_move_iterator(items_.Get() + index), 
            std::make_move_iterator(items_.Get() + size_), items_.Get() + size_ + 1);
        std::swap(items_[index], value);
        ++size_;
        return items_.Get() + index;
    }
 
    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        --size_;
    }
 
    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos <= end());
        size_t index = static_cast<size_t>(pos - items_.Get());
        std::copy(std::make_move_iterator(items_.Get() + index + 1), 
        std::make_move_iterator(items_.Get() + size_), items_.Get() + index);
        --size_;
        return items_.Get() + index;
    }
 
    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
 
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> tmp(new_capacity);
            std::copy(std::make_move_iterator(items_.Get()), 
                std::make_move_iterator(items_.Get() + size_), tmp.Get());
            items_.swap(tmp);
            capacity_ = new_capacity;  
        }
    }
 
private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};
 
template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() == rhs.GetSize()) {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
    return false;
}
 
template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}
 
template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
 
template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}
 
template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs) && lhs != rhs;
}
 
template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}