#include <functional>
#include <dyslang/dyslang2.h>
#include <slang.h>

namespace dyslang2
{
    static const std::string getPrelude = R"(
        #include <type_traits>
        #include <stdexcept>
		#include <iostream>
		#include <any>

        template <typename T> struct is_vector : std::false_type {};
        template <typename T, size_t N> struct is_vector<Vector<T, N>> : std::true_type {};
        
        template <typename T> struct vector_info {};
        template <typename T, size_t N> struct vector_info<Vector<T, N>> {
            using type = T;
            static constexpr size_t size = N;
        };
		
		template <typename T, typename PROPERTIES_T> 
        T getProperty(const char* key, T dummy, PROPERTIES_T& props){
			uint64_t count;
			std::any* value = (std::any*)props->get(key, &count);
			assert(count == 1);
            return *std::any_cast<T*>(*value);
        }
		template <typename T, int N, typename PROPERTIES_T> 
        Vector<T, N> getProperty(const char* key, Vector<T, N> dummy, PROPERTIES_T& props){
			uint64_t count;
			std::any* value = (std::any*)props->get(key, &count);
			assert(count == N);
			return *((Vector<T, N>*)std::any_cast<T*>(*value));
        }
		template <typename T, size_t N, typename PROPERTIES_T> 
        FixedArray<T, N> getProperty(const char* key, FixedArray<T, N> dummy, PROPERTIES_T& props){
			uint64_t count;
			std::any* value = (std::any*)props->get(key, &count);
			assert(count == N);
			return *((FixedArray<T, N>*)std::any_cast<T*>(*value));
        }
/*
		template <typename T, int ROWS, int COLS, typename PROPERTIES_T> 
        Matrix<T, ROWS, COLS> getProperty(const char* key, Matrix<T, ROWS, COLS> dummy, PROPERTIES_T& props){	
			Matrix<T, ROWS, COLS>* value;
            uint64_t count;
			props->get(key, (T**)&value, &count);
            if (count != (ROWS * COLS)) std::cout << "Warning <dyslang>: \'" << key << "\' Property size mismatch" << std::endl;
            return *value;
        }*/
    )";

    static const std::string getter = "\
    T get<T>(NativeString key, IPropertiesInternal properties) {\
        __requirePrelude(R\"(" + getPrelude +
                                      ")\");\
        __intrinsic_asm R\"(getProperty($0, $TR(), $1))\";\
    }";

    static const std::string setPrelude = R"(
		#include <any>
		template <typename T, typename PROPERTIES_T> 
        void setProperty(const char* key, T* value, PROPERTIES_T& props){
			std::any a = value;
            props->set(key, (void*)&a, 1);
        }
		template <typename T, int N, typename PROPERTIES_T>
        void setProperty(const char* key, Vector<T, N>* value, PROPERTIES_T& props){
			std::any a = (T*)value;
            props->set(key, (void*)&a, N);
        }
        template <typename T, size_t N, typename PROPERTIES_T> 
        void setProperty(const char* key, FixedArray<T, N>* value, PROPERTIES_T& props){
			std::any a = (T*)value;
            props->set(key, (void*)&a, N);
        }
		//template <typename T, int ROWS, int COLS, typename PROPERTIES_T> 
  //      void setProperty(const char* key, Matrix<T, ROWS, COLS>& value, PROPERTIES_T& props){
  //          props->set(key, (T*)&value, ROWS * COLS);
  //      }
    )";

    static const std::string setter = "\
    void set<T>(NativeString key, Ptr<T> value, IPropertiesInternal properties) {\
        __requirePrelude(R\"(" + setPrelude +
                                      ")\");\
        __intrinsic_asm R\"(setProperty($0, $1, $2))\";\
    };";

    static const std::string iprops = R"(
		[COM("A2F54866-7AEF-4905-B4CE-47AC73CA3C07")]
		interface IPropertiesInternal {
		    uint32_t has_property(NativeString);
            void set(NativeString, const Ptr<void>, const uint64_t);
            Ptr<void> get(NativeString, uint64_t*);
		};
	)";
    static const std::string props = R"(
		struct Properties : IProperties {
	        private IPropertiesInternal __properties;
	        __init(IPropertiesInternal properties) {
	            __properties = properties;
	        }
	        bool has(NativeString key) {
	            return (bool)__properties.has_property(key);
	        }
		    void set<T>(NativeString key, Ptr<T> value) {
				set<T>(key, value, __properties);
		    }
            T get<T>(NativeString key) {
                return get<T>(key, __properties);
			}
		};
	)";

    void checkError(slang::IBlob *diagnostics_blob)
    {
        if (diagnostics_blob != nullptr)
        {
            std::printf("%s", static_cast<const char *>(diagnostics_blob->getBufferPointer()));
        }
    }

    DynamicClass::DynamicClass(const std::string &filename, const std::string &name)
    {
        Slang::ComPtr<slang::IGlobalSession> slangSession;
        slangSession.attach(spCreateSession(nullptr));

        Slang::ComPtr<slang::ICompileRequest> request;
        SLANG_RETURN_VOID_ON_FAIL(slangSession->createCompileRequest(request.writeRef()))

        const int targetIndex = request->addCodeGenTarget(SLANG_SHADER_HOST_CALLABLE);
        request->setTargetFlags(targetIndex, SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM);

        const int translationUnitIndex = request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, nullptr);

        // Set the source file for the translation unit
        request->addTranslationUnitSourceFile(translationUnitIndex, filename.c_str());

        std::string source = iprops + getter + setter + props + "\n\
        void __memcpy<T>(Ptr<void> ptr, T data, uint size) { __intrinsic_asm R\"(memcpy($0, &$1, $2))\"; }\n\
		export __extern_cpp uint __size() { return sizeof(" +
                             name + "); }\n\
    	export __extern_cpp void __init(Ptr<void> ptr) { __memcpy(ptr, " +
                             name + "(), __size()); }\n\
		export __extern_cpp void __traverse(Ptr<void> ptr, IPropertiesInternal props) { ((" +
                             name + "*)ptr)->traverse(Properties(props)); }\n";

        std::string interface = "IEmitter<float>";
        source += "\
        export __extern_cpp uint __sizeDynamic() { return sizeof(" +
                  interface + "); }\n\
		export __extern_cpp void __initDynamic(Ptr<void> ptr, uint typeID) { __memcpy(ptr, createDynamicObject<" +
                  interface + ", " + name + ">(typeID, " + name + "()), __sizeDynamic()); }\n";

        request->addTranslationUnitSourceString(translationUnitIndex, "__internal", source.c_str());

        const SlangResult compileRes = request->compile();
        if (auto diagnostics = request->getDiagnosticOutput())
            printf("%s", diagnostics);

        Slang::ComPtr<slang::IBlob> diagnosticsBlob;

        Slang::ComPtr<ISlangSharedLibrary> sharedLibrary;
        SLANG_RETURN_VOID_ON_FAIL(request->getTargetHostCallable(0, sharedLibrary.writeRef()))

        sizeFunc = (uint32_t (*)(void))sharedLibrary->findFuncByName("__size");
        const uint32_t size = sizeFunc();

        auto sizeDynamicFunc = (uint32_t (*)(void))sharedLibrary->findFuncByName("__sizeDynamic");
        const uint32_t sizeDynamic = sizeDynamicFunc();

        initFunc = (void (*)(void *))sharedLibrary->findFuncByName("__init");
        std::vector<uint8_t> data(size);
        initFunc(data.data());

        auto initDynamicFunc = (void (*)(void *, uint32_t))sharedLibrary->findFuncByName("__initDynamic");
        std::vector<uint8_t> dataDynamic(sizeDynamic);
        initDynamicFunc(dataDynamic.data(), 6);

        Properties props;
        int a = 47;
        props.set("x", &a);
        std::printf("%s", props.to_string().c_str());
        traverseFunc = (void (*)(void *, IProperties *))sharedLibrary->findFuncByName("__traverse");
        traverseFunc(data.data(), &props);
        std::printf("%s", props.to_string().c_str());

        auto vi = props.get<int>("x");
        auto v = props.get<std::vector<float>>("b");

        void (*add_ptr)() = sharedLibrary->findFuncByName("add");
    }
}