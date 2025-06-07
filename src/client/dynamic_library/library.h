#pragma once
#include <dlfcn.h>
#include <string>

class DynamicLibrary {
public:
    DynamicLibrary(const std::string& path);

    auto isLoaded() const -> bool;

    template<typename T>
    auto loadFunction(const std::string &name) const -> T *;

private:
    void* mHandle = nullptr;
};

template<typename T>
auto DynamicLibrary::loadFunction(const std::string &name) const -> T * {
    if (!isLoaded()) {
        return nullptr;
    }
    return (T*) (dlsym(mHandle, name.c_str()));
}
