#pragma once
#include <iostream>

template <typename T>
class SingletonBase
{
public:

    virtual ~SingletonBase() {
    }

    SingletonBase(const SingletonBase&) = delete;

    SingletonBase& operator=(const SingletonBase&) = delete;

    static T& get_instance() {
        static T instance;

        return instance;
    }

protected:

    
    SingletonBase() {
    }
};
