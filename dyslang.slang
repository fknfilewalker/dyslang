module dyslang;

namespace dyslang
{
    [COM("A2F54866-7AEF-4905-B4CE-47-AC-73-CA-3C-07")]
    internal interface IProperties {
        bool has(const NativeString);

#define dyslang_properties_get(TYPE) uint64_t get(const NativeString, TYPE**, size_t* dims /*3*/, int64_t* stride_in_bytes /*3*/);
	    dyslang_properties_get(void)
		dyslang_properties_get(int32_t)
        dyslang_properties_get(uint32_t)
        dyslang_properties_get(int64_t)
        dyslang_properties_get(uint64_t)
        dyslang_properties_get(float)
        dyslang_properties_get(double)
#undef dyslang_properties_get

#define dyslang_properties_set(TYPE) void set(const NativeString, TYPE* ptr, size_t* dims /*3*/, int64_t* stride_in_bytes /*3*/, uint64_t total_size_in_bytes, uint64_t type);
		dyslang_properties_set(void)
        dyslang_properties_set(int32_t)
        dyslang_properties_set(uint32_t)
        dyslang_properties_set(int64_t)
        dyslang_properties_set(uint64_t)
        dyslang_properties_set(float)
        dyslang_properties_set(double)
#undef dyslang_properties_set
	};

	public struct DynamicArray<T> {
        public T* data;
        public uint64_t count;
    };

    public struct _DescriptorHandle<T:IOpaqueDescriptor> {
        public vector<uint32_t, 2> id;
        public DescriptorHandle<T> get() {
            return {id};
        }
    };

    namespace __private {
        [require(cpp)]
        T get<T>(NativeString key, IProperties properties) {
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
        void set<T>(NativeString key, T value, IProperties properties) {
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

    public struct Properties {
        private IProperties __properties;
        __init(IProperties properties){
            __properties = properties;
        }
        public bool has(NativeString key) {

            __target_switch
            {
            case cpp: 
                return (bool)__properties.has(key);
            default: 
                return false;
            }
        }

        public T get<T>(NativeString key) {
            __target_switch
            {
            case cpp: 
                return __private::get<T>(key, __properties);
            default: 
                return {};
            }
        }

        public void set<T>(NativeString key, T value) {
            __target_switch
            {
            case cpp: 
                __private::set<T>(key, value, __properties);
            }
        }
    };
}

public typealias Properties = dyslang::Properties;
public typealias DynamicArray<T> = dyslang::DynamicArray<T>;
public typealias _DescriptorHandle<T:IOpaqueDescriptor> = dyslang::_DescriptorHandle<T>;
