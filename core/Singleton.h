#ifndef SINGLETON_H_INCLUDED
#define SINGLETON_H_INCLUDED

namespace tim
{

    template<class T>
    class Singleton
    {
    public:
        static T& instance();

        static void freeInstance()
        {
            delete _instance;
        }

    protected:
        Singleton(){}

    private:
        static T* _instance;

        Singleton(const Singleton&);
        Singleton& operator=(const Singleton&);
    };

    template<class T>
    T& Singleton<T>::instance()
    {
        if(!_instance)
            _instance = new T;
        return *_instance;
    }

    template<class T>
    T* Singleton<T>::_instance = nullptr;
}

#endif // SINGLETON_H_INCLUDED
