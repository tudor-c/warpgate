#pragma once
#include <dlfcn.h>
#include <string>

class DynamicLibrary {
public:
    DynamicLibrary(const std::string& path);

    bool isLoaded() const;

    template<typename T>
    T* loadFunction(const std::string& name) const;

private:
    void* mHandle = nullptr;
};

template<typename T>
T* DynamicLibrary::loadFunction(const std::string &name) const {
    if (!isLoaded()) {
        return nullptr;
    }
    return (T*) (dlsym(mHandle, name.c_str()));
}
