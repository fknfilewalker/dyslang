#pragma once
#include <functional>
#include <string>
#include <filesystem>
#ifdef _WIN32
#include <Shlwapi.h>
#elif defined( __linux__ )
#include <linux/limits.h>
#include <unistd.h>
#include <dlfcn.h>
#elif defined(__APPLE__)
#include <dlfcn.h>
#include <mach-o/dyld.h>
#endif

namespace dyslang::platform
{
    static std::filesystem::path& executable_filepath()
    {
#ifdef _WIN32
        char carr[MAX_PATH];
        GetModuleFileName(nullptr, carr, MAX_PATH);
        static std::filesystem::path path{ carr };
#elif defined(__linux__)
        char carr[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", carr, sizeof(carr) - 1);
        if (len != -1) carr[len] = '\0';
        static std::filesystem::path path{ carr };
#elif defined(__APPLE__)
        char carr [PATH_MAX];
        uint32_t bufsize = PATH_MAX;
        if(!_NSGetExecutablePath(carr, &bufsize)) puts(carr);
        static std::filesystem::path path{ carr };
#endif
        return path;
    }
}

namespace dyslang::platform
{
    constexpr bool isDebug =
#if !defined( NDEBUG ) || defined( _DEBUG )
        true
#else 
        false
#endif
        ;

    constexpr enum class OS { Windows, Linux, MacOS } os =
#ifdef _WIN32
        OS::Windows
#elif __linux__
        OS::Linux
#elif __APPLE__
        OS::MacOS
#endif
        ;
    constexpr bool isWindows = os == OS::Windows;
    constexpr bool isLinux = os == OS::Linux;
    constexpr bool isMacOS = os == OS::MacOS;
    constexpr bool isApple = isMacOS;

#ifdef _WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif

    struct SharedLib
    {
#ifdef _WIN32
        using Handle = HMODULE;
#else
		using Handle = void*;
#endif

        explicit SharedLib(const Handle handle = {}) : _handle{ handle } {}

		// move constructor
		SharedLib(SharedLib&& other) noexcept : _handle{ other._handle }
        {
        	other._handle = nullptr;
        }
        SharedLib& operator=(SharedLib&& other) noexcept
        {
            _handle = other._handle;
            other._handle = nullptr;
	        return *this;
        }

        static SharedLib open(const char* name)
        {
            std::string path{ name };
#ifdef _WIN32
            path += ".dll";
#elif defined(__linux__)
            path += ".so";
#elif defined(__APPLE__)
            path += ".dylib";
#endif

#ifdef _WIN32
            return SharedLib{ LoadLibrary(path.c_str()) };
#elif defined(__linux__) || defined(__APPLE__)
            return SharedLib{ dlopen(path.c_str(), RTLD_LAZY) };
#else
            return SharedLib();
#endif
        }

        ~SharedLib() {
            if (valid()) {
#ifdef _WIN32
                FreeLibrary(_handle);// > 0;
#elif defined(__linux__) || defined(__APPLE__)
                dlclose(_handle);
#endif
            }
        }

        template <typename T>
        T loadVariable(const char* name)
        {
#ifdef _WIN32
            return reinterpret_cast<T>(GetProcAddress(_handle, name));
#elif defined(__linux__) || defined(__APPLE__)
            return reinterpret_cast<T>(dlsym(_handle, name));
#endif  
        }

        template <typename R, typename ...Args>
    	std::function<R(Args...)> loadFunction(const char* name)
        {
            typedef R(STDCALL* FuncPtr)(Args...);
#ifdef _WIN32
            //void* p = GetProcAddress(_handle, name);
            FuncPtr ptr = reinterpret_cast<FuncPtr>(GetProcAddress(_handle, name));
#elif defined(__linux__) || defined(__APPLE__)
            FuncPtr ptr = (FuncPtr)dlsym(_handle, name);
#endif
            return ptr;
        }

        [[nodiscard]] bool valid() const
		{
			return _handle != nullptr;
		}

        [[nodiscard]] Handle handle() const
        {
            return _handle;
        }
    private:
        Handle _handle;
    };
}
