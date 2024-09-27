#include <dyslang/slangc.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <slang.h>
#include <slang-com-ptr.h>

using namespace dyslang;
namespace {
    thread_local Slang::ComPtr<slang::IGlobalSession> globalSession;

    void checkError(slang::IBlob* diagnosticsBlob)
    {
        if (diagnosticsBlob != nullptr)
        {
            printf("%s", static_cast<const char*>(diagnosticsBlob->getBufferPointer()));
        }
        diagnosticsBlob = nullptr;
    }

    std::string bytesToString(const void* data, const size_t len)
    {
	    const auto p = static_cast<const uint8_t*>(data);
        std::stringstream ss;
        ss << std::hex;

        for (size_t i = 0; i < len; ++i)
            ss << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(p[i]);

        return ss.str();
    }
}

struct Slangc::SlangcPrivate
{
    Slang::ComPtr<slang::ISession> _session;
    Slang::ComPtr<slang::IBlob> _diagnosticsBlob;
    std::unordered_map<std::string_view, slang::IComponentType*> _modules;
    std::vector<slang::IComponentType*> _entryPoints;
    std::vector<slang::ITypeConformance*> _typeConformances;
    std::vector<slang::IComponentType*> _programComponents;
    Slang::ComPtr<slang::IComponentType> _tempProgram;
    Slang::ComPtr<slang::IComponentType> _composedProgram;
};

Slangc::Slangc(const std::vector<const char*>& includes, const std::vector<Slangc::ArgPair>& defines) : _p{ new SlangcPrivate() }
{
    if (!globalSession) {
        if (SLANG_FAILED(slang::createGlobalSession(globalSession.writeRef()))) throw std::runtime_error("slang: error creating global session");
    }

    slang::SessionDesc sessionDesc = {};
    slang::TargetDesc targetDesc = {};
    if (true) {
        targetDesc.format = SLANG_GLSL;
        targetDesc.profile = globalSession->findProfile("glsl460");
    }
    else {
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = globalSession->findProfile("spirv_1_5");
        targetDesc.flags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;
    }
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.allowGLSLSyntax = true;
    sessionDesc.searchPathCount = static_cast<SlangInt>(includes.size());
    sessionDesc.searchPaths = includes.data();
    sessionDesc.preprocessorMacroCount = static_cast<SlangInt>(defines.size());
    sessionDesc.preprocessorMacros = reinterpret_cast<const slang::PreprocessorMacroDesc*>(defines.data());

    if (SLANG_FAILED(globalSession->createSession(sessionDesc, _p->_session.writeRef()))) throw std::runtime_error("slang: error creating session");
}

Slangc::~Slangc()
{
    delete _p;
    _p = nullptr;
}

void Slangc::addModule(const std::string_view moduleName) const
{
    slang::IModule* module = _p->_session->loadModule(moduleName.data(), _p->_diagnosticsBlob.writeRef());
    checkError(_p->_diagnosticsBlob);
    if (!module) throw std::runtime_error("slang: module null");
    _p->_modules[moduleName] = module;
}

void Slangc::addModule(std::string_view moduleName, std::string_view modulePath, const void* blob) const {
    slang::IModule* module = _p->_session->loadModuleFromIRBlob(moduleName.data(), modulePath.data(), (slang::IBlob*)blob, _p->_diagnosticsBlob.writeRef());
    checkError(_p->_diagnosticsBlob);
    if (!module) throw std::runtime_error("slang: module null");
    _p->_modules[moduleName] = module;
}

void Slangc::addEntryPoint(const std::string_view moduleName, const std::string_view entryPointName) const
{
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    dynamic_cast<slang::IModule*>(_p->_modules.at(moduleName))->findEntryPointByName(entryPointName.data(), entryPoint.writeRef());
    if (!entryPoint) throw std::runtime_error("slang: entrypoint null");
    _p->_entryPoints.push_back(entryPoint);
}

void Slangc::finalizeModulesAndEntryPoints() const
{
    Slang::ComPtr<slang::IComponentType> tempProgram;

    std::vector<slang::IComponentType*> componentTypes;
    std::vector<slang::IComponentType*> modules;
    modules.reserve(_p->_modules.size());

    for (const auto& kv : _p->_modules) modules.push_back(kv.second);

    componentTypes.insert(componentTypes.end(), modules.begin(), modules.end());
    componentTypes.insert(componentTypes.end(), _p->_entryPoints.begin(), _p->_entryPoints.end());

    const SlangResult result = _p->_session->createCompositeComponentType(
        componentTypes.data(),
        componentTypes.size(),
        tempProgram.writeRef(),
        _p->_diagnosticsBlob.writeRef());
    checkError(_p->_diagnosticsBlob);
    if (SLANG_FAILED(result)) throw std::runtime_error("slang: composition error");

    _p->_modules.clear();
    _p->_entryPoints.clear();
    _p->_programComponents.clear();

    _p->_programComponents.push_back(tempProgram);
    _p->_tempProgram = tempProgram;
}

std::pair<unsigned, unsigned> Slangc::globalResourceArrayBinding() const
{
	auto layout = _p->_tempProgram->getLayout(0, _p->_diagnosticsBlob.writeRef());
	checkError(_p->_diagnosticsBlob);
    if (!layout) throw std::runtime_error("slang: layout null");
    unsigned parameterCount = layout->getParameterCount();
    for (unsigned pp = 0; pp < parameterCount; pp++)
    {
        slang::VariableLayoutReflection* parameter = layout->getParameterByIndex(pp);
		const std::string name = parameter->getName();
        if (name == "__global_resource_array")
        {
            unsigned binding = parameter->getBindingIndex();
            unsigned space = parameter->getBindingSpace();
            return { binding, space };
        }
        int x = 2;
    }
    throw std::runtime_error("slang: __global_resource_array not found");
}

void Slangc::addTypeConformance(const std::string_view type, const std::string_view conformance, int64_t id_override) const
{
    slang::ProgramLayout* layout = _p->_tempProgram->getLayout();
    slang::TypeReflection* t = layout->findTypeByName(type.data());
    slang::TypeReflection* c = layout->findTypeByName(conformance.data());

    slang::ITypeConformance* t_c;
    const SlangResult result = _p->_session->createTypeConformanceComponentType(
        c,
        t,
        &t_c,
        id_override,
        _p->_diagnosticsBlob.writeRef());
    checkError(_p->_diagnosticsBlob);
    if (SLANG_FAILED(result)) throw std::runtime_error("slang: type conformance error");
    _p->_typeConformances.push_back(t_c);
}

Slangc::Hash Slangc::compose() const
{
    _p->_programComponents.insert(_p->_programComponents.end(), _p->_typeConformances.begin(), _p->_typeConformances.end());

    const SlangResult result = _p->_session->createCompositeComponentType(
        _p->_programComponents.data(),
        _p->_programComponents.size(),
        _p->_composedProgram.writeRef(),
        _p->_diagnosticsBlob.writeRef());
    checkError(_p->_diagnosticsBlob);
    if (SLANG_FAILED(result)) throw std::runtime_error("slang: composition error");

    Slang::ComPtr<slang::IBlob> hash;
    _p->_composedProgram->getEntryPointHash(0, 0, hash.writeRef());
    return bytesToString(hash->getBufferPointer(), hash->getBufferSize());
}

std::vector<uint8_t> Slangc::compile() const
{
    Slang::ComPtr<slang::IBlob> spirvCode;
    const SlangResult result = _p->_composedProgram->getEntryPointCode(
        0, 0, spirvCode.writeRef(), _p->_diagnosticsBlob.writeRef()
    );
    checkError(_p->_diagnosticsBlob);
    if (result != 0) return {};

    std::vector<uint8_t> out(spirvCode->getBufferSize(), 0);
    std::memcpy(out.data(), spirvCode->getBufferPointer(), out.size());
    return out;
}
