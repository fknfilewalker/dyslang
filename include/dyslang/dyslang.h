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
#define CREATE_OBJECT_INPLACE(NAME, ...) __create_object_inplace_helper<NAME UNWRAP_VARIANT __VA_ARGS__>(ptr, NAME UNWRAP_VARIANT __VA_ARGS__(dyslang::Properties(props)))
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

#define slangGInterface(_VISIBILITY, _GENERIC, _NAME) \
_GENERIC \
struct _NAME : public ISlangUnknown {
#define slangGInterfaceUUID(_VISIBILITY, _GENERIC, _NAME, _UUID) \
_GENERIC \
struct _NAME : public ISlangUnknown { \
_UUID

#define __generic template
#define __concept(TYPE, VAR) TYPE VAR
#define annotations(...)
#define init(NAME) NAME
#define interface struct
#define typealias using

#define vbegin(RETURN) virtual SLANG_NO_THROW RETURN SLANG_MCALL  // for interface methods
#define vend = 0        // for interface methods

#elif defined(__SLANG__)

#define __concept(TYPE, VAR) VAR : TYPE
#define annotations(...) [__VA_ARGS__]
#define init(NAME) __init
#define vbegin(RETURN) RETURN
#define vend

#define UUID(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31) \
    [COM(#_0 #_1 #_2 #_3 #_4 #_5 #_6 #_7 "-" #_8 #_9 #_10 #_11 "-" #_12 #_13 #_14 #_15  "-" #_16 #_17 #_18 #_19 "-" #_20 #_21 "-" #_22 #_23 "-" #_24 #_25 "-" #_26 #_27 "-" #_28 #_29 "-" #_30 #_31)]
#define slangInterface(_VISIBILITY, _NAME) _VISIBILITY interface _NAME {
#define slangInterfaceUUID(_VISIBILITY, _NAME, _UUID) \
_UUID \
_VISIBILITY interface _NAME {
#define slangGInterface(_VISIBILITY, _GENERIC, _NAME) \
_GENERIC \
_VISIBILITY interface _NAME {
#define slangGInterfaceUUID(_VISIBILITY, _GENERIC, _NAME, _UUID) \
_UUID \
_GENERIC \
_VISIBILITY interface _NAME {

#ifdef __SLANG_CPP__
bool operator==(NativeString left, NativeString right)
{
    __requirePrelude(R"(
		#include <cstring>
    )");
    __intrinsic_asm R"(strcmp($0, $1) == 0)";
}
bool operator!=(NativeString left, NativeString right)
{
    return !(left == right);
}

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
	void __create_object_inplace_helper<T>(Ptr<void> ptr, T data) {\
		__intrinsic_asm R"(memcpy($0, &$1, sizeof($T1)))";\
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
#include <slang.h>
#include <dyslang/platform.h>
#include <dyslang/plugin.h>
#include <dyslang/slangc.h>

namespace dyslang {
    template <typename Tuple>
    struct tuple_to_variant;

    template <typename... Ts>
    struct tuple_to_variant<std::tuple<Ts...>>
    {
        using type = std::variant<Ts ...>;
    };

	template <typename T> concept arithmetic = std::integral<T> || std::floating_point<T>;
    template <typename T> inline constexpr bool is_arithmetic_v = std::is_floating_point_v<T> || std::is_integral_v<T>;

    template <arithmetic T, size_t N> using vector = std::array<T, N>;
    template <arithmetic T, size_t R, size_t C> using matrix = std::array<vector<T, C>, R>;
    /*template <arithmetic T, size_t R, size_t C> struct matrix {
        static constexpr int rows_v = R;
        static constexpr int columns_v = C;
        static constexpr int length_v = R * C;
        using vector_t = vector<T, C>;

        template <typename... Ts>
        matrix(Ts... ts) requires(sizeof...(ts) == length_v)
        {
            int i = 0;
            ((entries[i++] = static_cast<T>(ts)), ...);
        }
    	const T* data() const { return entries.data(); }

		operator std::array<vector<T, C>, R>& () { return rows; }
        union
        {
            std::array<vector<T, C>, R> rows;
            vector<T, length_v> entries;
        };
    };*/

    inline constexpr auto none = std::nullopt;

    // glsl/hlsl/slang bool is 32 bits
    // c++ bool is 8 bits
    //struct b32
    //{
    //    int32_t value; // should it be int32_t or uint32_t?
    //    b32() : value{} {}
    //    b32(const bool v) : value{ v ? 1 : 0 } {}
    //    operator bool() const { return value > 0; }
    //};
    using b32 = uint32_t;
    using CString = const char*;
    using size_t = std::size_t;

    // our version of the slang Optional
    template <typename T>
    struct Optional
    {
        Optional() : hasValue{}, value{} {}
        Optional(std::nullopt_t) : Optional{} {}
        Optional(const T& v) : hasValue{ true }, value{ v } {}

        b32 hasValue;
        T value;

        Optional& operator=(const T& v) { value = v; hasValue = 1; return *this; }
        Optional& operator=(const std::nullopt_t&) { value = {}; hasValue = 0; return *this; }
    };
}

#else
namespace dyslang {
	typealias b32 = uint32_t;
	typealias CString = NativeString;
}
#endif

namespace dyslang
{
    slangInterfaceUUID(
        internal,
        IProperties,
        UUID(A,2,F,5,4,8,6,6, 7,A,E,F, 4,9,0,5, B,4,C,E, 4,7, A,C, 7,3, C,A, 3,C, 0,7)
    )
        vbegin(dyslang::b32) has(const dyslang::CString) vend;

#define dyslang_properties_get(TYPE) vbegin(void) get(const dyslang::CString, TYPE**, size_t* dims /*3*/, int64_t* stride_in_bytes /*3*/) vend;
        dyslang_properties_get(int32_t)
        dyslang_properties_get(uint32_t)
        dyslang_properties_get(int64_t)
        dyslang_properties_get(uint64_t)
        dyslang_properties_get(float)
        dyslang_properties_get(double)
#undef dyslang_properties_get

#define dyslang_properties_set(TYPE) vbegin(void) set(const dyslang::CString key, TYPE* ptr, size_t* dims /*3*/, int64_t* stride_in_bytes /*3*/) vend;
        dyslang_properties_set(int32_t)
        dyslang_properties_set(uint32_t)
        dyslang_properties_set(int64_t)
        dyslang_properties_set(uint64_t)
        dyslang_properties_set(float)
        dyslang_properties_set(double)
#undef dyslang_properties_set
	};

#ifdef __SLANG_CPP__

namespace __private {
    T get<T>(dyslang::CString key, dyslang::IProperties properties) {
        __requirePrelude(R"(
                #include <type_traits>
                #include <stdexcept>
				#include <iostream>
				#include <array>
				#include <cstdio>

                template <typename T> struct is_vector : std::false_type {};
                template <typename T, size_t N> struct is_vector<Vector<T, N>> : std::true_type {};
                
                template <typename T> struct vector_info {};
                template <typename T, size_t N> struct vector_info<Vector<T, N>> {
                    using type = T;
                    static constexpr size_t size = N;
                };
				
				template <typename T, typename PROPERTIES_T> 
                T getProperty(const char* key, T dummy, PROPERTIES_T props){
                    T* value;
					std::array<size_t, 3> dims = { 0, 0, 0 };
					std::array<int64_t, 3> stride_in_bytes = { 0, 0, 0 };
					props->get(key, &value, dims.data(), stride_in_bytes.data());
                    if (dims[0] != 0) std::cout << "Warning <dyslang>: \'" << key << "\' Property size mismatch" << std::endl;
                    return *value;
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
            )");
        __intrinsic_asm R"(getProperty($0, $TR(), $1))";
    }

    void set<T>(dyslang::CString key, __ref T value, dyslang::IProperties properties) {
        __requirePrelude(R"(
			template <typename T, typename PROPERTIES_T> 
            void setProperty(const char* key, T* value, PROPERTIES_T& props){
				std::array<size_t, 3> dims = { 0, 0, 0 };
				std::array<int64_t, 3> stride_in_bytes = { 0, 0, 0 };
                props->set(key, value, dims.data(), stride_in_bytes.data());
            }
			template <typename T, int N, typename PROPERTIES_T>
            void setProperty(const char* key, Vector<T, N>* value, PROPERTIES_T& props){
				std::array<size_t, 3> dims = { N, 0, 0 };
				std::array<int64_t, 3> stride_in_bytes = { sizeof(T), 0, 0 };
                props->set(key, (T*)value, dims.data(), stride_in_bytes.data());
            }
            template <typename T, size_t N, typename PROPERTIES_T> 
            void setProperty(const char* key, FixedArray<T, N>* value, PROPERTIES_T& props){
                std::array<size_t, 3> dims = { N, 0, 0 };
				std::array<int64_t, 3> stride_in_bytes = { sizeof(T), 0, 0 };
                props->set(key, (T*)value, dims.data(), stride_in_bytes.data());
            }
			template <typename T, int ROWS, int COLS, typename PROPERTIES_T> 
            void setProperty(const char* key, Matrix<T, ROWS, COLS>* value, PROPERTIES_T& props){
				std::array<size_t, 3> dims = { ROWS, COLS, 0 };
				std::array<int64_t, 3> stride_in_bytes = { COLS * sizeof(T), sizeof(T), 0 };
                props->set(key, (T*)value, dims.data(), stride_in_bytes.data());
            }
        )");
        __intrinsic_asm R"(setProperty($0, $1, $2))";
    }
}
#endif
#ifdef __SLANG__

struct Properties {
    private dyslang::IProperties __properties;
    __init(dyslang::IProperties properties){
        __properties = properties;
    }
    bool has(dyslang::CString key) {
#ifdef __SLANG_CPP__
        return (bool)__properties.has(key);
#else
        return false;
#endif
    }

    T get<T>(dyslang::CString key) {
#ifdef __SLANG_CPP__
        return __private::get<T>(key, __properties);
#else
        return { {} };
#endif
    }

    void set<T>(dyslang::CString key, __ref T value) {
#ifdef __SLANG_CPP__
        __private::set<T>(key, value, __properties);
#endif
    }
};

#elif __cplusplus
	
	namespace detail {
        template <typename T, typename = void> struct dim_traits { static constexpr std::array<size_t, 3> value = { 0, 0, 0 }; };
        template <typename T, size_t N> struct dim_traits<std::array<T, N>> {
            static constexpr std::array<size_t, 3> value = [] {
                auto sub = dim_traits<T>::value;
                return std::array<size_t, 3>{ N, sub[0], sub[1] };
            }();
        };
        template <typename T> constexpr std::array<size_t, 3> get_dims_v = [] {
            auto sub = dim_traits<std::remove_cv_t<std::remove_reference_t<T>>>::value;
			//if (sub[0] == 0) sub[0] = 1;
            return sub;
        }();

        template <typename T, typename = void> struct stride_traits { static constexpr std::array<int64_t, 4> value = { sizeof(T), 0, 0, 0 }; };
        template <typename T, int64_t N>
        struct stride_traits<std::array<T, N>> {
            static constexpr std::array<int64_t, 4> value = [] {
                std::array<int64_t, 4> sub = stride_traits<T>::value;
                return std::array<int64_t, 4>{ N * sizeof(T), sub[0], sub[1], sub[2] };
            }();
        };
        template <typename T> constexpr std::array<int64_t, 3> get_stride_v = [] {
            std::array<int64_t, 4> sub = stride_traits<std::remove_cv_t<std::remove_reference_t<T>>>::value;
			return std::array<int64_t, 3>{ sub[1], sub[2], sub[3] };
        }();

        template <typename T, typename = void> struct type_traits { using type = T; };
        template <typename T, int64_t N> struct type_traits<std::array<T, N>> { using type = type_traits<T>::type; };
        template <typename T> using get_type_t = type_traits<std::remove_cv_t<std::remove_reference_t<T>>>::type;
	}

	struct Properties : public IProperties
	{
	    using SupportedTypes = std::tuple<
            int32_t*, uint32_t*, int64_t*, uint64_t*, float*, double*
	    >;
	    using VariantType = tuple_to_variant<SupportedTypes>::type;
		struct Entry
		{
			VariantType ptr;
            vector<size_t, 3> dimension = { 0, 0, 0 };
			vector<int64_t, 3> stride_in_bytes = { 0, 0, 0 };

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
		vbegin(dyslang::b32) has(const dyslang::CString key) SLANG_OVERRIDE { return properties.contains(key); }

	#define dyslang_properties_get(TYPE) \
	    vbegin(void) get(const dyslang::CString key, TYPE** ptr, size_t* dims, int64_t* stride_in_bytes) SLANG_OVERRIDE { \
	        auto& entry = properties[key]; \
        	if (!std::holds_alternative<TYPE*>(entry.ptr))\
				std::cout << "Property \'" + std::string(key) + "\' type mismatch." << std::endl;\
	        *ptr = std::get<TYPE*>(entry.ptr); \
	        *(vector<size_t, 3>*)dims = entry.dimension; \
            *(vector<int64_t, 3>*)stride_in_bytes = entry.stride_in_bytes; \
	    }
	    dyslang_properties_get(int32_t)
	    dyslang_properties_get(uint32_t)
	    dyslang_properties_get(int64_t)
	    dyslang_properties_get(uint64_t)
	    dyslang_properties_get(float)
	    dyslang_properties_get(double)
	#undef dyslang_properties_get

	#define dyslang_properties_set(TYPE) vbegin(void) set(const dyslang::CString key, TYPE* ptr, size_t* dims, int64_t* stride_in_bytes) SLANG_OVERRIDE { properties[key] = Entry{ ptr, *(vector<size_t, 3>*)dims, *(vector<int64_t, 3>*)stride_in_bytes }; }
	    dyslang_properties_set(int32_t)
	    dyslang_properties_set(uint32_t)
	    dyslang_properties_set(int64_t)
	    dyslang_properties_set(uint64_t)
	    dyslang_properties_set(float)
	    dyslang_properties_set(double)
	#undef dyslang_properties_set

		template <typename T> void set(const dyslang::CString key, T& data)
        {
            vector<size_t, 3> dims = detail::get_dims_v<T>;
            vector<int64_t, 3> stride_in_bytes = detail::get_stride_v<T>;
            set(key, (detail::get_type_t<T>*)&data, dims.data(), stride_in_bytes.data());
        }

        template <typename T> T& get(const dyslang::CString key)
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
	            std::visit([&value, &result](auto&& ptr) {
	                using T = std::decay_t<std::remove_cv_t<std::remove_reference_t<decltype(ptr)>>>;
	                if (value.dimension[0] == 0)
	                    result += std::to_string(ptr[0]);
	                else 
		            {
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
	};

#endif
}
