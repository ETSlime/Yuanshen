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

    SimpleArray(UINT initialCapacity): size(0)
    {
        if (initialCapacity < 0)
            return;

        capacity = initialCapacity;
        array = new T[capacity];
    }

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

    const T& front() const
    {
        if (size == 0) 
        {
            return T();
        }
        return array[0];
    }

    const T& back() const
    {
        if (size == 0) 
        {
            return T();
        }
        return array[size - 1]; // Returns a copy of the last element
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

    void resize(int newCapacity) {
        if (newCapacity < size) {
            return;
        }

        T* newArray = new T[newCapacity];
        for (int i = 0; i < size; i++) {
            newArray[i] = array[i];
        }
        delete[] array;
        array = newArray;
        capacity = newCapacity;
    }

    T& operator[](int index) {
        return array[index];
    }

    const T& operator[](int index) const {
        return array[index];
    }
};