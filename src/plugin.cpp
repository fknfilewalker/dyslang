#include <dyslang/plugin.h>
#include <string>
#include <functional>
#include <filesystem>
#include <stdexcept>

dyslang::Plugin::Plugin(const std::string_view lib_path) : lib{ dyslang::platform::SharedLib::open(lib_path.data()) }
{
    if (!lib.valid()) {
        throw std::runtime_error("Failed to load plugin");
    }

    f_interface_name = lib.loadFunction<const char*>("__interface_name");
    f_implementation_name = lib.loadFunction<const char*>("__implementation_name");
    f_interface_variant_name = lib.loadFunction<const char*, const char*>("__interface_variant_name");
    f_available_variants = lib.loadFunction<const char*>("__available_variants");
    f_implementation_variant_name = lib.loadFunction<const char*, const char*>("__implementation_variant_name");
    f_variant_size = lib.loadFunction<unsigned int, const char*>("__implementation_size");
    f_create_object = lib.loadFunction<void, dyslang::IProperties*, const char*, void*>("__create_object");
    f_traverse = lib.loadFunction<void, dyslang::IProperties*, const char*, void*>("__traverse");
    f_slang_module_ir_size = lib.loadFunction<size_t>("__slang_module_ir_size");
    f_slang_module_ir_data_ptr = lib.loadFunction<uint8_t*>("__slang_module_ir_data_ptr");

    if (!f_interface_name || !f_implementation_name || !f_interface_variant_name || !f_implementation_variant_name || !f_variant_size || !f_create_object || !f_traverse || !f_slang_module_ir_size || !f_slang_module_ir_data_ptr) {
        throw std::runtime_error("Failed to load plugin");
    }

    interface_name = f_interface_name();
    implementation_name = f_implementation_name();
    available_variants = f_available_variants();

    // load external slang module file
    // std::string slang_path = dyslang::platform::executable_path + "/" + std::string{ lib_path } + ".slang-module";
    // std::cout << slang_path << '\n';
    // std::ifstream input(slang_path, std::ios::binary);
    // slang_module = std::vector<uint8_t>(std::istreambuf_iterator<char>(input), {});

	// load internal slang module
    // auto slang_module_size = f_slang_module_ir_size();
    // auto slang_module_ptr = f_slang_module_ir_data_ptr();
    // slang_module = std::vector<uint8_t>(slang_module_ptr, slang_module_ptr + slang_module_size);

	// direct load slang module from dynamic library
	slang_module = SlangBinaryBlob{ f_slang_module_ir_data_ptr(), f_slang_module_ir_size() };
}

std::string dyslang::Plugin::interface_variant_name(const char* variant) const
{
    return f_interface_variant_name(variant);
}

std::string dyslang::Plugin::implementation_variant_name(const char* variant) const
{
    return f_implementation_variant_name(variant);
}

std::string dyslang::Plugin::to_string() const {
    return "Plugin:\n Interface: " + interface_name + "\n Implementation: " + implementation_name + "\n";
}

const dyslang::SlangBinaryBlob* dyslang::Plugin::slang_module_blob() const
{
    return &slang_module;
}
