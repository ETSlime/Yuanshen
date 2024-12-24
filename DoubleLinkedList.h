#pragma once

template<typename T>
class Node 
{
public:
    T data;
    Node* prev;
    Node* next;

    Node(T val) : data(val), prev(nullptr), next(nullptr) {}
};

template<typename T>
class DoubleLinkedList 
{
private:
    Node<T>* head;
    Node<T>* tail;
    int size;

public:
    DoubleLinkedList() : head(nullptr), tail(nullptr), size(0) {}

    ~DoubleLinkedList() 
    {
        clear();
    }

    Node<T>* getHead() const 
    {
        return head;
    }

    void push_back(const T& item) 
    {
        Node<T>* newNode = new Node<T>(item);
        if (tail == nullptr) 
        {
            head = tail = newNode;
        }
        else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        size++;
    }

    void push_front(const T& item) 
    {
        Node<T>* newNode = new Node<T>(item);
        if (head == nullptr) 
        {
            head = tail = newNode;
        }
        else {
            newNode->next = head;
            head->prev = newNode;
            head = newNode;
        }
        size++;
    }

    void pop_back() 
    {
        if (tail == nullptr) return;
        Node<T>* temp = tail;
        tail = tail->prev;
        if (tail == nullptr) 
        {
            head = nullptr;
        }
        else 
        {
            tail->next = nullptr;
        }
        delete temp;
        size--;
    }

    void pop_front() 
    {
        if (head == nullptr) return;
        Node<T>* temp = head;
        head = head->next;
        if (head == nullptr) 
        {
            tail = nullptr;
        }
        else 
        {
            head->prev = nullptr;
        }
        delete temp;
        size--;
    }

    void remove(Node<T>* node) 
    {
        if (node == nullptr) return;

        if (node == head) {
            pop_front();
        }
        else if (node == tail) {
            pop_back();
        }
        else 
        {
            if (node->prev) 
            {
                node->prev->next = node->next;
            }
            if (node->next) 
            {
                node->next->prev = node->prev;
            }
            delete node;
            size--;
        }
    }

    void clear() 
    {
        Node<T>* current = head;
        while (current != nullptr) 
        {
            Node<T>* next = current->next;
            delete current;
            current = next;
        }
        head = tail = nullptr;
        size = 0;
    }

    int getSize() const 
    {
        return size;
    }
};
