#include "library.h"

#include <dlfcn.h>

DynamicLibrary::DynamicLibrary(const std::string &path) :
    mHandle(dlopen(path.c_str(), RTLD_LAZY)) {}

auto DynamicLibrary::isLoaded() const -> bool {
    return mHandle != nullptr;
}
