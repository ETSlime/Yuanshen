#pragma once
//=============================================================================
//
// ���s�V�悭���Ԃ����L���[����� [SimpleQueue.h]
// Author : 
// FIFO�i������o���j���[���ŃA�C�e������ׂĂ����A������Ƃ���Ȏ���L���[�N���X��
// �z�o�b�t�@�Ō����悭�����āA���T�C�Y���΂�����Ή����邨�������C�h�^�C�v�Ȃ�~
//
//=============================================================================

template<typename T>
class SimpleQueue 
{
private:
    T* array;
    int capacity;
    int size;
    int head;
    int tail;

public:
    SimpleQueue() : array(nullptr), capacity(0), size(0), head(0), tail(0) {}

    ~SimpleQueue() 
    {
        if (array)
            delete[] array;
    }

    void enqueue(const T& item) 
    {
        if (size >= capacity) 
        {
            int newCapacity = capacity == 0 ? 4 : capacity * 2;
            T* newArray = new T[newCapacity];

            // �z��̗v�f��V�����z��ɍĔz�u
            int current = head;
            for (int i = 0; i < size; i++) 
            {
                newArray[i] = array[current];
                current = (current + 1) % capacity;
            }

            delete[] array;
            array = newArray;
            capacity = newCapacity;
            head = 0;
            tail = size;
        }

        array[tail] = item;
        tail = (tail + 1) % capacity;
        size++;
    }

    void dequeue() 
    {
        if (size == 0)
            return;

        head = (head + 1) % capacity;
        size--;

        if (size == 0) 
        {
            head = 0;
            tail = 0;
        }
    }

    // �z��̐擪�v�f
    T& front() 
    {
        return array[head];
    }

    // �z��̍Ō�̗v�f
    T& back() 
    {
        return array[(tail - 1 + capacity) % capacity];
    }

    void clear() 
    {
        delete[] array;
        array = nullptr;
        size = 0;
        capacity = 0;
        head = 0;
        tail = 0;
    }

    int getSize() const 
    {
        return size;
    }

    bool isEmpty() const 
    {
        return size == 0;
    }
};
