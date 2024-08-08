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

    Slang::ComPtr<slang::ISession> session;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.searchPathCount = static_cast<SlangInt>(includes.size());
    sessionDesc.searchPaths = includes.data();

    if (SLANG_FAILED(globalSession->createSession(sessionDesc, session.writeRef()))) exitWithError("Error creating slang session");
    
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IModule* module = session->loadModuleFromSourceString(source.module.c_str(), source.path.c_str(), source.source.c_str(), diagnosticsBlob.writeRef());
    checkError(diagnosticsBlob);
	if(!module) exitWithError("Error loading slang module");

	Slang::ComPtr<slang::IBlob> blob;
    if(SLANG_FAILED(module->serialize(blob.writeRef()))) exitWithError("Error serializing slang module");
    return blob;
}

Slang::ComPtr<slang::IBlob> compilerSharedLib(const Slang::ComPtr<slang::IGlobalSession>& globalSession, const SourceFile& source) {
    SlangCompileRequest* slangRequest = spCreateCompileRequest(globalSession);
    int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_SHADER_SHARED_LIBRARY);
    spAddPreprocessorDefine(slangRequest, "__SLANG_CPP__", "1");
    spSetTargetFlags(slangRequest, targetIndex, SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM);
    int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
    spAddTranslationUnitSourceString(slangRequest, translationUnitIndex, source.module.c_str(), source.source.c_str());
    const SlangResult compileRes = spCompile(slangRequest);
    if(auto diagnostics = spGetDiagnosticOutput(slangRequest)) printf("%s", diagnostics);

    if(SLANG_FAILED(compileRes)) {
        spDestroyCompileRequest(slangRequest);
        exitWithError("Error compiling slang module to shared library");
    }

    Slang::ComPtr<slang::IBlob> blob;
    if(SLANG_FAILED(spGetTargetCodeBlob(slangRequest, 0, blob.writeRef()))){
        exitWithError("Error getting shared library from slang request");
    }
    return blob;
}

int main(const int argc, char* argv[]) {
    if(argc < 2) {
        std::string executable = std::filesystem::path{ argv[0] }.filename().string();
        std::cout << "Usage: " << executable << " <path-to-slang-file>" << '\n';
        return 1;
    }

    auto outputDir = std::filesystem::current_path();
    std::filesystem::current_path(std::filesystem::path{ argv[1] }.parent_path());
    std::cout << "Working Dir: " << std::filesystem::current_path() << '\n';
    std::cout << "argc: " << argc << '\n';
    std::cout << "argv: " << argv[1] << '\n';

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    Slang::ComPtr<slang::ISession> session;
    if (!globalSession) {
        if (SLANG_FAILED(slang::createGlobalSession(globalSession.writeRef()))) exitWithError("Error creating global slang session");
    }

    auto source = loadFileToString(argv[1]);

    // compile slang module
    const auto slangModuleBlob = compileSlangModule(globalSession, source);
    const auto dataString = copyBlobToString(slangModuleBlob);
    {
        const std::string moduleFilename = outputDir.string() + "/" + source.module + ".slang-module";
        writeStringToFile(dataString, moduleFilename);
        std::cout << "Slang Module (" << slangModuleBlob->getBufferSize() << " bytes): " << std::filesystem::current_path().append(moduleFilename) << '\n';
    }

    // add slang module ir to source
    //source.source += "\nexport __extern_cpp NativeString __slang_module_ir() { return \"" + std::string("IR") + "\"; }";
    // std::cout << "Source: " << source.source << '\n';

    // compile shared lib for host
    {
        const auto sharedLibBlob = compilerSharedLib(globalSession, source);
        const auto dataString = copyBlobToString(sharedLibBlob);
        const std::string sharedLibFilename = outputDir.string() + "/" + source.module + sharedLibExt;
        writeStringToFile(dataString, sharedLibFilename);
        std::cout << "Shared Lib (" << sharedLibBlob->getBufferSize() << " bytes): " << std::filesystem::current_path().append(sharedLibFilename) << '\n';
    }

    return 0;
}
