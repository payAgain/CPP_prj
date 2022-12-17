//
// Created by YiMing D on 2022/12/11.
//

#ifndef DREAMER_SINGLETON_H
#define DREAMER_SINGLETON_H

#include "memory"

template<class T>
class Singleton{
public:
    static T* getInstance() {
        static T t;
        return &t;
    }
};

template<class T>
class SharedSingleton{
public:
    static std::shared_ptr<T*> getInstance() {
        static std::shared_ptr<T*> t{new T()};
        return t;
    }
};

#endif //DREAMER_SINGLETON_H
