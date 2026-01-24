#include <dyslang/plugin.h>
#include <string>
#include <functional>
#include <filesystem>
#include <stdexcept>

#include <dyslang/dyslang.h>

dyslang::CompPlugin::CompPlugin(const std::string_view lib_path) : lib{ dyslang::platform::SharedLib::open(lib_path.data()) }
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

std::string dyslang::CompPlugin::interface_variant_name(const char* variant) const
{
    return f_interface_variant_name(variant);
}

std::string dyslang::CompPlugin::implementation_variant_name(const char* variant) const
{
    return f_implementation_variant_name(variant);
}

std::string dyslang::CompPlugin::to_string() const {
    return "Plugin:\n Interface: " + interface_name + "\n Implementation: " + implementation_name + "\n Variants: " + available_variants + "\n";
}

const dyslang::SlangBinaryBlob* dyslang::CompPlugin::slang_module_blob() const
{
    return &slang_module;
}

// ----------

void dyslang::Plugin::add_implementation(const std::string& source, const std::string& name)
{
    implementations.emplace_back(source, name);
}

void dyslang::Plugins::compose()
{
    _p = _p->compose();
}

void dyslang::Plugins::add_interface(const std::string& source, const std::string& name)
{
	interfaces[name] = Plugin{._source = source, ._name = name, .implementations = std::vector<Implementation>() };
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
        std::string additional = R"tag(module cpp_only;
[require(cpp)] bool operator==(NativeString left, NativeString right)
{
    __requirePrelude("#include <cstring>");
    __intrinsic_asm R"(strcmp($0, $1) == 0)";
}
[require(cpp)] bool operator!=(NativeString left, NativeString right)
{
    return !(left == right);
}
[require(cpp)] void __copy_data_to_ptr<T>(Ptr<void> ptr, T data) {
    __intrinsic_asm R"(memcpy($0, &$1, sizeof($T1)))";
}

import dyslang;
namespace __private {
	[COM("A2F54866-7AEF-4905-B4CE-47-AC-73-CA-3C-07")]
    public interface IProperties {
        bool has(const NativeString);

#define dyslang_properties_get(TYPE) uint64_t get(const NativeString, TYPE **, size_t *dims /*3*/, int64_t *stride_in_bytes /*3*/);
        dyslang_properties_get(void)
        dyslang_properties_get(int32_t)
        dyslang_properties_get(uint32_t)
        dyslang_properties_get(int64_t)
        dyslang_properties_get(uint64_t)
        dyslang_properties_get(float)
        dyslang_properties_get(double)
#undef dyslang_properties_get

#define dyslang_properties_set(TYPE) void set(const NativeString, TYPE *ptr, size_t *dims /*3*/, int64_t *stride_in_bytes /*3*/, uint64_t total_size_in_bytes, uint64_t type);
        dyslang_properties_set(void)
        dyslang_properties_set(int32_t)
        dyslang_properties_set(uint32_t)
        dyslang_properties_set(int64_t)
        dyslang_properties_set(uint64_t)
        dyslang_properties_set(float)
        dyslang_properties_set(double)
#undef dyslang_properties_set
    };

	[require(cpp)]
    public T get<T>(NativeString key, IProperties properties) {
        __requirePrelude(R"(
                #include <type_traits>
                #include <stdexcept>
                #include <iostream>
                #include <array>
                #include <cstdio>

                template <typename T> struct is_vector : std::false_type {};
                template <typename T, size_t N> struct is_vector<Vector<T, N>> : std::true_type {};
                
                template <typename T> struct vector_info {
                    using type = void;
                    static constexpr size_t rows = 0;
                    static constexpr size_t cols = 0;
                    static constexpr size_t size = 0;
                };
                template <typename T, size_t N> struct vector_info<Vector<T, N>> {
                    using type = T;
                    static constexpr size_t rows = N;
                    static constexpr size_t cols = 0;
                    static constexpr size_t size = N;
                };
                template <typename T, size_t R, size_t C> struct vector_info<Matrix<T, R, C>> {
                    using type = T;
                    static constexpr size_t rows = R;
                    static constexpr size_t cols = C;
                    static constexpr size_t size = R * C;
                };

                template <typename T> inline constexpr bool is_arithmetic_v = std::is_floating_point_v<T> || std::is_integral_v<T>;

                struct DynamicArrayHelper { void* ptr; uint64_t count; };
                
                template <typename T, typename PROPERTIES_T> 
                T getProperty(const char* key, T dummy, PROPERTIES_T props)// require(is_arithmetic_v<std::remove_cv_t<std::remove_reference_t<T>>>)
                {
                    if constexpr (is_arithmetic_v<std::remove_cv_t<std::remove_reference_t<T>>>) {
                        T* value;
                        std::array<size_t, 3> dims = { 0, 0, 0 };
                        std::array<int64_t, 3> stride_in_bytes = { 0, 0, 0 };
                        props->get(key, &value, dims.data(), stride_in_bytes.data());
                        if (dims[0] != 0) std::cout << "Warning <dyslang>: \'" << key << "\' Property size mismatch" << std::endl;
                        return *value;
                    } 
                    else if constexpr (std::is_pointer_v<T>) {
                        void* value;
                        std::array<size_t, 3> dims = { 0, 0, 0 };
                        std::array<int64_t, 3> stride_in_bytes = { 0, 0, 0 };
                        props->get(key, &value, dims.data(), stride_in_bytes.data());
                        return (T)value;
                    } 
                    else if constexpr (sizeof(T) == 8) { // DescriptorHandle
                        uint32_t* value;
                        std::array<size_t, 3> dims = { 0, 0, 0 };
                        std::array<int64_t, 3> stride_in_bytes = { 0, 0, 0 };
                        props->get(key, &value, dims.data(), stride_in_bytes.data());
                        return *(T*)value;
                    } 
                    else if constexpr (sizeof(T) == 16) { // DynamicArray
                        using da_t = std::remove_pointer_t<decltype(dummy.data_0)>;
                        using value_t = vector_info<da_t>::type;
                        value_t* value;
                        std::array<size_t, 3> dims = { 0, 0, 0 };
                        std::array<int64_t, 3> stride_in_bytes = { 0, 0, 0 };
                        props->get(key, &value, dims.data(), stride_in_bytes.data());
                        DynamicArrayHelper result = { value, dims[0] };
                        return *reinterpret_cast<T*>(&result);
                    }
                    return T();
                }
                
                template <typename T, int N, typename PROPERTIES_T> 
                Vector<T, N> getProperty(const char* key, Vector<T, N> dummy, PROPERTIES_T props){	
                    Vector<T, N>* value;
                    std::array<size_t, 3> dims = { 0, 0, 0 };
                    std::array<int64_t, 3> stride_in_bytes = { 0, 0, 0 };
                    props->get(key, (T**)&value, dims.data(), stride_in_bytes.data());
                    if (dims[0] != N) std::cout << "Warning <dyslang>: \'" << key << "\' Property size mismatch" << std::endl;
                    return *value;
                }
                template <typename T, size_t N, typename PROPERTIES_T> 
                FixedArray<T, N> getProperty(const char* key, FixedArray<T, N> dummy, PROPERTIES_T& props){	
                    FixedArray<T, N>* value;
                    std::array<size_t, 3> dims = { 0, 0, 0 };
                    std::array<int64_t, 3> stride_in_bytes = { 0, 0, 0 };
                    props->get(key, (T**)&value, dims.data(), stride_in_bytes.data());
                    if (dims[0] != N) std::cout << "Warning <dyslang>: \'" << key << "\' Property size mismatch" << std::endl;
                    return *value;
                }
                template <typename T, int ROWS, int COLS, typename PROPERTIES_T> 
                Matrix<T, ROWS, COLS> getProperty(const char* key, Matrix<T, ROWS, COLS> dummy, PROPERTIES_T& props){	
                    Matrix<T, ROWS, COLS>* value;
                    std::array<size_t, 3> dims = { 0, 0, 0 };
                    std::array<int64_t, 3> stride_in_bytes = { 0, 0, 0 };
                    props->get(key, (T**)&value, dims.data(), stride_in_bytes.data());
                    if (dims[0] != ROWS || dims[1] != COLS) std::cout << "Warning <dyslang>: \'" << key << "\' Property size mismatch" << std::endl;
                    return *value;
                }

                template <typename T>
                struct CompileTimeInit {
                    static constexpr T value = T{};
                    static constexpr T get() { return value; }
                };
                template <typename T>
                struct CompileTimeInit<T*> {
                    static constexpr T storage = T{};
                    static constexpr const T* value = &storage;
                    static constexpr T* get() { return const_cast<T*>(value); }
                };
            )");
        __intrinsic_asm R"(getProperty($0, CompileTimeInit<$TR>::get(), $1))";
    }

    [require(cpp)]
    internal void set<T>(NativeString key, T value, IProperties properties) {
        __requirePrelude(R"(
                template <typename T, typename PROPERTIES_T> 
                void setProperty(const char* key, T* value, PROPERTIES_T& props){
                    using Type = std::remove_cv_t<std::remove_reference_t<T>>;
                    if constexpr (is_arithmetic_v<Type>) {
                        std::array<size_t, 3> dims = { 0, 0, 0 };
                        std::array<int64_t, 3> stride_in_bytes = { 0, 0, 0 };
                        props->set(key, value, dims.data(), stride_in_bytes.data(), sizeof(T), 0);
                    } 
                    else if constexpr (std::is_pointer_v<T>) { // Pointer
                        std::array<size_t, 3> dims = { 0, 0, 0 };
                        std::array<int64_t, 3> stride_in_bytes = { 0, 0, 0 };
                        props->set(key, (void*)*value, dims.data(), stride_in_bytes.data(), sizeof(std::remove_pointer_t<T>), 3);
                    }
                    else if constexpr (sizeof(T) == 8) { // DescriptorHandle
                        std::array<size_t, 3> dims = { 2, 0, 0 };
                        std::array<int64_t, 3> stride_in_bytes = { sizeof(uint32_t), 0, 0 };
                        uint32_t* ptr = (uint32_t*)value;
                        props->set(key, ptr, dims.data(), stride_in_bytes.data(), sizeof(T), 2);
                    }
                    else if constexpr (sizeof(T) == 16) { // DynamicArray
                        using da_t = std::remove_pointer_t<decltype(value->data_0)>;
                        using value_t = vector_info<da_t>::type;
                        value_t* ptr = (value_t*)value->data_0;
                        std::array<size_t, 3> dims = { value->count_0, vector_info<da_t>::rows, vector_info<da_t>::cols };
                        std::array<int64_t, 3> stride_in_bytes = { sizeof(value_t), 0, 0 };
                        props->set(key, ptr, dims.data(), stride_in_bytes.data(), value->count_0 * sizeof(da_t), 1);
                    }
                }
                template <typename T, int N, typename PROPERTIES_T>
                void setProperty(const char* key, Vector<T, N>* value, PROPERTIES_T& props){
                    std::array<size_t, 3> dims = { N, 0, 0 };
                    std::array<int64_t, 3> stride_in_bytes = { sizeof(T), 0, 0 };
                    props->set(key, (T*)value, dims.data(), stride_in_bytes.data(), sizeof(Vector<T, N>), 0);
                }
                template <typename T, size_t N, typename PROPERTIES_T> 
                void setProperty(const char* key, FixedArray<T, N>* value, PROPERTIES_T& props){
                    std::array<size_t, 3> dims = { N, 0, 0 };
                    std::array<int64_t, 3> stride_in_bytes = { sizeof(T), 0, 0 };
                    props->set(key, (T*)value, dims.data(), stride_in_bytes.data(), sizeof(FixedArray<T, N>), 0);
                }
                template <typename T, int ROWS, int COLS, typename PROPERTIES_T> 
                void setProperty(const char* key, Matrix<T, ROWS, COLS>* value, PROPERTIES_T& props){
                    std::array<size_t, 3> dims = { ROWS, COLS, 0 };
                    std::array<int64_t, 3> stride_in_bytes = { COLS * sizeof(T), sizeof(T), 0 };
                    props->set(key, (T*)value, dims.data(), stride_in_bytes.data(), sizeof(Matrix<T, ROWS, COLS>), 0);
                }
            )");
        __intrinsic_asm R"(setProperty($0, $1, $2))";
    }
}

internal struct Properties : dyslang::IProperties {
	private __private::IProperties __properties;
	internal __init(__private::IProperties p) { __properties = p; }
	internal bool has(NativeString key) {
	    __target_switch
	    {
	    case cpp: 
	        return (bool)__properties.has(key);
	    default: 
	        return false;
	    }
	}
	internal T get<T>(NativeString key) {
        __target_switch
        {
        case cpp: 
            return __private::get<T>(key, __properties);
        default: 
            return {};
        }
	}
	internal void set<T>(NativeString key, T value) {
		__target_switch
        {
        case cpp: 
            __private::set<T>(key, value, __properties);
		default:
			return;
        }
	}
};
)tag";

		for (auto& plugin : interfaces) {
		    additional += "import " + std::filesystem::path(plugin.second._source).stem().string() + ";\n";
		    for (auto& impl : plugin.second.implementations) {
	            additional += "import " + std::filesystem::path(impl._source).stem().string() + ";\n";
		    }
		}

        // create function
        additional += "export __extern_cpp void __create(__private::IProperties prop, NativeString variant, void* data) {\n";
        additional += "    uint32_t* id = (uint32_t*)data;\n";
        additional += "    id[0] = 1;\n";
        for (auto& plugin : interfaces) {
            SlangInt count = 0;
            for (auto& impl : plugin.second.implementations) {
                additional += "    if(variant == \"" + impl._name + "\") { id[2] = " + std::to_string(count++) + "; __copy_data_to_ptr((void*)&id[4], " + impl._name + "(Properties(prop))); }\n";
            }
        }
		
        additional += "}\n";

        additional += "export __extern_cpp void __traverse(__private::IProperties prop, NativeString variant, void* data) {\n";
        additional += "    uint32_t* id = (uint32_t*)data;\n";
        for (auto& plugin : interfaces) {
            //additional += "    if(variant == \"" + plugin.second._name + "\") ((" + plugin.second._name +"*)data)->traverse(Properties(prop));\n";
            for (auto& impl : plugin.second.implementations) {
                additional += "    if(variant == \"" + impl._name + "\") ((" + impl._name + "*)(void*)&id[4])->traverse(Properties(prop));\n";
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


	f_create = (TouchFuncType)dylib->findFuncByName("__create");
	f_traverse = (TouchFuncType)dylib->findFuncByName("__traverse");
    f_size_of = (SizeOfFuncType)dylib->findFuncByName("__size_of");


}
