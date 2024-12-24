#pragma once
#include "main.h"

struct CharPtrHash
{
    unsigned int operator()(const char* str) const
    {
        unsigned long hash = 5381; // djb2ƒAƒ‹ƒSƒŠƒYƒ€‚Ì‰Šú’l
        int c;

        // •¶š—ñ‚ğˆê•¶š‚¸‚Âˆ—
        while ((c = *str++))
        {
            hash = ((hash << 5) + hash) + c; // hash * 33 + c
        }

        return hash;
    }
};

struct CharPtrEquals
{
    bool operator()(const char* a, const char* b) const
    {
        return strcmp(a, b) == 0;
    }
};

template<typename Key, typename Value, typename HashFunc, typename EqualsFunc>
class HashMap
{
private:

    struct Data
    {
        Key key;
        Value value;

        Data(const Key& key, const Value& value) : key(key), value(value) {};
    };

    struct Node 
    {
        Data data;
        Node* next;
        Node(const Key& key, const Value& value) : data(key, value), next(nullptr) {}
    };

    Node** buckets;
    int size;

    HashFunc hashFunc;
    EqualsFunc equalsFunc;

public:
    HashMap(int size, HashFunc hashFunc, EqualsFunc equalsFunc)
        : size(size), hashFunc(hashFunc), equalsFunc(equalsFunc)
    {
        buckets = new Node * [size];
        for (int i = 0; i < size; ++i) 
        {
            buckets[i] = nullptr;
        }
    }

    ~HashMap()
    {
        for (int i = 0; i < size; ++i) 
        {
            Node* current = buckets[i];
            while (current != nullptr) 
            {
                Node* toDelete = current;
                current = current->next;
                delete toDelete;
            }
        }
        delete[] buckets;
    }

    void insert(const Key& key, const Value& value)
    {
        unsigned int index = hashFunc(key) % size;
        Node* newNode = new Node(key, value);
        newNode->next = buckets[index];
        buckets[index] = newNode;
    }

    Value* search(const Key& key) const
    {
        unsigned int index = hashFunc(key) % size;
        Node* current = buckets[index];
        while (current != nullptr)
        {
            if (equalsFunc(current->data.key, key))
            {
                return &current->data.value;
            }
            current = current->next;
        }
        return nullptr;
    }

    bool remove(const Key& key)
    {
        unsigned int index = hashFunc(key) % size;
        Node* current = buckets[index];
        Node* previous = nullptr;

        while (current != nullptr)
        {
            if (equalsFunc(current->data.key, key))
            {
                if (previous == nullptr) 
                {
                    buckets[index] = current->next;
                }
                else {
                    previous->next = current->next;
                }
                delete current;
                return true;
            }
            previous = current;
            current = current->next;
        }
        return false;
    }
};