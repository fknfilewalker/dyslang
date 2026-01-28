#pragma once
// source https://github.com/Erlkoenig90/map-macro/tree/master
#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(__VA_ARGS__)))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL3(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL4(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL5(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))

#ifdef _MSC_VER
// MSVC needs more evaluations
#define EVAL6(...) EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
#define EVAL(...)  EVAL6(EVAL6(__VA_ARGS__))
#else
#define EVAL(...)  EVAL5(__VA_ARGS__)
#endif

#define MAP_END(...)
#define MAP_OUT

#define EMPTY() 
#define DEFER(id) id EMPTY()

#define MAP_GET_END2() 0, MAP_END
#define MAP_GET_END1(...) MAP_GET_END2
#define MAP_GET_END(...) MAP_GET_END1
#define MAP_NEXT0(test, next, ...) next MAP_OUT
#define MAP_NEXT1(test, next) DEFER ( MAP_NEXT0 ) ( test, next, 0)
#define MAP_NEXT(test, next)  MAP_NEXT1(MAP_GET_END test, next)
#define MAP_INC(X) MAP_INC_ ## X

#define MAP0(f, x, peek, ...) f(x) DEFER ( MAP_NEXT(peek, MAP1) ) ( f, peek, __VA_ARGS__ ) 
#define MAP1(f, x, peek, ...) f(x) DEFER ( MAP_NEXT(peek, MAP0) ) ( f, peek, __VA_ARGS__ )

#define MAP0_UD(f, userdata, x, peek, ...) f(x,userdata) DEFER ( MAP_NEXT(peek, MAP1_UD) ) ( f, userdata, peek, __VA_ARGS__ ) 
#define MAP1_UD(f, userdata, x, peek, ...) f(x,userdata) DEFER ( MAP_NEXT(peek, MAP0_UD) ) ( f, userdata, peek, __VA_ARGS__ ) 

#define MAP0_UD_I(f, userdata, index, x, peek, ...) f(x,userdata,index) DEFER ( MAP_NEXT(peek, MAP1_UD_I) ) ( f, userdata, MAP_INC(index), peek, __VA_ARGS__ ) 
#define MAP1_UD_I(f, userdata, index, x, peek, ...) f(x,userdata,index) DEFER ( MAP_NEXT(peek, MAP0_UD_I) ) ( f, userdata, MAP_INC(index), peek, __VA_ARGS__ ) 

#define MAP_LIST0(f, x, peek, ...) , f(x) DEFER ( MAP_NEXT(peek, MAP_LIST1) ) ( f, peek, __VA_ARGS__ ) 
#define MAP_LIST1(f, x, peek, ...) , f(x) DEFER ( MAP_NEXT(peek, MAP_LIST0) ) ( f, peek, __VA_ARGS__ ) 
#define MAP_LIST2(f, x, peek, ...)   f(x) DEFER ( MAP_NEXT(peek, MAP_LIST1) ) ( f, peek, __VA_ARGS__ ) 

#define MAP_LIST0_UD(f, userdata, x, peek, ...) , f(x, userdata) DEFER ( MAP_NEXT(peek, MAP_LIST1_UD) ) ( f, userdata, peek, __VA_ARGS__ ) 
#define MAP_LIST1_UD(f, userdata, x, peek, ...) , f(x, userdata) DEFER ( MAP_NEXT(peek, MAP_LIST0_UD) ) ( f, userdata, peek, __VA_ARGS__ ) 
#define MAP_LIST2_UD(f, userdata, x, peek, ...)   f(x, userdata) DEFER ( MAP_NEXT(peek, MAP_LIST1_UD) ) ( f, userdata, peek, __VA_ARGS__ ) 

#define MAP_LIST0_UD_I(f, userdata, index, x, peek, ...) , f(x, userdata, index) DEFER ( MAP_NEXT(peek, MAP_LIST1_UD_I) ) ( f, userdata, MAP_INC(index), peek, __VA_ARGS__ ) 
#define MAP_LIST1_UD_I(f, userdata, index, x, peek, ...) , f(x, userdata, index) DEFER ( MAP_NEXT(peek, MAP_LIST0_UD_I) ) ( f, userdata, MAP_INC(index), peek, __VA_ARGS__ ) 
#define MAP_LIST2_UD_I(f, userdata, index, x, peek, ...)   f(x, userdata, index) DEFER ( MAP_NEXT(peek, MAP_LIST0_UD_I) ) ( f, userdata, MAP_INC(index), peek, __VA_ARGS__ ) 

/**
 * Applies the function macro `f` to each of the remaining parameters.
 */
#define MAP(f, ...) EVAL(MAP1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

 /**
 * Applies the function macro `f` to each of the remaining parameters and
 * inserts commas between the results.
 */
#define MAP_LIST(f, ...) EVAL(MAP_LIST2(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

 /**
 * Applies the function macro `f` to each of the remaining parameters and passes userdata as the second parameter to each invocation,
 * e.g. MAP_UD(f, x, a, b, c) evaluates to f(a, x) f(b, x) f(c, x)
 */
#define MAP_UD(f, userdata, ...) EVAL(MAP1_UD(f, userdata, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

 /**
 * Applies the function macro `f` to each of the remaining parameters, inserts commas between the results,
 * and passes userdata as the second parameter to each invocation,
 * e.g. MAP_LIST_UD(f, x, a, b, c) evaluates to f(a, x), f(b, x), f(c, x)
 */
#define MAP_LIST_UD(f, userdata, ...) EVAL(MAP_LIST2_UD(f, userdata, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

 /**
  * Applies the function macro `f` to each of the remaining parameters, passes userdata as the second parameter to each invocation,
  * and the index of the invocation as the third parameter,
  * e.g. MAP_UD_I(f, x, a, b, c) evaluates to f(a, x, 0) f(b, x, 1) f(c, x, 2)
  */
#define MAP_UD_I(f, userdata, ...) EVAL(MAP1_UD_I(f, userdata, 0, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

  /**
   * Applies the function macro `f` to each of the remaining parameters, inserts commas between the results,
   * passes userdata as the second parameter to each invocation, and the index of the invocation as the third parameter,
   * e.g. MAP_LIST_UD_I(f, x, a, b, c) evaluates to f(a, x, 0), f(b, x, 1), f(c, x, 2)
   */
#define MAP_LIST_UD_I(f, userdata, ...) EVAL(MAP_LIST2_UD_I(f, userdata, 0, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// helper
#define JOIN2(a, b) a b
#define UNPACK2(a, b) a, b

// for zipping variants
#define ZIP(PAIR, X) (UNPACK2 PAIR, UNPACK2 X)
#define ZIP_PAIR(X,...) MAP_LIST_UD(ZIP, X, __VA_ARGS__)
#define VARIANTS_ZIP(NAME, TRANSFORMATION, ...) ZIP_PAIR((NAME, TRANSFORMATION), __VA_ARGS__)

// generating return variants
#define ADD_IF_RETURN(VARIANT_STRING, VARIANT_TYPE, NAME, TRANSFORMATION) if (variant == #VARIANT_STRING) return TRANSFORMATION(NAME, VARIANT_TYPE);
#define ADD_IF_RETURN_PAIR(pair) ADD_IF_RETURN pair
#define RETURN_VARIANTS(...) MAP(ADD_IF_RETURN_PAIR, __VA_ARGS__)
#define IMPLEMENT_VARIANTS(NAME, TRANSFORMATION, ...) RETURN_VARIANTS(VARIANTS_ZIP(NAME, TRANSFORMATION, __VA_ARGS__))

// get variant list
#define GET_VARIANT(VARIANT, IMPL) #VARIANT ":" #IMPL ","
#define GET_VARIANT_PAIR(X) GET_VARIANT X
#define GET_VARIANT_STRING(...) MAP(GET_VARIANT_PAIR, __VA_ARGS__)

#define UNWRAP_VARIANT(...) __VA_ARGS__
#define UNWRAP_VARIANT_STRING(...) #__VA_ARGS__
// args (... are the template types)
#define IDENTITY(NAME, ...) UNWRAP_VARIANT __VA_ARGS__
#define STRINGIFY(NAME, ...) #NAME UNWRAP_VARIANT_STRING __VA_ARGS__
#define SIZE_OF(NAME, ...) sizeof(NAME UNWRAP_VARIANT __VA_ARGS__)
#define CREATE_OBJECT_INPLACE(NAME, ...) __copy_data_to_ptr<NAME UNWRAP_VARIANT __VA_ARGS__>(ptr, NAME UNWRAP_VARIANT __VA_ARGS__(dyslang::Properties(props)))
#define TRAVERSE_OBJECT(NAME, ...) Ptr<NAME UNWRAP_VARIANT __VA_ARGS__>(ptr).traverse(dyslang::Properties(props))

// set by plugincompiler
#ifndef __DYSLANG_VARIANTS__
// remove warnings
#define __DYSLANG_VARIANTS__
#endif 

//template<typename T>
//struct Test {
//
//};
//#define INTERFACE ITest
//#define IMPLEMENTATION Test
//#define IMPLEMENTATION_PARENTHESIS(NAME, X) (NAME<X>)
//#define INTERFACE_PARENTHESIS(x) (INTERFACE<x>)
//#define CREATE(x) <IMPLEMENTATION<x>>(ptr, IMPLEMENTATION<x>({props}))
//#define VARIANTS_RAW (float_rgb, ()), (double_rgb, (<double, double, float>))
//#define VARIANTS_RAW_NON_TEMPLATE (float_rgb, )
//const char* test(const char* variant) {
//	GET_VARIANT_STRING(VARIANTS_RAW)
//	IMPLEMENT_VARIANTS(IMPLEMENTATION, STRINGIFY, VARIANTS_RAW)
//	IMPLEMENT_VARIANTS(IMPLEMENTATION, SIZE_OF, VARIANTS_RAW)
//	IMPLEMENT_VARIANTS(IMPLEMENTATION, SIZE_OF, VARIANTS_RAW_NON_TEMPLATE)
//	return "";
//}

#ifdef __cplusplus

#define UUID(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31) \
SLANG_COM_INTERFACE(0x##_0 ##_1 ##_2 ##_3 ##_4 ##_5 ##_6 ##_7, 0x##_8 ##_9 ##_10 ##_11, 0x##_12 ##_13 ##_14 ##_15, { 0x##_16 ##_17, 0x##_18 ##_19, 0x##_20 ##_21, 0x##_22 ##_23, 0x##_24 ##_25, 0x##_26 ##_27, 0x##_28 ##_29, 0x##_30 ##_31 } )
#define slangInterface(_VISIBILITY, _NAME) struct _NAME : public ISlangUnknown {
#define slangInterfaceUUID(_VISIBILITY, _NAME, _UUID) \
struct _NAME : public ISlangUnknown { \
_UUID

#define __generic template
#define concept(TYPE, VAR) TYPE VAR
#define annotations(...)
#define interface struct
#define vbegin(RETURN) virtual SLANG_NO_THROW RETURN SLANG_MCALL  // for interface methods
#define vend = 0        // for interface methods

#elif defined(__SLANG__)
#define concept(TYPE, VAR) VAR : TYPE
#define annotations(...) [__VA_ARGS__]
#define vbegin(RETURN) RETURN
#define vend

#define UUID(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31) \
    [COM(#_0 #_1 #_2 #_3 #_4 #_5 #_6 #_7 "-" #_8 #_9 #_10 #_11 "-" #_12 #_13 #_14 #_15  "-" #_16 #_17 #_18 #_19 "-" #_20 #_21 "-" #_22 #_23 "-" #_24 #_25 "-" #_26 #_27 "-" #_28 #_29 "-" #_30 #_31)]
#define slangInterface(_VISIBILITY, _NAME) _VISIBILITY interface _NAME {
#define slangInterfaceUUID(_VISIBILITY, _NAME, _UUID) \
_UUID \
_VISIBILITY interface _NAME {

[require(cpp)]
bool operator==(NativeString left, NativeString right)
{
    __requirePrelude(R"(#include <cstring>)");
    __intrinsic_asm R"(strcmp($0, $1) == 0)";
}
[require(cpp)]
bool operator!=(NativeString left, NativeString right)
{
    return !(left == right);
}

[require(cpp)]
void __copy_data_to_ptr<T>(Ptr<void> ptr, T data) {
    __intrinsic_asm R"(memcpy($0, &$1, sizeof($T1)))";\
}

#ifdef __SLANG_CPP__
#define IMPLEMENT_PLUGIN(INTERFACE, IMPLEMENTATION) \
    export __extern_cpp NativeString __interface_name() { return #INTERFACE; }\
    export __extern_cpp NativeString __implementation_name() { return #IMPLEMENTATION; }\
	export __extern_cpp NativeString __available_variants() { return GET_VARIANT_STRING(__DYSLANG_VARIANTS__); }\
	export __extern_cpp NativeString __interface_variant_name(NativeString variant) {\
		IMPLEMENT_VARIANTS(INTERFACE, STRINGIFY, __DYSLANG_VARIANTS__)\
		return "";\
	}\
	export __extern_cpp NativeString __implementation_variant_name(NativeString variant) {\
		IMPLEMENT_VARIANTS(IMPLEMENTATION, STRINGIFY, __DYSLANG_VARIANTS__)\
		return "";\
	}\
    export __extern_cpp size_t __implementation_size(NativeString variant) {\
		IMPLEMENT_VARIANTS(IMPLEMENTATION, SIZE_OF, __DYSLANG_VARIANTS__)\
        return 0;\
    }\
	export __extern_cpp void __create_object(dyslang::IProperties props, NativeString variant, Ptr<void> ptr) {\
		IMPLEMENT_VARIANTS(IMPLEMENTATION, CREATE_OBJECT_INPLACE, __DYSLANG_VARIANTS__)\
	}\
	export __extern_cpp void __traverse(dyslang::IProperties props, NativeString variant, Ptr<void> ptr) {\
		IMPLEMENT_VARIANTS(IMPLEMENTATION, TRAVERSE_OBJECT, __DYSLANG_VARIANTS__)\
	}
#else
#define IMPLEMENT_PLUGIN(INTERFACE, NAME)
#endif
#endif

#ifndef __SLANG__
#include <variant>
#include <iostream>
#include <map>
#include <string>
#include <cstring>
#include <concepts>
#include <optional>
#include <array>
#include <span>
#include <slang.h>
#include <dyslang/platform.h>
#include <dyslang/plugin.h>
#include <dyslang/slangc.h>

namespace dyslang {
    template <typename Tuple> struct tuple_to_variant;
    template <typename... Ts> struct tuple_to_variant<std::tuple<Ts...>>
    { using type = std::variant<Ts ...>; };

	template <typename T> concept arithmetic = std::integral<T> || std::floating_point<T>;
    template <typename T> inline constexpr bool is_arithmetic_v = std::is_floating_point_v<T> || std::is_integral_v<T>;

    template <arithmetic T, size_t N> using vector = std::array<T, N>;
    template <arithmetic T, size_t R, size_t C> using matrix = std::array<vector<T, C>, R>;
    template <arithmetic T, size_t N> using Vector = vector<T, N>;
    template <arithmetic T, size_t R, size_t C> using Matrix = matrix<T, R, C>;

    // glsl/hlsl/slang bool is 32 bits
    // c++ bool is 8 bits
    using b32 = uint32_t;
    using size_t = std::size_t;
    using NativeString = const char*;
    
    template<typename T> using DynamicArray = std::span<T>;
    template<typename T> using Ref = T**;
}
#else
namespace dyslang {
    typealias arithmetic = __BuiltinArithmeticType;
	typealias b32 = uint32_t;

	public struct DynamicArray<T> {
        typedef T Element;
        public T* data;
        public uint64_t count;
    };
}
#endif

namespace dyslang
{
    slangInterfaceUUID(
        internal,
        IProperties,
        UUID(A,2,F,5,4,8,6,6, 7,A,E,F, 4,9,0,5, B,4,C,E, 4,7, A,C, 7,3, C,A, 3,C, 0,7)
    )
        vbegin(dyslang::b32) has(const NativeString) vend;

#define dyslang_properties_get(TYPE) vbegin(uint64_t) get(const NativeString, TYPE**, size_t* dims /*3*/, int64_t* stride_in_bytes /*3*/) vend;
	    dyslang_properties_get(void)
		dyslang_properties_get(int32_t)
        dyslang_properties_get(uint32_t)
        dyslang_properties_get(int64_t)
        dyslang_properties_get(uint64_t)
        dyslang_properties_get(float)
        dyslang_properties_get(double)
#undef dyslang_properties_get

#define dyslang_properties_set(TYPE) vbegin(void) set(const NativeString key, TYPE* ptr, size_t* dims /*3*/, int64_t* stride_in_bytes /*3*/, uint64_t total_size_in_bytes, uint64_t type) vend;
		dyslang_properties_set(void)
        dyslang_properties_set(int32_t)
        dyslang_properties_set(uint32_t)
        dyslang_properties_set(int64_t)
        dyslang_properties_set(uint64_t)
        dyslang_properties_set(float)
        dyslang_properties_set(double)
#undef dyslang_properties_set

		vbegin(Ref<IProperties>) _add_scope(NativeString) vend;
        vbegin(Ref<IProperties>) _get_scope(NativeString) vend;
        vbegin(void*) _new(size_t bytes) vend;
	};

    // todo: use DescriptorHandle directly
    #ifdef __SLANG__
    public struct _DescriptorHandle<T : IOpaqueDescriptor> {
        typedef vector<uint32_t, 2> Type;
        public vector<uint32_t, 2> id;
        public DescriptorHandle<T> get() {
            return {id};
        }
    };
    #else
    struct DescriptorHandle : vector<uint32_t, 2>
    {
        using type = vector<uint32_t, 2>;
    };
    #endif

#ifdef __SLANG__
namespace __private {
    void prelude(){
        __requirePrelude(R"(
            #include <type_traits>
            #include <stdexcept>
            #include <iostream>
            #include <array>
            #include <cstdio>

            template <typename T> struct is_vector : std::false_type {};
            template <typename T, size_t N> struct is_vector<Vector<T, N>> : std::true_type {};
            template <typename T> struct is_matrix : std::false_type {};
            template <typename T, size_t R, size_t C> struct is_matrix<Matrix<T, R, C>> : std::true_type {};
            
            template <typename T> struct vector_info {
                using type = T;
                static constexpr size_t rows = 0, cols = 0, size = 0;
            };
            template <typename T, size_t N> struct vector_info<FixedArray<T, N>> {
                using type = T;
                static constexpr size_t rows = N, cols = 0, size = N;
            };
            template <typename T, size_t N> struct vector_info<Vector<T, N>> {
                using type = T;
                static constexpr size_t rows = N, cols = 0, size = N;
            };
            template <typename T, size_t R, size_t C> struct vector_info<Matrix<T, R, C>> {
                using type = T;
                static constexpr size_t rows = R, cols = C, size = R * C;
            };

            template <typename T> inline constexpr bool is_arithmetic_v = std::is_floating_point_v<T> || std::is_integral_v<T>;
            struct DynamicArrayHelper { void* ptr; uint64_t count; };
        )");
    }

    [require(cpp)]
    T get<T>(NativeString key, dyslang::IProperties properties) {
        __requirePrelude(R"(
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

					uintptr_t temp_int = *(uintptr_t*)value;
                    DynamicArrayHelper result = { (void*)temp_int, dims[0] };
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

    // slang templates do not map to explicit function types
    [require(cpp)]
    void set<T>(NativeString key, T *ptr, size_t *dims /*3*/, int64_t *stride_in_bytes /*3*/, uint64_t total_size_in_bytes, uint64_t type, dyslang::IProperties properties) {
        __intrinsic_asm R"($6->set($0, (vector_info<std::remove_pointer_t<$T1>>::type*)$1, $2, $3, $4, $5))";
    }

    [require(cpp)]
    Tuple<vector<size_t, 3>, vector<int64_t, 3>> info<T>(T value) {
        __intrinsic_asm R"(
        $TR{ 
            Vector<size_t, 3>(vector_info<$T0>::rows, vector_info<$T0>::cols, 0 ), 
            is_matrix<$T0>::value ? Vector<int64_t, 3>( vector_info<$T0>::cols * sizeof(vector_info<$T0>::type), sizeof(vector_info<$T0>::type), 0 ) : Vector<int64_t, 3>( sizeof(vector_info<$T0>::type), 0, 0 ) 
        })";
    }
}

public struct Properties {
    private dyslang::IProperties __properties;
    __init(dyslang::IProperties properties){
        __private::prelude();
        __properties = properties;
        has("");
    }
    __init() {}

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

    public void set<T>(NativeString key, T* value)
    {
        __target_switch
        {
        case cpp:
            // printf("Set Arithmetic: %s\n", key);
            var i = __private::info<T>({});
            __private::set<T>(key, value, &i._0[0], &i._1[0], uint64_t(sizeof(T)), uint64_t(0), __properties);
        }
    }

    public void set<T>(NativeString key, T** value)
    {
        __target_switch
        {
        case cpp:
            // printf("Set Pointer: %s\n", key);
            var i = __private::info<T>( {});
            __private::set<T*>(key, value, &i._0[0], &i._1[0], uint64_t(sizeof(T)), uint64_t(3), __properties);
        }
    }

    public void set<T : IOpaqueDescriptor>(NativeString key, dyslang::_DescriptorHandle<T>* value)
    {
        __target_switch
        {
        case cpp:
            // printf("Set _DescriptorHandle: %s\n", key);
            var i = __private::info<value.Type>({});
            __private::set<value.Type>(key, (value.Type*)&value.id, &i._0[0], &i._1[0], uint64_t(sizeof(T)), uint64_t(2), __properties);
        }
    }

    public void set<T : IArithmetic>(NativeString key, dyslang::DynamicArray<T>* value)
    {
        __target_switch
        {
        case cpp:
            // printf("Set DynamicArray: %s\n", key);
            var i = __private::info<value.Element>({});
            i._0 = i._0.zxy;
            i._0[0] = size_t(value.count);
            i._1 = i._1.zxy;
            i._1[0] = sizeof(T);
            __private::set<value.Element>(key, (value.Element*)&value.data, &i._0[0], &i._1[0], uint64_t(value.count *sizeof(T)), uint64_t(1), __properties);
        }
    }

    public Properties get_scope(NativeString key) {
        __target_switch
        {
        case cpp:
            return Properties(__properties._get_scope(key));
        default:
            return Properties();
        }
    }

    public Properties set_scope(NativeString key) {
        __target_switch
        {
        case cpp:
            return Properties(__properties._add_scope(key));
        default:
            return Properties();
        }
    }

    __generic<typename T>
    public T* new() {
        __target_switch
        {
        case cpp:
            return (T*)__properties._new(sizeof(T));
        default:
            return nullptr;
        }
    }
};

#elif __cplusplus
	
	namespace detail {
        template <typename, typename = void> struct dim_traits { static constexpr std::array<size_t, 3> value = { 0, 0, 0 }; };
        template <typename T, size_t N> struct dim_traits<Vector<T, N>> {
            static constexpr std::array<size_t, 3> value = [] {
                auto sub = dim_traits<T>::value;
                return std::array<size_t, 3>{ N, sub[0], sub[1] };
            }();
        };
        template <typename T, size_t R, size_t C> struct dim_traits<Matrix<T, R, C>> {
            static constexpr std::array<size_t, 3> value = [] {
                return std::array<size_t, 3>{ R, C, 0 };
            }();
        };
        template <typename T> struct dim_traits<DynamicArray<T>> {
            static constexpr std::array<size_t, 3> value = [] {
                auto sub = dim_traits<T>::value;
                return std::array<size_t, 3>{ 0, sub[0], sub[1] };
            }();
        };
        template <> struct dim_traits<DescriptorHandle> { static constexpr std::array<size_t, 3> value = dim_traits<DescriptorHandle::type>::value; };
        template <typename T> constexpr std::array<size_t, 3> get_dims_v = [] {
            auto sub = dim_traits<std::remove_cv_t<std::remove_reference_t<T>>>::value;
            return sub;
        }();

        template <typename T, typename = void> struct stride_traits { static constexpr std::array<int64_t, 4> value = { sizeof(T), 0, 0, 0 }; };
		template <typename T, size_t N> struct stride_traits<Vector<T, N>> {
            static constexpr std::array<int64_t, 4> value = [] {
                auto sub = stride_traits<T>::value;
                return std::array<int64_t, 4>{ N * sizeof(T), sub[0], sub[1], sub[2] };
            }();
        };
        template <typename T> struct stride_traits<DynamicArray<T>> {
            static constexpr std::array<int64_t, 4> value = [] {
                auto sub = stride_traits<T>::value;
                return std::array<int64_t, 4>{ sizeof(T), sub[0], sub[1], sub[2] };
            }();
        };
        template <> struct stride_traits<DescriptorHandle> { static constexpr std::array<int64_t, 4> value = stride_traits<DescriptorHandle::type>::value; };
        template <typename T> constexpr std::array<int64_t, 3> get_stride_v = [] {
            auto sub = stride_traits<std::remove_cv_t<std::remove_reference_t<T>>>::value;
			return std::array<int64_t, 3>{ sub[1], sub[2], sub[3] };
        }();

        template <typename T> struct type_traits { using type = std::conditional_t<dyslang::is_arithmetic_v<T>, T, void>; };
        template <typename T, size_t N> struct type_traits<Vector<T, N>> { using type = type_traits<T>::type; };
        template <typename T, size_t R, size_t C> struct type_traits<Matrix<T, R, C>> { using type = type_traits<T>::type; };
        template <> struct type_traits<DescriptorHandle> { using type = type_traits<DescriptorHandle::type>::type; };
        template <typename T> struct type_traits<DynamicArray<T>> { using type = type_traits<T>::type; };
        template <typename T> using get_type_t = type_traits<std::remove_cv_t<std::remove_reference_t<T>>>::type;

        template <typename> struct is_dynamic_array : std::false_type {};
        template <typename T> struct is_dynamic_array<DynamicArray<T>> : std::true_type {};
        template <typename T> concept IsDynamicArray = is_dynamic_array<T>::value;

        template <typename T> struct get_dynamic_array_type { using type = T; };
        template <typename T> struct get_dynamic_array_type<DynamicArray<T>> { using type = T; };

        template <typename T, typename = void> struct parameter_type_traits { static constexpr uint64_t value = 0; };
        template <typename T> struct parameter_type_traits<DynamicArray<T>> { static constexpr uint64_t value = 1; };
        template <> struct parameter_type_traits<DescriptorHandle> { static constexpr uint64_t value = 2; };
        template <typename T> struct parameter_type_traits<T*> { static constexpr uint64_t value = 3; };
	}

	struct Properties : IProperties
	{
	    using SupportedTypes = std::tuple<
            void*, int32_t*, uint32_t*, int64_t*, uint64_t*, float*, double*, Properties*
		>; // type 1 we store T**, for type 3 we store void**
	    using VariantType = tuple_to_variant<SupportedTypes>::type;
		struct Entry
		{
			VariantType ptr;
            vector<size_t, 3> dimension = { 0, 0, 0 };
			vector<int64_t, 3> stride_in_bytes = { 0, 0, 0 };
            uint64_t total_size_in_bytes = 0;
            uint64_t type = 0; // 1 for dynamic array, 2 descriptor handle, 3 external pointer, 0 for others

            [[nodiscard]] size_t count() const {
                size_t size = 1;
                for (const auto dim : dimension) {
                    size *= dim;
                }
                return size;
            }
		};
	    // We don't need queryInterface for this impl, or ref counting
	    vbegin(SlangResult) queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE { return SLANG_E_NOT_IMPLEMENTED; }
	    vbegin(uint32_t) addRef() SLANG_OVERRIDE { return 1; }
	    vbegin(uint32_t) release() SLANG_OVERRIDE { return 1; }

	    // Properties
		virtual ~Properties() = default;
		vbegin(dyslang::b32) has(const NativeString key) SLANG_OVERRIDE { return properties.contains(key); }

	#define dyslang_properties_get(TYPE) \
	    vbegin(uint64_t) get(const NativeString key, TYPE** ptr, size_t* dims, int64_t* stride_in_bytes) SLANG_OVERRIDE { \
	        auto& entry = properties[key]; \
        	if (!std::holds_alternative<TYPE*>(entry.ptr)) \
				std::cout << "Property \'" + std::string(key) + "\' type mismatch." << std::endl; \
	        *ptr = std::get<TYPE*>(entry.ptr); \
	        *(vector<size_t, 3>*)dims = entry.dimension; \
            *(vector<int64_t, 3>*)stride_in_bytes = entry.stride_in_bytes; \
            return entry.type; \
	    }
        dyslang_properties_get(void)
	    dyslang_properties_get(int32_t)
	    dyslang_properties_get(uint32_t)
	    dyslang_properties_get(int64_t)
	    dyslang_properties_get(uint64_t)
	    dyslang_properties_get(float)
	    dyslang_properties_get(double)
	#undef dyslang_properties_get

	#define dyslang_properties_set(TYPE) vbegin(void) set(const NativeString key, TYPE* ptr, size_t* dims, int64_t* stride_in_bytes, uint64_t total_size_in_bytes, uint64_t type) SLANG_OVERRIDE { properties[key] = Entry{ ptr, *(vector<size_t, 3>*)dims, *(vector<int64_t, 3>*)stride_in_bytes, total_size_in_bytes, type }; }
		dyslang_properties_set(void)
		dyslang_properties_set(int32_t)
	    dyslang_properties_set(uint32_t)
	    dyslang_properties_set(int64_t)
	    dyslang_properties_set(uint64_t)
	    dyslang_properties_set(float)
	    dyslang_properties_set(double)
	#undef dyslang_properties_set

        void* _new(size_t bytes) SLANG_OVERRIDE
        {
            return malloc(bytes);
		}

        Ref<IProperties> _add_scope(NativeString key) SLANG_OVERRIDE
        {
            scopes.emplace_back();
			properties[key] = Entry{ &scopes.back(), {0,0,0}, {0,0,0}, 0, 0 };
            return _get_scope(key);
		}

        Ref<IProperties> _get_scope(NativeString key) SLANG_OVERRIDE
		{
			return (IProperties**)&std::get<Properties*>(properties[key].ptr);
        }

        Properties& add_scope(NativeString key) {
            return *(Properties*)*_add_scope(key);
		}
        Properties& get_scope(NativeString key) {
            return *(Properties*)*_get_scope(key);
        }

		template <typename T> void set(const NativeString key, T& data)
        {
            vector<size_t, 3> dims = detail::get_dims_v<T>;
            vector<int64_t, 3> stride_in_bytes = detail::get_stride_v<T>;
            set(key, (detail::get_type_t<T>*) & data, dims.data(), stride_in_bytes.data(), sizeof(T), detail::parameter_type_traits<T>::value);
        }
        template <typename T> void set(const NativeString key, T* data)
        {
            vector<size_t, 3> dims = detail::get_dims_v<T>;
            vector<int64_t, 3> stride_in_bytes = detail::get_stride_v<T>;
            set(key, (detail::get_type_t<T>*) & data, dims.data(), stride_in_bytes.data(), sizeof(T), 3);
        }
        template <typename T> void set(const NativeString key, DynamicArray<T>& data)
        {
            vector<size_t, 3> dims = detail::get_dims_v<DynamicArray<T>>;
            vector<int64_t, 3> stride_in_bytes = detail::get_stride_v<DynamicArray<T>>;
            dims[0] = data.size();
            stride_in_bytes[0] *= data.size();
            set(key, (detail::get_type_t<T>*) &data, dims.data(), stride_in_bytes.data(), data.size() * sizeof(T), 1);
        }

        template <typename T> T& get(const NativeString key)
        {
            vector<size_t, 3> dims;
            vector<int64_t, 3> stride_in_bytes;
            detail::get_type_t<T>* ptr;
            // check if property is correct type
            if (!has(key))
                throw std::runtime_error("Property \'" + std::string(key) + "\' does not exist.");
            if (!std::holds_alternative<detail::get_type_t<T>*>(properties[key].ptr))
                throw std::runtime_error("Property \'" + std::string(key) + "\' type mismatch.");
            get(key, &ptr, dims.data(), stride_in_bytes.data());
            return *(T*)ptr;
        }
        template <detail::IsDynamicArray T> T& get(const NativeString key)
        {
            vector<size_t, 3> dims;
            vector<int64_t, 3> stride_in_bytes;
            detail::get_type_t<T>* ptr;
            // check if property is correct type
            if (!has(key))
                throw std::runtime_error("Property \'" + std::string(key) + "\' does not exist.");
            if (!std::holds_alternative<detail::get_type_t<T>*>(properties[key].ptr))
                throw std::runtime_error("Property \'" + std::string(key) + "\' type mismatch.");
            get(key, &ptr, dims.data(), stride_in_bytes.data());
            return *(T*)ptr;
        }

	    [[nodiscard]] std::string to_string() const {
	        std::string result = "Properties:\n";
	        for (const auto& [key, value] : properties) {
	            result += " ";
	            result += key;
	            result += ": ";
                std::visit(
                    [&key, &value, &result](auto* ptr) {
                    using T = std::decay_t<std::remove_cv_t<std::remove_reference_t<decltype(ptr)>>>;
                    result += "Type=" + std::to_string(value.type) + " ";
                    if (ptr == nullptr) {
                        result += "null";
                        return;
                    }
                    else if constexpr (std::is_same_v<decltype(ptr), Properties*>) {
                        result += "\n<< ";
                        result += key;
						result += " ";
                    	result += ptr->to_string();
                        result += ">> ";
                        result += key;
                    }
                    else if constexpr (std::is_same_v<decltype(ptr), void*>) {
                        result += "void*";
                        result += " (total size: " + std::to_string(value.total_size_in_bytes) + " bytes)";
                    }
                    else {
                        if (value.type == 1) {
                            uintptr_t temp_int = *(uintptr_t*)ptr;
                            ptr = (decltype(ptr))temp_int;
                        }
                        if (value.dimension[0] == 0 && value.dimension[1] == 0 && value.dimension[2] == 0)
                            result += std::to_string(ptr[0]);
                        else
                        {
                            if (value.type == 2) result += "DescriptorHandle ";
                            std::array<int64_t, 3> stride = {
                                value.stride_in_bytes[0] / static_cast<int64_t>(sizeof(T)),
                                value.stride_in_bytes[1] / static_cast<int64_t>(sizeof(T)),
                                value.stride_in_bytes[2] / static_cast<int64_t>(sizeof(T))
                            };
                            size_t index = 0;

                            if (value.dimension[2]) result += "[";
                            for (size_t i = 0; i < std::max(size_t(1), value.dimension[2]); ++i) {
                                if (value.dimension[1]) result += "[";
                                for (size_t j = 0; j < std::max(size_t(1), value.dimension[1]); ++j) {
                                    if (value.dimension[0]) result += "[";
                                    std::string sep;
                                    for (size_t k = 0; k < std::max(size_t(1), value.dimension[0]); ++k) {
                                        result += sep + std::to_string(ptr[index]);
                                        index++;
                                        sep = " ";
                                    }
                                    if (value.dimension[0]) result += "]";
                                }
                                if (value.dimension[1]) result += "]";
                            }
                            if (value.dimension[2]) result += "]";

                            result += " (dims: " + std::to_string(value.dimension[0]) + ", " +
                                      std::to_string(value.dimension[1]) + ", " + std::to_string(value.dimension[2]) + ")";
                            result += " (stride: " + std::to_string(value.stride_in_bytes[0]) + ", " +
                                      std::to_string(value.stride_in_bytes[1]) + ", " + std::to_string(value.stride_in_bytes[2]) + ")";
                            result += " (total size: " + std::to_string(value.total_size_in_bytes) + " bytes)";
                        }
                    }
                }, value.ptr);
	            result += "\n";
	        }
	        return result;
	    }
	    struct cmp_str
	    {
	        bool operator()(char const* a, char const* b) const { return std::strcmp(a, b) < 0; }
	    };
	    std::map<const char*, Entry, cmp_str> properties;
		std::vector<Properties> scopes;
	};
#endif
}
