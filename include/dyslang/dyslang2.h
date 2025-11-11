#pragma once
#include <map>
#include <stdexcept>
#include <string>
#include <array>
#include <variant>
#include <vector>
#include <functional>
#include <span>
#include <any>
#include <optional>
#include <cstring>
#include <slang.h>
#include <slang-com-ptr.h>
#include <slang-com-helper.h>

namespace dyslang2
{
    template <typename T>
    concept integral = std::integral<T>;
    template <typename T>
    concept floating_point = std::floating_point<T>;
    template <typename T>
    concept arithmetic = std::integral<T> || std::floating_point<T>;
    template <typename T>
    inline constexpr bool is_arithmetic_v = std::is_floating_point_v<T> || std::is_integral_v<T>;

    using b32 = uint32_t;

    class IProperties : public ISlangUnknown
    {
        SLANG_COM_INTERFACE(
            0xA2F54866,
            0x7AEF,
            0x4905,
            {0xB4, 0xCE, 0x47, 0xAC, 0x73, 0xCA, 0x3C, 0x07})

        virtual b32 has_property(const char *) = 0;
        virtual void set(const char *, void *, uint64_t) = 0;
        virtual void *get(const char *, uint64_t *) = 0;
    };

    struct Parameters
    {
        std::any values;
        std::vector<size_t> dimension;
        // std::vector<int64_t> stride_in_bytes;
    };
    struct Properties : public IProperties
    {
        SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const &uuid, void **outObject) SLANG_OVERRIDE { return SLANG_E_NOT_IMPLEMENTED; }
        SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
        SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }

        b32 SLANG_MCALL has_property(const char *key) SLANG_OVERRIDE
        {
            return properties.contains(key);
        }

        void SLANG_MCALL set(const char *key, void *ptr, const uint64_t count) SLANG_OVERRIDE
        {
            properties[key] = Parameters{.values = *static_cast<std::any *>(ptr), .dimension = {count}};
        }

        void *SLANG_MCALL get(const char *key, uint64_t *count) SLANG_OVERRIDE
        {
            if (!properties.contains(key))
                throw std::runtime_error("Property not found: " + std::string(key));
            auto &value = properties[key];
            *count = properties[key].dimension[0];
            return &properties[key].values;
        }

        template <typename T>
        T get(const char* key) requires(std::is_trivial_v<T>)
        {
            uint64_t c;
            std::any& a = *static_cast<std::any*>(get(key, &c));
            assert(c == 1);
            return *std::any_cast<T*>(a);
        }
        template<typename T>
        T get(const char* key)
        {
            uint64_t c;
            std::any& a = *static_cast<std::any*>(get(key, &c));
            assert(c != 1);
            return T( std::any_cast<typename T::value_type*>(a), std::any_cast<typename T::value_type*>(a) + c );
        }

        template <typename T>
        Properties& set(const char *key, T *data)
        {
            std::any a = data;
            set(key, &a, 1);
            return *this;
        }
		template <typename T>
        Properties& set(const char *key, std::vector<T>* data)
		{
			std::any a = data->data();
			set(key, &a, data->size());
            return *this;
		}
        template <typename T, size_t N>
        Properties& set(const char* key, std::array<T, N>* data)
        {
            std::any a = data->data();
            set(key, &a, N);
            return *this;
        }
        // template <typename T, size_t N>
        // void set(const char *key, const std::array<T, N> &data) { set(key, data.data(), N); }
        //  template <arithmetic T, std::size_t M, std::size_t N> void set(const char* key, const dyslang::matrix<T, M, N>& data) { set(key, data.data(), M * N); }

        /*template <typename T>
        T find(const char* key) {
            if(properties.contains(key)) {
                try {
                    return std::get<T>(properties[key]);
                } catch (std::bad_variant_access& e) {
                    std::cout << "Bad variant access: " << e.what() << '\n';
                }
            }
            return T{};
        }*/

        [[nodiscard]] std::string to_string() const
        {
            std::string result = "Properties:\n";
            for (const auto &[key, value] : properties)
            {
                result += " ";
                result += key;
                result += ": ";

                std::string sep;
                for (uint64_t i = 0; i < value.dimension[0]; i++)
                {
                    result += sep;
                    if (value.values.type() == typeid(float *))
                        result += std::to_string(std::any_cast<float *>(value.values)[i]);
                    if (value.values.type() == typeid(int *))
                        result += std::to_string(std::any_cast<int *>(value.values)[i]);
                    if (value.values.type() == typeid(uint32_t*))
                        result += std::to_string(std::any_cast<uint32_t*>(value.values)[i]);

                    sep = ", ";
                }
                result += "\n";
            }
            return result;
        }
        Properties& print()
        {
            std::printf("%s\n", to_string().c_str());
            return *this;
        }

        struct cmp_str
        {
            bool operator()(char const *a, char const *b) const { return std::strcmp(a, b) < 0; }
        };
        std::map<const char *, Parameters, cmp_str> properties;
    };

    struct DynamicObject
    {
        void traverse(IProperties&);
        Properties traverse();

        template <typename R, typename ...Args>
        std::function<R(Args...)> loadFunction(const char* name)
        {
            typedef R(*FuncPtr)(Args...);
            return reinterpret_cast<FuncPtr>(_library->findFuncByName(name));
        }

        std::vector<uint8_t> data;
        std::function<void(uint8_t*, IProperties *)> traverseFunc;
        Slang::ComPtr<ISlangSharedLibrary> _library;
    };

    struct DynamicClass
    {
        DynamicClass(std::string filepath, std::string classname, std::string interfacename = "");
        DynamicObject init(IProperties&);
        size_t size() const;
        bool has_interface() const;

        std::string _filepath, _classname, _interfacename;
        std::optional<uint32_t> _id;
        std::function<uint32_t(void)> _sizeFunc, _dynamicSizeFunc;
        std::function<void(uint8_t*, IProperties *)> _traverseFunc;
        Slang::ComPtr<ISlangSharedLibrary> _library;
        Slang::ComPtr<slang::IModule> _module;
    };
}
