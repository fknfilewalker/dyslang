#pragma once
#include <string>
#include <functional>
#include <filesystem>
#include <dyslang/platform.h>

namespace dyslang
{
    struct IProperties;
    struct Properties;
    struct Plugin;
    template <typename T> struct Object;
    struct ObjectData;

    struct Plugin
    {
        explicit Plugin(std::string_view lib_path);

        template <typename T>
        std::unique_ptr<Object<T>> create(Properties& props, const char* variant) {
            auto obj = std::make_unique<Object<T>>(*this, variant);
            f_create_object(&props, variant, obj->data.get_data_ptr());
            obj->interface_name = f_interface_variant_name(variant);
            obj->implementation_name = f_implementation_variant_name(variant);
            return obj;
        }

        [[nodiscard]] std::string interface_variant_name(const char* variant) const;
        [[nodiscard]] std::string implementation_variant_name(const char* variant) const;
        [[nodiscard]] std::string to_string() const;

        platform::SharedLib lib;
        std::string interface_name;
        std::string implementation_name;
        std::string available_variants;
        std::vector<uint8_t> slang_module;

        std::function<const char*()> f_interface_name;
        std::function<const char*()> f_implementation_name;
        std::function<const char*()> f_available_variants;
        std::function<const char* (const char*)> f_interface_variant_name;
        std::function<const char* (const char*)> f_implementation_variant_name;
        std::function<unsigned int (const char*)> f_variant_size;
        std::function<void(IProperties*, const char*, void*)> f_create_object;
        std::function<void(IProperties*, const char*, void*)> f_traverse;
        std::function<size_t()> f_slang_module_ir_size;
        std::function<uint8_t*()> f_slang_module_ir_data_ptr;
    };

    struct ObjectData {
        static constexpr size_t virtual_table_offset = 4 * sizeof(uint32_t);
        explicit ObjectData(const size_t data_size) : data(data_size + virtual_table_offset) {}
        uint8_t* get_virtual_table_ptr() { return data.data(); }
        uint8_t* get_data_ptr() { return data.data() + virtual_table_offset; }
        [[nodiscard]] static size_t get_virtual_table_size() { return virtual_table_offset; }
        [[nodiscard]] size_t get_data_size() const { return data.size() - virtual_table_offset; }
        [[nodiscard]] size_t get_size() const { return data.size(); }
        // todo check if correct offset
        void set_type_conformance_index(const uint32_t index) { *reinterpret_cast<uint32_t*>(data.data() + 3 * sizeof(uint32_t)) = index; }
		[[nodiscard]] uint32_t get_type_conformance_index() const { return *reinterpret_cast<const uint32_t*>(data.data() + 3 * sizeof(uint32_t)); }

        std::vector<uint8_t> data;
    };

    template <typename T>
    struct Object
    {
        explicit Object(
            Plugin& plugin,
            const char* variant) : plugin{ plugin }, variant{ variant }, data(plugin.f_variant_size(variant)) {}

        //T* get() { return reinterpret_cast<T*>(data.data()); }
        //T* operator->() { return reinterpret_cast<T*>(data.data()); }

        void traverse(Properties& props) {
            plugin.f_traverse((IProperties*)&props, variant, data.get_data_ptr());
        }

        [[nodiscard]] std::string to_string() const {
            return "Object:\n Interface: " + interface_name + "\n Implementation: " + implementation_name + "\n Variant: " + variant + "\n VTable Size: " + std::to_string(data.get_virtual_table_size()) + " Bytes\n Data Size: " + std::to_string(data.get_data_size()) + " Bytes\n Total Size: " + std::to_string(data.get_size()) + " Bytes";
        }

        Plugin& plugin;
        const char* variant;
        std::string interface_name;
        std::string implementation_name;
        ObjectData data;
    };
}
