//#pragma once
//#ifndef __SINGLETON_H__
//#define __SINGLETON_H__
//template <typename T>
//struct Singleton {
//    Singleton() = default;
//    ~Singleton() = default;
//
//    // Delete the copy and move constructors
//    Singleton(const Singleton&) = delete;
//    Singleton& operator=(const Singleton&) = delete;
//    Singleton(Singleton&&) = delete;
//    Singleton& operator=(Singleton&&) = delete;
//public:
//    static T* GetSingleton() {
//        static T instance{};
//        return &instance;
//    }
//
//
//};
//#endif