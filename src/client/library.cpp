#include "library.h"

#include <dlfcn.h>

DynamicLibrary::DynamicLibrary(const std::string &path) :
    mHandle(dlopen(path.c_str(), RTLD_LAZY)) {}

bool DynamicLibrary::isLoaded() const {
    return mHandle != nullptr;
}
