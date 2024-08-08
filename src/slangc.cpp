#include <dyslang/slangc.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <slang.h>
#include <slang-com-ptr.h>

using namespace dyslang;
namespace {
    thread_local Slang::ComPtr<slang::IGlobalSession> global_session;

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
    Slang::ComPtr<slang::IBlob> _diagnostics_blob;
    std::unordered_map<std::string_view, slang::IComponentType*> _modules;
    std::vector<slang::IComponentType*> _entry_points;
    std::vector<slang::ITypeConformance*> _type_conformances;
    std::vector<slang::IComponentType*> _program_components;
    Slang::ComPtr<slang::IComponentType> _temp_program;
    Slang::ComPtr<slang::IComponentType> _composed_program;
};

Slangc::Slangc(const std::vector<const char*>& includes, const std::vector<Slangc::ArgPair>& defines) : _p{ new SlangcPrivate() }
{
    if (!global_session) {
        if (SLANG_FAILED(slang::createGlobalSession(global_session.writeRef()))) throw std::runtime_error("slang: error creating global session");
    }

    slang::SessionDesc sessionDesc = {};
    slang::TargetDesc targetDesc = {};
    if (true) {
        targetDesc.format = SLANG_GLSL;
        targetDesc.profile = global_session->findProfile("glsl460");
    }
    else {
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = global_session->findProfile("spirv_1_5");
        targetDesc.flags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;
    }
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.allowGLSLSyntax = true;
    sessionDesc.searchPathCount = static_cast<SlangInt>(includes.size());
    sessionDesc.searchPaths = includes.data();
    sessionDesc.preprocessorMacroCount = static_cast<SlangInt>(defines.size());
    sessionDesc.preprocessorMacros = reinterpret_cast<const slang::PreprocessorMacroDesc*>(defines.data());

    if (SLANG_FAILED(global_session->createSession(sessionDesc, _p->_session.writeRef()))) throw std::runtime_error("slang: error creating session");
}

Slangc::~Slangc()
{
	delete _p;
    _p = nullptr;
}

void Slangc::addModule(const std::string_view module_name) const
{
    slang::IModule* module = _p->_session->loadModule(module_name.data(), _p->_diagnostics_blob.writeRef());
    checkError(_p->_diagnostics_blob);
    if (!module) throw std::runtime_error("slang: module null");
    _p->_modules[module_name] = module;
}

void Slangc::addModule(std::string_view module_name, std::string_view module_path, const void* blob) const {
    slang::IModule* module = _p->_session->loadModuleFromIRBlob(module_name.data(), module_path.data(), (slang::IBlob*) blob, _p->_diagnostics_blob.writeRef());
    checkError(_p->_diagnostics_blob);
    if (!module) throw std::runtime_error("slang: module null");
    _p->_modules[module_name] = module;
}

void Slangc::addEntryPoint(const std::string_view module_name, const std::string_view entryPointName) const
{
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    dynamic_cast<slang::IModule*>(_p->_modules.at(module_name))->findEntryPointByName(entryPointName.data(), entryPoint.writeRef());
    if (!entryPoint) throw std::runtime_error("slang: entrypoint null");
    _p->_entry_points.push_back(entryPoint);
}

void Slangc::finalizeModulesAndEntryPoints() const
{
    Slang::ComPtr<slang::IComponentType> tempProgram;
    std::vector<slang::IComponentType*> componentTypes;
    std::vector<slang::IComponentType*> modules;
    modules.reserve(_p->_modules.size());

    for (const auto& kv : _p->_modules) modules.push_back(kv.second);

    componentTypes.insert(componentTypes.end(), modules.begin(), modules.end());
    componentTypes.insert(componentTypes.end(), _p->_entry_points.begin(), _p->_entry_points.end());

    const SlangResult result = _p->_session->createCompositeComponentType(
        componentTypes.data(),
        componentTypes.size(),
        tempProgram.writeRef(),
        _p->_diagnostics_blob.writeRef());
    checkError(_p->_diagnostics_blob);
    if (SLANG_FAILED(result)) throw std::runtime_error("slang: composition error");

    _p->_modules.clear();
    _p->_entry_points.clear();
    _p->_program_components.clear();

    _p->_program_components.push_back(tempProgram);
    _p->_temp_program = tempProgram;
}

void Slangc::addTypeConformance(const std::string_view conformance_type, const std::string_view interface_type, const int64_t id_override) const
{
    slang::ProgramLayout* layout = _p->_temp_program->getLayout();
    slang::TypeReflection* ct = layout->findTypeByName(conformance_type.data());
    slang::TypeReflection* it = layout->findTypeByName(interface_type.data());

    slang::ITypeConformance* t_c;
    const SlangResult result = _p->_session->createTypeConformanceComponentType(
        ct,
        it,
        &t_c,
        id_override,
        _p->_diagnostics_blob.writeRef());
    checkError(_p->_diagnostics_blob);
    if (SLANG_FAILED(result)) throw std::runtime_error("slang: type conformance error");
    _p->_type_conformances.push_back(t_c);
}

Slangc::Hash Slangc::compose() const
{
    _p->_program_components.insert(_p->_program_components.end(), _p->_type_conformances.begin(), _p->_type_conformances.end());

    const SlangResult result = _p->_session->createCompositeComponentType(
        _p->_program_components.data(),
        _p->_program_components.size(),
        _p->_composed_program.writeRef(),
        _p->_diagnostics_blob.writeRef());
    checkError(_p->_diagnostics_blob);
    if (SLANG_FAILED(result)) throw std::runtime_error("slang: composition error");

    Slang::ComPtr<slang::IBlob> hash;
    _p->_composed_program->getEntryPointHash(0, 0, hash.writeRef());
    return bytesToString(hash->getBufferPointer(), hash->getBufferSize());
}

std::vector<uint8_t> Slangc::compile() const
{
    Slang::ComPtr<slang::IBlob> spirv_code;
    const SlangResult result = _p->_composed_program->getEntryPointCode(
        0, 0, spirv_code.writeRef(), _p->_diagnostics_blob.writeRef()
    );
    checkError(_p->_diagnostics_blob);
    if(result != 0) return {};

    std::vector<uint8_t> out(spirv_code->getBufferSize(), 0);
    std::memcpy(out.data(), spirv_code->getBufferPointer(), out.size());
    return out;
}