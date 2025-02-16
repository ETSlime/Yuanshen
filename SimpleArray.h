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

    SimpleArray(const SimpleArray& other) : array(nullptr), capacity(0), size(0)
    {
        if (other.size > 0) {
            array = new T[other.capacity];
            for (int i = 0; i < other.size; i++) {
                array[i] = other.array[i];
            }
            capacity = other.capacity;
            size = other.size;
        }
    }

    SimpleArray& operator=(const SimpleArray& other)
    {
        if (this != &other) {
            delete[] array;
            array = new T[other.capacity];
            for (int i = 0; i < other.size; i++) {
                array[i] = other.array[i];
            }
            capacity = other.capacity;
            size = other.size;
        }
        return *this;
    }

    ~SimpleArray() 
    {
        if (array)
        {
            delete[] array;
            array = nullptr;
        }
            
    }

    void push_back(const T& item) {
        if (size >= capacity) {
            int newCapacity = (capacity == 0) ? 4 : capacity * 2;
            T* newArray = new T[newCapacity];
            //for (int i = 0; i < size; i++) {
            //    newArray[i] = array[i];
            //}
            for (int i = 0; i < size; i++) {
                newArray[i] = std::move(array[i]);
            }
            delete[] array;
            array = newArray;
            capacity = newCapacity;
        }
        array[size] = item;
        size++;
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
        //delete[] array;
        //array = nullptr;
        //size = 0;
        //capacity = 0;

        for (int i = 0; i < size; i++)
        {
            array[i].~T();
        }
        size = 0;
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

    void shrink_to_fit()
    {
        if (size == 0)
        {
            delete[] array;
            array = nullptr;
            capacity = 0;
        }
        else if (size < capacity)
        {
            T* newArray = new T[size];
            for (int i = 0; i < size; i++)
            {
                newArray[i] = std::move(array[i]);
            }
            delete[] array;
            array = newArray;
            capacity = size;
        }
    }


    T& operator[](int index) {
        return array[index];
    }

    const T& operator[](int index) const {
        return array[index];
    }
};