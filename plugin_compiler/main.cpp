#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <sstream>
#include <slang.h>
#include <slang-com-ptr.h>

#ifdef _WIN32
auto sharedLibExt = ".dll";
#elif __APPLE__
auto sharedLibExt = ".dylib";
#else // linux
#include <cstring>
auto sharedLibExt = ".so";
#endif

namespace {
    // exit with error message
    void exitWithError(const std::string& message)
    {
        std::cerr << message << '\n';
        exit(1);
    }

    void checkError(slang::IBlob* diagnosticsBlob)
    {
        if (diagnosticsBlob != nullptr)
        {
            printf("%s", static_cast<const char*>(diagnosticsBlob->getBufferPointer()));
        }
        diagnosticsBlob = nullptr;
    }

    struct SourceFile {
        std::string module;
        std::string path;
        std::string source;
    };

    void writeStringToFile(const std::string& str, const std::filesystem::path& path)
    {
        std::ofstream ofs{ path, std::ofstream::binary | std::ios::out };
        if (ofs.fail()) exitWithError("Failed to open file: " + path.string());
        ofs.write(str.c_str(), str.length());
        ofs.close();
    }

    SourceFile loadFileToString(const std::filesystem::path& path)
    {
        SourceFile result;
        std::ifstream ifs{ path };
        if (ifs.fail()) exitWithError("Failed to open file: " + path.string());
        std::stringstream buffer;
        buffer << ifs.rdbuf();
        result.source = buffer.str();
        result.path = path.parent_path().string();
        result.module = path.stem().string();
        return result;
    }

    std::string copyBlobToString(const Slang::ComPtr<slang::IBlob>& blob)
    {
        std::string result(blob->getBufferSize(), 0);
        std::memcpy(result.data(), blob->getBufferPointer(), result.size());
        return result;
    }
}

Slang::ComPtr<slang::IBlob> compileSlangModule(const Slang::ComPtr<slang::IGlobalSession>& globalSession, const SourceFile& source)
{
    const std::vector includes = { source.path.c_str() };
    std::vector<slang::CompilerOptionEntry> copts{
        {.name = slang::CompilerOptionName::LanguageVersion, .value = {slang::CompilerOptionValueKind::Int, 2026}}
    };

    Slang::ComPtr<slang::ISession> session;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.searchPathCount = static_cast<SlangInt>(includes.size());
    sessionDesc.searchPaths = includes.data();
    sessionDesc.compilerOptionEntries = copts.data();
    sessionDesc.compilerOptionEntryCount = static_cast<uint32_t>(copts.size());

    slang::PreprocessorMacroDesc macros[] = {
        { "__DYSLANG__", "1" }
    };
    sessionDesc.preprocessorMacroCount = static_cast<SlangInt>(std::size(macros));
    sessionDesc.preprocessorMacros = macros;

    if (SLANG_FAILED(globalSession->createSession(sessionDesc, session.writeRef()))) exitWithError("Error creating slang session");

    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IModule* module = session->loadModuleFromSourceString(source.module.c_str(), source.path.c_str(), source.source.c_str(), diagnosticsBlob.writeRef());
    checkError(diagnosticsBlob);
    if (!module) exitWithError("Error loading slang module");

    Slang::ComPtr<slang::IBlob> blob;
    if (SLANG_FAILED(module->serialize(blob.writeRef()))) exitWithError("Error serializing slang module");
    return blob;
}

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

// -D__DYSLANG_VARIANTS__="(float_rgb,(<float>))"
std::string create_variant_define(std::string variants) {
    std::erase_if(variants, isspace);
    auto variant_list = split(variants, ';');

    std::string define, sep;
    for (const auto& variant : variant_list) {
        auto v = split(variant, ':');
        define += sep + "(" + v[0] + ", (" + (v.size() == 2 ? v[1] : "") + "))";
        sep = ", ";
    }
    return define;
}

Slang::ComPtr<slang::IBlob> compilerSharedLib(const Slang::ComPtr<slang::IGlobalSession>& globalSession, const SourceFile& source, const char* variants) {
    //std::string define = create_variant_define("float_rgb:float; double_rgb:double"); //"(float_rgb, float), (double_rgb, double)"
    std::string define = create_variant_define(variants);
    //std::cout << "Variants: " << define << '\n';

    SlangCompileRequest* slangRequest = spCreateCompileRequest(globalSession);
    int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_SHADER_SHARED_LIBRARY);
    spAddPreprocessorDefine(slangRequest, "__DYSLANG__", "1");
    spAddPreprocessorDefine(slangRequest, "__SLANG_CPP__", "1");
    spAddPreprocessorDefine(slangRequest, "__DYSLANG_VARIANTS__", define.c_str());
    
    spSetTargetFlags(slangRequest, targetIndex, SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM);
    int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
    spAddTranslationUnitSourceString(slangRequest, translationUnitIndex, source.module.c_str(), source.source.c_str());
    const SlangResult compileRes = spCompile(slangRequest);
    if (auto diagnostics = spGetDiagnosticOutput(slangRequest)) printf("%s", diagnostics);

    if (SLANG_FAILED(compileRes)) {
        spDestroyCompileRequest(slangRequest);
        exitWithError("Error compiling slang module to shared library");
    }

    Slang::ComPtr<slang::IBlob> blob;
    if (SLANG_FAILED(spGetTargetCodeBlob(slangRequest, 0, blob.writeRef()))) {
        exitWithError("Error getting shared library from slang request");
    }
    return blob;
}


int main(const int argc, char* argv[]) {
    if (argc < 3) {
        std::string executable = std::filesystem::path{ argv[0] }.filename().string();
        std::cout << "Usage: " << executable << " <path-to-slang-file> <variants>" << '\n';
        return 1;
    }

    auto outputDir = std::filesystem::current_path();
    std::filesystem::current_path(std::filesystem::path{ argv[1] }.parent_path());
    std::cout << "Working Dir: " << std::filesystem::current_path() << '\n';
    std::cout << "argc: " << argc << '\n';
    std::cout << "argv: " << argv[1] << " " << argv[2] << '\n';

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    if (!globalSession) {
        if (SLANG_FAILED(slang::createGlobalSession(globalSession.writeRef()))) exitWithError("Error creating global slang session");
    }

    auto source = loadFileToString(argv[1]);
    // compile slang module
    const auto slangModuleBlob = compileSlangModule(globalSession, source);
    const auto dataString = copyBlobToString(slangModuleBlob);
    // explicit write to slang module file
    /*{
        const std::string moduleFilename = outputDir.string() + "/" + source.module + ".slang-module";
        writeStringToFile(dataString, moduleFilename);
        std::cout << "Slang Module (" << slangModuleBlob->getBufferSize() << " bytes): " << std::filesystem::current_path().append(moduleFilename) << '\n';
    }*/

    std::string out, sep;
    for (size_t i = 0; i < slangModuleBlob->getBufferSize(); i++) {
        out += sep + std::to_string(static_cast<const uint8_t*>(slangModuleBlob->getBufferPointer())[i]);
        sep = ", ";
    }

    source.source += "\nexport __extern_cpp size_t __slang_module_ir_size() { return " + std::to_string(slangModuleBlob->getBufferSize()) + "; }";
    source.source += "\nPtr<uint8_t> __slang_module_ir_data_ptr_intern() {\
        __requirePrelude(\"static uint8_t __slang_module_ir_data[" + std::to_string(slangModuleBlob->getBufferSize()) + "] = {" + out + "};\");\
        __intrinsic_asm \"&__slang_module_ir_data[0]\";\
	}\n";
    source.source += "\nexport __extern_cpp Ptr<uint8_t> __slang_module_ir_data_ptr() { return __slang_module_ir_data_ptr_intern(); }\n";
    //std::cout << "Source: " << source.source << '\n';

    // compile shared lib for host
    {
        const auto sharedLibBlob = compilerSharedLib(globalSession, source, argv[2]);
        const auto dataString = copyBlobToString(sharedLibBlob);
        const std::string sharedLibFilename = outputDir.string() + "/" + source.module + sharedLibExt;
        writeStringToFile(dataString, sharedLibFilename);
        std::cout << "Shared Lib (" << sharedLibBlob->getBufferSize() << " bytes): " << std::filesystem::current_path().append(sharedLibFilename) << '\n';
    }

    return 0;
}
