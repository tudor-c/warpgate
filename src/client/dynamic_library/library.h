#pragma once
#include <dlfcn.h>
#include <string>

class DynamicLibrary {
public:
    explicit DynamicLibrary(const std::string& path);

    auto isLoaded() const -> bool;
    auto loadFunction(const std::string &name) const -> std::string(*)(std::vector<std::string>);

private:
    void* mHandle;
};