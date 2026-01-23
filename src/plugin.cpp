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
    return "Plugin:\n Interface: " + interface_name + "\n Implementation: " + implementation_name + "\n Variants: " + available_variants + "\n";
}

const dyslang::SlangBinaryBlob* dyslang::Plugin::slang_module_blob() const
{
    return &slang_module;
}

// ----------

void dyslang::Plugin2::add_implementation(const std::string& source, const std::string& name)
{
    implementations.emplace_back(source, name);
}

void dyslang::Plugins::compose()
{
    _p = _p->compose();
}

void dyslang::Plugins::add_interface(const std::string& source, const std::string& name)
{
	interfaces[name] = Plugin2{._source = source, ._name = name, .implementations = std::vector<Implementation>() };
}

namespace
{
    void checkError(slang::IBlob* diagnosticsBlob)
    {
        if (diagnosticsBlob != nullptr)
        {
            printf("%s", static_cast<const char*>(diagnosticsBlob->getBufferPointer()));
        }
        diagnosticsBlob = nullptr;
    }

}

void dyslang::Plugins::prepare()
{
    // add sources
	for (auto& plugin : interfaces) {
        add_module(plugin.second._source);
        for (auto& impl : plugin.second.implementations) {
            add_module(impl._source);
        }
    }
    _p = _p->compose();

	// add type conformance
    for (auto& plugin : interfaces) {
        int64_t count = 0;
        for (auto& impl : plugin.second.implementations) {
            add_type_conformance(plugin.second._name, impl._name, count++);
        }
    }

    // inject code
    {
        std::string additional;
        additional.reserve(1028);
    	additional +=
R"([require(cpp)] bool operator==(NativeString left, NativeString right)
{
    __requirePrelude("#include <cstring>");
    __intrinsic_asm "strcmp($0, $1) == 0";
}
[require(cpp)] bool operator!=(NativeString left, NativeString right)
{
    return !(left == right);
}
[require(cpp)] void __copy_data_to_ptr<T>(Ptr<void> ptr, T data) {
    __intrinsic_asm "memcpy($0, &$1, sizeof($T1));";
}
)";

		for (auto& plugin : interfaces) {
		    additional += "import " + std::filesystem::path(plugin.second._source).stem().string() + ";\n";
		    for (auto& impl : plugin.second.implementations) {
	            additional += "import " + std::filesystem::path(impl._source).stem().string() + ";\n";
		    }
		}

        // create function
        additional += "export __extern_cpp void __create(void*, NativeString variant, void* out) {\n";
        additional += "    uint32_t* id = (uint32_t*)out;\n";
        for (auto& plugin : interfaces) {
            SlangInt count = 0;
            for (auto& impl : plugin.second.implementations) {
                additional += "    if(variant == \"" + impl._name + "\") { id[2] = " + std::to_string(count++) + "; __copy_data_to_ptr((void*)&id[4], " + impl._name + "()); }\n";
            }
        }
		
        additional += "}\n";

        additional += "export __extern_cpp void __traverse(void*, NativeString variant, void* in) {\n";
        for (auto& plugin : interfaces) {
            additional += "    if(variant == \"" + plugin.second._name + "\") ((" + plugin.second._name +"*)in)->traverse();\n";
            for (auto& impl : plugin.second.implementations) {
                additional += "    if(variant == \"" + impl._name + "\") ((" + impl._name + "*)in)->traverse();\n";
            }
        }
        additional += "}\n";

        // size function
        additional += "export __extern_cpp size_t __size_of(NativeString variant) {\n";
        for (auto& plugin : interfaces) {
            additional += "    if(variant == \"" + plugin.first + "\") return sizeof(" + plugin.first + ");\n";
            for (auto& impl : plugin.second.implementations) {
                additional += "    if(variant == \"" + impl._name + "\") return sizeof(" + impl._name + ");\n";
            }
        }
        additional += "    return 0;\n}\n";
        std::printf("%s\n", additional.c_str());

        add_module("cpp_only", "", additional);
    }
    _p = _p->compose();

    Slang::ComPtr<ISlangSharedLibrary> dylib;
    const SlangResult result = _p->components.back()->getTargetHostCallable(1, dylib.writeRef(), _p->diagnosticsBlob.writeRef());
    if (SLANG_FAILED(result)) throw std::runtime_error("slang: dll error");

    typedef void(*TouchFuncType)(IProperties*, const char*, void*);
    typedef size_t(*SizeOfFuncType)(const char*);

	std::array<uint32_t, 100> bytes = {};

	f_create = (TouchFuncType)dylib->findFuncByName("__create");
	f_create(nullptr, "Diffuse<float>", bytes.data());

	f_traverse = (TouchFuncType)dylib->findFuncByName("__traverse");
	f_traverse(nullptr, "IBsdf<float>", bytes.data());

    f_size_of = (SizeOfFuncType)dylib->findFuncByName("__size_of");
    size_t bsdf_size = f_size_of("IBsdf<float>");
    size_t diffuse_size = f_size_of("Diffuse<float>");
    size_t shape_size = f_size_of("IShape<float>");
    size_t cube_size = f_size_of("Cube<float>");
    return;
}
