#pragma once
#include <dlfcn.h>
#include <string>

class DynamicLibrary {
public:
    explicit DynamicLibrary(const std::string& path);

    auto isLoaded() const -> bool;
    auto loadFunction(const std::string &name) const -> void(*)();

private:
    void* mHandle;
};