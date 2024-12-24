#pragma once
template<typename T>
class SimpleArray 
{
private:
    T* array;
    int capacity;
    int size;

public:
    SimpleArray() : array(nullptr), capacity(0), size(0) {}

    ~SimpleArray() 
    {
        if (array)
            delete[] array;
    }

    void push_back(const T& item) {
        if (size >= capacity) {
            capacity = capacity == 0 ? 4 : capacity * 2;
            T* newArray = new T[capacity];
            for (int i = 0; i < size; i++) {
                newArray[i] = array[i];
            }
            delete[] array;
            array = newArray;
        }
        array[size++] = item;
    }

    void clear()
    {
        delete[] array;
        array = nullptr;
        size = 0;
        capacity = 0;
    }

    int getSize() const {
        return size;
    }

    T& operator[](int index) {
        return array[index];
    }

    const T& operator[](int index) const {
        return array[index];
    }
};