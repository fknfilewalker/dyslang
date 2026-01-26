#pragma once
#include <string>
#include <functional>
#include <filesystem>
#include <slang-com-ptr.h>
#include <slang.h>
#include <dyslang/platform.h>
#include <dyslang/slangc.h>

namespace dyslang
{
	struct IProperties;
    struct Properties;
    struct CompPlugin;
    template <typename T> struct CompObject;
    struct ObjectData;

    class SlangBinaryBlob : public ISlangBlob
    {
    public:
        SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE { return SLANG_E_NOT_IMPLEMENTED; }
        SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
        SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }
        // ISlangBlob
        SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() SLANG_OVERRIDE { return _ptr; }
        SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() SLANG_OVERRIDE { return _len; }

        SlangBinaryBlob(const uint8_t* ptr, const size_t len) : _ptr{ ptr }, _len{ len } {}
        SlangBinaryBlob() = default;
    protected:
		const uint8_t* _ptr;
		size_t _len;
    };

    struct CompPlugin
    {
        explicit CompPlugin(std::string_view lib_path);

        template <typename T = void>
        std::unique_ptr<CompObject<T>> create(Properties& props, const char* variant) {
            auto obj = std::make_unique<CompObject<T>>(*this, variant);
            f_create_object(&props, variant, obj->data.get_data_ptr());
            obj->interface_name = f_interface_variant_name(variant);
            obj->implementation_name = f_implementation_variant_name(variant);
            return obj;
        }

        [[nodiscard]] std::string interface_variant_name(const char* variant) const;
        [[nodiscard]] std::string implementation_variant_name(const char* variant) const;
        [[nodiscard]] std::string to_string() const;
        [[nodiscard]] const SlangBinaryBlob* slang_module_blob() const;

        platform::SharedLib lib;
        std::string interface_name;
        std::string implementation_name;
        std::string available_variants;
        SlangBinaryBlob slang_module;

		// functions loaded from dylib
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
        static constexpr size_t rtti_header_size = 4u * sizeof(uint32_t);
        explicit ObjectData(const size_t data_size) : data(data_size + rtti_header_size) {}
        uint8_t* get_rtti_header_ptr() { return data.data(); }
        uint8_t* get_data_ptr() { return data.data() + rtti_header_size; }
        [[nodiscard]] size_t get_data_size() const { return data.size() - rtti_header_size; }
        [[nodiscard]] size_t get_size() const { return data.size(); }
        // from slang/slang-session.cpp
        // Slang RTTI header format:
        // byte 0-7: pointer to RTTI struct describing the type. (not used for now, set to 1 for valid
        // types, and 0 to represent null).
        // byte 8-11: 32-bit sequential ID of the type conformance witness.
        // byte 12-15: unused.
        void set_type_conformance_id(const uint32_t id) {
            *reinterpret_cast<uint32_t*>(data.data() + 0u * sizeof(uint32_t)) = 1;
            *reinterpret_cast<uint32_t*>(data.data() + 2u * sizeof(uint32_t)) = id;
        }
        [[nodiscard]] uint32_t get_type_conformance_id() const { return *reinterpret_cast<const uint32_t*>(data.data() + 2u * sizeof(uint32_t)); }

        size_t find_address_offset(const void* address) const {
            auto ptr = static_cast<const uint8_t*>(address);
            auto base = data.data();
            if (ptr < base || ptr >= base + data.size()) return SIZE_MAX;
            return static_cast<size_t>(ptr - base);
        }

        void write_data(const size_t offset, const auto& src, const size_t size) {
            std::memcpy(data.data() + offset, &src, size);
		}
		void write_ptr_to(const size_t offset, auto& d) {
			void* ptr = &d;
            std::memcpy(data.data() + offset, &ptr, sizeof(void*));
        }

        std::vector<uint8_t> data;
    };

    struct Object
    {
        std::string implementation_name;
        std::string interface_name;
        ObjectData data;
    };

    struct Implementation
    {
        std::string source, name;
    };
    struct Plugin
    {
        std::string source, name;
        std::vector<Implementation> implementations;
    };

    struct Plugins : dyslang::Slangc
    {
		Plugins(const std::vector<const char*>& includes, const std::vector<ArgPair>& defines) : Slangc(includes, defines) {}

        void add_interface(const std::string& source, const std::string& name, const std::vector<Implementation>& implementations);
        void prepare();
        void compose();

        Object create(const std::string& implementation_name, const std::string& interface_name, dyslang::Properties& props) const
		{
            Object o = { implementation_name, interface_name, ObjectData(f_size_of(interface_name.c_str()) - ObjectData::rtti_header_size)};
			f_create((IProperties*)&props, implementation_name.c_str(), o.data.data.data());
            return o;
		}
        void traverse(Object& object, Properties& props) const
        {
			f_traverse((IProperties*)&props, object.implementation_name.c_str(), object.data.data.data());
        }

        std::function<void(IProperties*, const char*, void*)> f_create;
        std::function<void(IProperties*, const char*, void*)> f_traverse;
        std::function<size_t(const char*)> f_size_of;

        std::unordered_map<std::string, Plugin> interfaces;
    };

    template <typename T>
    struct CompObject
    {
        explicit CompObject(
            CompPlugin& plugin,
            const char* variant) : plugin{ plugin }, variant{ variant }, data(plugin.f_variant_size(variant)) {}

        //T* get() { return reinterpret_cast<T*>(data.data()); }
        //T* operator->() { return reinterpret_cast<T*>(data.data()); }

        void traverse(Properties& props) {
            plugin.f_traverse(reinterpret_cast<IProperties*>(&props), variant, data.get_data_ptr());
        }

        [[nodiscard]] std::string to_string() const {
            return "Object:\n Interface: " + interface_name + "\n Implementation: " + implementation_name + "\n Variant: " + variant + "\n Conformance ID: " + std::to_string(data.get_type_conformance_id()) + "\n RTTI Header Size: " + std::to_string(data.rtti_header_size) + " Bytes\n Data Size: " + std::to_string(data.get_data_size()) + " Bytes\n Total Size: " + std::to_string(data.get_size()) + " Bytes\n";
        }

        CompPlugin& plugin;
        const char* variant;
        std::string interface_name;
        std::string implementation_name;
        ObjectData data;
    };
}
