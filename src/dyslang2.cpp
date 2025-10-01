#include <functional>
#include <dyslang/dyslang2.h>
#include <slang.h>
#include <dyslang/slangc.h>

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
    static const std::string s_props = R"(
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

    void DynamicObject::traverse(IProperties& props)
    {
        traverseFunc(data.data(), &props);
    }

    Properties DynamicObject::traverse()
    {
        Properties props;
        traverse(props);
        return props;
    }

    DynamicClass::DynamicClass(std::string filepath, std::string classname, std::string interfacename) :
        _filepath{ std::move(filepath) }, _classname{ std::move(classname) }, _interfacename{ std::move(interfacename) } {

        static uint32_t globalID = 1;
        if (has_interface()) _id = globalID++;

        Slang::ComPtr<slang::IGlobalSession> slangSession;
        slangSession.attach(spCreateSession(nullptr));

        if (!slangGlobalSession) {
            if (SLANG_FAILED(slang::createGlobalSession(slangGlobalSession.writeRef()))) throw std::runtime_error("slang: error creating global session");
        }

        std::vector<slang::CompilerOptionEntry> copts{
            {.name = slang::CompilerOptionName::LanguageVersion, .value = {slang::CompilerOptionValueKind::Int, 2026}},
            {.name = slang::CompilerOptionName::EnableExperimentalDynamicDispatch, .value = {slang::CompilerOptionValueKind::Int, 1}}
        };

        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SHADER_SHARED_LIBRARY;

        slang::SessionDesc sessionDesc = {};
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;
        sessionDesc.compilerOptionEntries = copts.data();
        sessionDesc.compilerOptionEntryCount = static_cast<uint32_t>(copts.size());

        Slang::ComPtr<slang::ISession> _session;
        if (SLANG_FAILED(slangGlobalSession->createSession(sessionDesc, _session.writeRef()))) throw std::runtime_error("slang: error creating session");

        Slang::ComPtr<slang::ICompileRequest> request;
        _session->createCompileRequest(request.writeRef());
        request->setTargetFlags(0, SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM);
        const int translationUnitIndex = request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
        request->addTranslationUnitSourceFile(translationUnitIndex, _filepath.c_str());

        std::string source = iprops + getter + setter + s_props + "\n\
        void __memcpy<T>(Ptr<void> ptr, T data, uint size) { __intrinsic_asm R\"(memcpy($0, &$1, $2))\"; }\n\
		export __extern_cpp uint __size() { return sizeof(" + _classname + "); }\n\
    	export __extern_cpp void __init(Ptr<" + _classname + "> ptr, IPropertiesInternal props) { *ptr = " + _classname + "(Properties(props)); }\n\
		export __extern_cpp void __traverse(Ptr<uint8_t> ptr, IPropertiesInternal props) { ((" + _classname + "*)ptr)->traverse(Properties(props)); }\n";

        if (!_interfacename.empty()) {
            source += "\
                export __extern_cpp uint __sizeDynamic() { return sizeof(" + _interfacename + "); }\n\
		        export __extern_cpp void __initDynamic(Ptr<" + _interfacename + "> ptr, IPropertiesInternal props) { *ptr = createDynamicObject<" +
                _interfacename + ", " + _classname + ">(" + std::to_string(_id.value()) + ", " + _classname + "(Properties(props))); }\n\
                export __extern_cpp void __traverseDynamic(Ptr<uint8_t> ptr, IPropertiesInternal props) { ((" + _classname + "*)(ptr+16))->traverse(Properties(props)); }\n";
        }
        request->addTranslationUnitSourceString(translationUnitIndex, "__internal", source.c_str());

        const SlangResult compileRes = request->compile();
        if (auto diagnostics = request->getDiagnosticOutput())
            printf("%s", diagnostics);

        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        {
            SlangResult _res = request->getTargetHostCallable(0, _library.writeRef());
            if (SLANG_FAILED(_res))
            {
                assert(false);
            }
        }
    }

    DynamicObject DynamicClass::init(IProperties& props)
    {
        if (!_interfacename.empty()) assert(_id != static_cast<uint32_t>(-1));
        else assert(!_id);

        _sizeFunc = (uint32_t(*)(void))_library->findFuncByName("__size");
        _dynamicSizeFunc = (uint32_t(*)(void))_library->findFuncByName("__sizeDynamic");

        void (*add_ptr)() = _library->findFuncByName("add");

        if (has_interface()) {
            auto initDynamicFunc = (void (*)(uint8_t*, IProperties*))_library->findFuncByName("__initDynamic");
            std::vector<uint8_t> dataDynamic(_dynamicSizeFunc());
            initDynamicFunc(dataDynamic.data(), &props);
            _traverseFunc = (void (*)(uint8_t*, IProperties*))_library->findFuncByName("__traverseDynamic");
            return { .data = dataDynamic, .traverseFunc = _traverseFunc, ._library = _library };
        } else {
            std::vector<uint8_t> data(_sizeFunc());
            std::function<void(uint8_t*, IProperties*)> initFunc = (void (*)(uint8_t*, IProperties*))_library->findFuncByName("__init");
            initFunc(data.data(), &props);
            _traverseFunc = (void (*)(uint8_t*, IProperties*))_library->findFuncByName("__traverse");
            return { .data = data, .traverseFunc = _traverseFunc, ._library = _library };
        }
    }

    size_t DynamicClass::size() const
    {
        if (has_interface()) return _dynamicSizeFunc();
        return _sizeFunc();
    }

    bool DynamicClass::has_interface() const
    {
        return !_interfacename.empty();
    }
}
