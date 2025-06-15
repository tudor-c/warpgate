#include "library.h"

#include <dlfcn.h>

#include "log.h"

DynamicLibrary::DynamicLibrary(const std::string &path) {
    mHandle = dlopen(path.c_str(), RTLD_LAZY);
    if (mHandle != nullptr) {
        lg::debug("Successfully loaded {}", path);
    }
    else {
        const char* error = dlerror();
        lg::error("Failed to load dynamic library {}", path);
        throw std::runtime_error(error);
    }
}

auto DynamicLibrary::isLoaded() const -> bool {
    return mHandle != nullptr;
}

auto DynamicLibrary::loadFunction(const std::string &name) const -> void(*)() {
    if (!isLoaded()) {
        return nullptr;
    }
    return reinterpret_cast<void(*)()>(dlsym(mHandle, name.c_str()));
}

