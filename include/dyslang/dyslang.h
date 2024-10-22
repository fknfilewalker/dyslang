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
#define GET_VARIANT(VARIANT, IMPL) #VARIANT ","
#define GET_VARIANT_PAIR(X) GET_VARIANT X
#define GET_VARIANT_STRING(...) MAP(GET_VARIANT_PAIR, __VA_ARGS__)

#define UNWRAP_VARIANT(...) __VA_ARGS__
#define UNWRAP_VARIANT_STRING(...) #__VA_ARGS__
// args (... are the template types)
#define IDENTITY(NAME, ...) UNWRAP_VARIANT __VA_ARGS__
#define STRINGIFY(NAME, ...) #NAME UNWRAP_VARIANT_STRING __VA_ARGS__
#define SIZE_OF(NAME, ...) sizeof(NAME UNWRAP_VARIANT __VA_ARGS__)
#define CREATE_OBJECT_INPLACE(NAME, ...) __create_object_inplace_helper<NAME UNWRAP_VARIANT __VA_ARGS__>(ptr, NAME UNWRAP_VARIANT __VA_ARGS__({props}))
#define TRAVERSE_OBJECT(NAME, ...) Ptr<NAME UNWRAP_VARIANT __VA_ARGS__>(ptr).traverse({props})

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
#define slang_public
#define slang_internal
#define slang_private
#define typealias using
#define gvar(VAR) typename VAR
#define gtvar(TYPE, VAR) TYPE VAR   // typed template variable
#define gvvar(TYPE, VAR) TYPE VAR   // typed non-type variable
#define gdtvar(TYPE, VAR, DEFAULT) TYPE VAR = DEFAULT // typed template variable
#define gdvvar(TYPE, VAR, DEFAULT) TYPE VAR = DEFAULT // typed template variable
#define generic(...) template <__VA_ARGS__>
#define subscript operator[]

#define associatedtype(NAME)
#define cassociatedtype(NAME, CONSTRAIN)

// #define var auto
#define let const auto

#define differentiable
#define constructor(NAME) NAME
#define mutating        // for methods that modify the object

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
#define vbegin(RETURN) virtual SLANG_NO_THROW RETURN SLANG_MCALL  // for interface methods
#define vend = 0        // for interface methods

#elif defined(__SLANG__)
#define slang_public public
#define slang_internal internal
#define slang_private private
#define typealias typealias
#define gvar(VAR) VAR
#define gtvar(TYPE, VAR) VAR : TYPE
#define gvvar(TYPE, VAR) let VAR : TYPE
#define gdtvar(TYPE, VAR, DEFAULT) VAR : TYPE = DEFAULT
#define gdvvar(TYPE, VAR, DEFAULT) let VAR : TYPE = DEFAULT
#define generic(...) __generic<__VA_ARGS__>
#define subscript __subscript

#define associatedtype(NAME) associatedtype NAME
#define cassociatedtype(NAME, CONSTRAIN) associatedtype NAME : CONSTRAIN

#define differentiable [Differentiable]
#define constructor(NAME) __init
#define mutating [mutating]


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
#define vbegin(RETURN) RETURN
#define vend
#define auto var


#ifdef __SLANG_CPP__
bool operator==(dyslang::CString left, dyslang::CString right)
{
    __requirePrelude(R"(
		#include <cstring>
    )");
    __intrinsic_asm R"(strcmp($0, $1) == 0)";
}
bool operator!=(dyslang::CString left, dyslang::CString right)
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

    template<typename T>
    struct always_false : std::false_type {};

    template<typename T> inline constexpr bool always_false_v = always_false<T>::value;

    template <typename T> concept integral = std::integral<T>;
    template <typename T> concept floating_point = std::floating_point<T>;
	template <typename T> concept arithmetic = std::integral<T> || std::floating_point<T>;
    template <typename T> inline constexpr bool is_arithmetic_v = std::is_floating_point_v<T> || std::is_integral_v<T>;

    template <arithmetic T, uint64_t N> using vector = std::array<T, N>;//glm::vec<N, T>;
    template <arithmetic T, uint64_t M, uint64_t N> using matrix = std::array<std::array<T, N>, M>;//glm::mat<M, N, T>;

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
        Optional() : value{}, hasValue{} {}
        Optional(std::nullopt_t) : Optional{} {}
        Optional(const T& v) : value{ v }, hasValue{ true } {}

        T value;
        b32 hasValue;

        Optional& operator=(const T& v) { value = v; hasValue = 1; return *this; }
        Optional& operator=(const std::nullopt_t&) { value = {}; hasValue = 0; return *this; }
    };
}

#else
namespace dyslang {
	slang_internal typealias integral = __BuiltinIntegerType;
	slang_internal typealias floating_point = __BuiltinFloatingPointType;
	slang_internal typealias arithmetic = __BuiltinArithmeticType;

	slang_internal typealias b32 = uint32_t;
	slang_internal typealias CString = NativeString;
}
#endif

namespace dyslang {
    slang_internal typealias i32 = int;
    slang_internal typealias u32 = uint32_t;
    slang_internal typealias i64 = int64_t;
    slang_internal typealias u64 = uint64_t;

    slang_internal typealias f32 = float;
    slang_internal typealias f64 = double;

    generic(gtvar(arithmetic, T)) slang_internal typealias v2 = vector<T, 2>;
    generic(gtvar(arithmetic, T)) slang_internal typealias v3 = vector<T, 3>;
    generic(gtvar(arithmetic, T)) slang_internal typealias v4 = vector<T, 4>;

    slang_internal typealias b32v2 = v2<uint32_t>;
    slang_internal typealias b32v3 = v3<uint32_t>;
    slang_internal typealias b32v4 = v4<uint32_t>;

    slang_internal typealias i32v2 = v2<int32_t>;
    slang_internal typealias i32v3 = v3<int32_t>;
    slang_internal typealias i32v4 = v4<int32_t>;

    slang_internal typealias u32v2 = v2<uint32_t>;
    slang_internal typealias u32v3 = v3<uint32_t>;
    slang_internal typealias u32v4 = v4<uint32_t>;

    slang_internal typealias i64v2 = v2<int64_t>;
    slang_internal typealias i64v3 = v3<int64_t>;
    slang_internal typealias i64v4 = v4<int64_t>;

    slang_internal typealias u64v2 = v2<uint64_t>;
    slang_internal typealias u64v3 = v3<uint64_t>;
    slang_internal typealias u64v4 = v4<uint64_t>;

    slang_internal typealias f32v2 = v2<float>;
    slang_internal typealias f32v3 = v3<float>;
    slang_internal typealias f32v4 = v4<float>;

    slang_internal typealias f64v2 = v2<double>;
    slang_internal typealias f64v3 = v3<double>;
    slang_internal typealias f64v4 = v4<double>;

    generic(gtvar(arithmetic, T)) slang_internal typealias m2x2 = matrix<T, 2, 2>;
    generic(gtvar(arithmetic, T)) slang_internal typealias m3x3 = matrix<T, 3, 3>;
    generic(gtvar(arithmetic, T)) slang_internal typealias m4x4 = matrix<T, 4, 4>;

    slang_internal typealias f32m2x2 = m2x2<float>;
    slang_internal typealias f32m3x3 = m3x3<float>;
    slang_internal typealias f32m4x4 = m4x4<float>;

    slang_internal typealias f64m2x2 = m2x2<double>;
    slang_internal typealias f64m3x3 = m3x3<double>;
    slang_internal typealias f64m4x4 = m4x4<double>;

    generic(gvar(First), gvar(Second)) struct Pair
    {
        First first;
        Second second;
    };

    generic(gvar(T), gvvar(u32, N)) struct HashMap {
        u32 bucketIndex(const u32 hash) {
            return hash % N;
        }

        Optional<T> get(const u32 hash) {
            return values[bucketIndex(hash)];
        }

        mutating b32 insert(const u32 hash, const T value) {
            let idx = bucketIndex(hash);
            if (values[idx].hasValue) return false;
            values[idx] = Optional<T>(value);
            return true;
        }

        Optional<T> values[N];
    };
}


#ifdef __SLANG__
//[[vk::binding(0, 0)]]
__DynamicResource<__DynamicResourceKind::General> __global_resource_array[];
#endif
// Host should set index for GPU texture array

namespace dyslang
{
#ifdef __cplusplus
    template <typename T> struct Texture1D { };
    template <typename T> struct Texture2D { };

	// todo limit to resource types
    template <typename T>
    concept resource = !always_false_v<T>;
#elif __SLANG__
    slang_internal typealias resource = __IDynamicResourceCastable<__DynamicResourceKind::General>;
#endif

    struct ResourceRefBase {
        dyslang::i32 _idx;
        bool has() { return _idx >= 0; }
    };
#ifdef __cplusplus
    struct ResourceRef : ResourceRefBase
#elif __SLANG__
    generic(gtvar(resource, T)) slang_internal struct ResourceRef : ResourceRefBase
#endif
    {
#ifdef __cplusplus
        dyslang::i32 get() { return _idx; }
#elif __SLANG__
    	T get() { return __global_resource_array[_idx].as<T>(); }
#endif
    };

#if __SLANG__
    generic(gvar(T)) slang_internal typealias Texture2DRef = dyslang::ResourceRef<Texture2D<T>>;
    generic(gvar(T)) slang_internal typealias Sampler2DRef = dyslang::ResourceRef<Sampler2D<T>>;
#endif

    slangInterfaceUUID(
        internal,
        IProperties,
        UUID(A,2,F,5,4,8,6,6, 7,A,E,F, 4,9,0,5, B,4,C,E, 4,7, A,C, 7,3, C,A, 3,C, 0,7)
    )
        vbegin(dyslang::b32) has_property(dyslang::CString) vend;
		vbegin(void) get(dyslang::CString, dyslang::f32**, uint64_t*) vend;
        vbegin(void) get(dyslang::CString, dyslang::f64**, uint64_t*) vend;
        vbegin(void) get(dyslang::CString, dyslang::ResourceRefBase**, uint64_t*) vend;

        vbegin(void) set(dyslang::CString, const dyslang::f32*, uint64_t) vend;
        vbegin(void) set(dyslang::CString, const dyslang::f64*, uint64_t) vend;
        vbegin(void) set(dyslang::CString, const dyslang::ResourceRefBase*, uint64_t) vend;
	};

#ifdef __SLANG_CPP__

namespace __private {
    void get<T>(dyslang::CString key, T** value, dyslang::IProperties properties) {
        __requirePrelude(R"(
                #include <type_traits>
                #include <stdexcept>

                template <typename T> struct is_vector : std::false_type {};
                template <typename T, size_t N> struct is_vector<Vector<T, N>> : std::true_type {};
                
                template <typename T> struct vector_info {};
                template <typename T, size_t N> struct vector_info<Vector<T, N>> {
                    using type = T;
                    static constexpr size_t size = N;
                };
				
				template <typename T, typename PROPERTIES_T> 
                void getProperty(const char* key, T** value, PROPERTIES_T& props){
                    
					uint64_t count;
					props->get(key, value, &count);
				}
				template <typename T, int N, typename PROPERTIES_T> 
                void getProperty(const char* key, Vector<T, N>** value, PROPERTIES_T& props){	
                    	
					uint64_t count;
					props->get(key, (T**)value, &count);
				}
			
            )");
        __intrinsic_asm R"(getProperty($0, $1, $2))";
    }

    void set<T>(dyslang::CString key, T value, dyslang::IProperties properties) {
        __requirePrelude(R"(
			template <typename T, typename PROPERTIES_T> 
            void setProperty(const char* key, T& value, PROPERTIES_T& props){
                props->set(key, &value, 1);
            }
			template <typename T, int N, typename PROPERTIES_T>
            void setProperty(const char* key, Vector<T, N>& value, PROPERTIES_T& props){
                props->set(key, (T*)&value, N);
            }
            template <typename T, size_t N, typename PROPERTIES_T> 
            void setProperty(const char* key, FixedArray<T, N>& value, PROPERTIES_T& props){
                props->set(key, (T*)&value, N);
            }
        )");
        __intrinsic_asm R"(setProperty($0, $1, $2))";
    }
}
#endif
#ifdef __SLANG__

struct Properties {
    dyslang::IProperties __properties;
    bool has(dyslang::CString key) {
#ifdef __SLANG_CPP__
        return (bool)__properties.has_property(key);
#else
        return false;
#endif
    }

    T get<T>(dyslang::CString key) {
#ifdef __SLANG_CPP__
        T* ptr = nullptr;
        __private::get<T>(key, &ptr, __properties);
        return *ptr;
#else
        return {};
#endif
    }

    void set<T>(dyslang::CString key, T value) {
#ifdef __SLANG_CPP__
        __private::set<T>(key, value, __properties);
#endif
    }

    dyslang::ResourceRef<T> getResourceRef<T : __IDynamicResourceCastable<__DynamicResourceKind::General>>(dyslang::CString key) {
#ifdef __SLANG_CPP__
        dyslang::ResourceRefBase* ptr = nullptr;
        __private::get<dyslang::ResourceRefBase>(key, &ptr, __properties);
        return dyslang::ResourceRef<T>( ptr->_idx );
#else
        return {};
#endif
    }
    dyslang::Texture2DRef<T> getTexture2DRef<T>(dyslang::CString key) { return getResourceRef<Texture2D<T>>(key); }
	dyslang::Sampler2DRef<T> getSampler2DRef<T>(dyslang::CString key) { return getResourceRef<Sampler2D<T>>(key); }

    void setResourceRef<T : __IDynamicResourceCastable<__DynamicResourceKind::General>>(dyslang::CString key, dyslang::ResourceRef<T> value) {
#ifdef __SLANG_CPP__
        __private::set<dyslang::ResourceRefBase>(key, value, __properties);
#endif
    }
};

#elif __cplusplus
    struct Properties : public IProperties
    {
        using SupportedTypes = std::tuple<
            std::vector<f32>,
            std::vector<f64>,
            std::vector<dyslang::ResourceRefBase>
        >;
        using VariantType = tuple_to_variant<SupportedTypes>::type;
        // We don't need queryInterface for this impl, or ref counting
        vbegin(SlangResult) queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE { return SLANG_E_NOT_IMPLEMENTED; }
        vbegin(uint32_t) addRef() SLANG_OVERRIDE { return 1; }
        vbegin(uint32_t) release() SLANG_OVERRIDE { return 1; }

        // Properties
        vbegin(dyslang::b32) has_property(const dyslang::CString key) SLANG_OVERRIDE { return properties.contains(key); }
        vbegin(void) get(const dyslang::CString key, dyslang::f32** ptr, uint64_t* count) SLANG_OVERRIDE {
            auto& value = std::get<std::vector<dyslang::f32>>(properties[key]);
            *count = value.size();
            *ptr = value.data();
        }
    	vbegin(void) get(const dyslang::CString key, dyslang::f64** ptr, uint64_t* count) SLANG_OVERRIDE {
            auto& value = std::get<std::vector<dyslang::f64>>(properties[key]);
			*count = value.size();
			*ptr = value.data();
        }
        vbegin(void) get(const dyslang::CString key, dyslang::ResourceRefBase** ptr, uint64_t* count) SLANG_OVERRIDE {
            auto& value = std::get<std::vector<dyslang::ResourceRefBase>>(properties[key]);
            *count = value.size();
            *ptr = value.data();
        }

    
        vbegin(void) set(const dyslang::CString key, const dyslang::f32* ptr, const uint64_t count) SLANG_OVERRIDE { properties[key] = std::vector<dyslang::f32>{ ptr, ptr + count }; }
        vbegin(void) set(const dyslang::CString key, const dyslang::f64* ptr, const uint64_t count) SLANG_OVERRIDE { properties[key] = std::vector<dyslang::f64>{ ptr, ptr + count }; }
        vbegin(void) set(const dyslang::CString key, const dyslang::ResourceRefBase* ptr, const uint64_t count) SLANG_OVERRIDE { properties[key] = std::vector<dyslang::ResourceRefBase>{ ptr, ptr + count }; }

        template <typename T> void set(const dyslang::CString key, const T& data) { set(key, &data, 1); }
    	template <typename T> void set(const dyslang::CString key, const std::vector<T>& data) { set(key, data.data(), data.size()); }
        template <typename T, size_t N> void set(const dyslang::CString key, const std::array<T, N>& data) { set(key, data.data(), N); }

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

        [[nodiscard]] std::string to_string() const {
            std::string result = "Properties:\n";
            for (const auto& [key, value] : properties) {
                result += " ";
                result += key;
                result += ": ";
                std::visit([&result](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (is_arithmetic_v<T>)
                        result += std::to_string(arg);
                    else if constexpr (std::is_same_v<std::vector<dyslang::ResourceRefBase>, T>) {
                        std::string sep;
                        for (const auto& v : arg) {
                            result += sep + "ResourceRef{ index=" + std::to_string(v._idx) + " }";
                            sep = ", ";
                        }
                    }
                    else {
						std::string sep;
                        for(const auto& v : arg) {
							result += sep + std::to_string(v);
                            sep = ", ";
						}
                    }
                }, value);
                result += "\n";
            }
            return result;
        }
        struct cmp_str
        {
            bool operator()(char const* a, char const* b) const { return std::strcmp(a, b) < 0; }
        };
        std::map<const char* , VariantType, cmp_str> properties;
    };
#endif
}
