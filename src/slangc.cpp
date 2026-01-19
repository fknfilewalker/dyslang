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
    void checkError(slang::IBlob* diagnostics_blob)
    {
        if (diagnostics_blob != nullptr)
        {
            printf("%s", static_cast<const char*>(diagnostics_blob->getBufferPointer()));
        }
        diagnostics_blob = nullptr;
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

struct dyslang::SlangComposerPrivate
{
    Slang::ComPtr<slang::ISession> session;
    std::vector<Slang::ComPtr<slang::IComponentType>> components;
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;

    void add_component(slang::IComponentType* ct) { components.push_back(Slang::ComPtr{ ct }); }

    std::shared_ptr<SlangComposerPrivate> compose()
    {
        Slang::ComPtr<slang::IComponentType> composition;
        const SlangResult result = session->createCompositeComponentType(
            reinterpret_cast<slang::IComponentType**>(components.data()),
            components.size(),
            composition.writeRef(),
            diagnosticsBlob.writeRef());
        checkError(diagnosticsBlob);
        if (SLANG_FAILED(result)) throw std::runtime_error("slang: composition error");

        return std::make_shared<SlangComposerPrivate>( session, std::vector{ composition }, Slang::ComPtr<slang::IBlob>());
    }
};

Slangc::Slangc(const std::vector<const char*>& includes, const std::vector<ArgPair>& defines) : _p{ new SlangComposerPrivate() }
{
    static Slang::ComPtr<slang::IGlobalSession> globalSession;
    if (!globalSession) {
        if (SLANG_FAILED(slang::createGlobalSession(globalSession.writeRef()))) throw std::runtime_error("slang: error creating global session");
    }

    std::vector<slang::CompilerOptionEntry> copts{
        {slang::CompilerOptionName::VulkanUseEntryPointName, {slang::CompilerOptionValueKind::Int, 1}},
        // {slang::CompilerOptionName::VulkanInvertY, {slang::CompilerOptionValueKind::Int, 1}},
        {slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1}},
        {slang::CompilerOptionName::IgnoreCapabilities, {slang::CompilerOptionValueKind::Int, 1}},
        {slang::CompilerOptionName::LanguageVersion,  {slang::CompilerOptionValueKind::Int, 2026}},
        {slang::CompilerOptionName::EnableExperimentalDynamicDispatch, {slang::CompilerOptionValueKind::Int, 1}},
        // {slang::CompilerOptionName::MinimumSlangOptimization, {slang::CompilerOptionValueKind::Int, 1} },
        // {slang::CompilerOptionName::DebugInformation, {slang::CompilerOptionValueKind::Int, SlangDebugInfoLevel::SLANG_DEBUG_INFO_LEVEL_MAXIMAL}},
        // {slang::CompilerOptionName::Capability, {slang::CompilerOptionValueKind::String, 0, 0, "spvBindlessTextureNV"} },
        // {slang::CompilerOptionName::DisableWarning, {slang::CompilerOptionValueKind::String, 0, 0, "41203"}}, // reinterpret<> into not equally sized types
        {slang::CompilerOptionName::Optimization, {slang::CompilerOptionValueKind::Int, 1}},
    };

    slang::TargetDesc targetDesc[2] = {};
    targetDesc[0].format = SLANG_GLSL;
    targetDesc[0].profile = globalSession->findProfile("glsl460");

    targetDesc[1].format = SLANG_SPIRV;
    targetDesc[1].profile = globalSession->findProfile("spirv_1_6");
    targetDesc[1].flags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc[0];
    sessionDesc.targetCount = 2;
    sessionDesc.compilerOptionEntries = copts.data();
    sessionDesc.compilerOptionEntryCount = static_cast<SlangInt>(copts.size());
    //sessionDesc.allowGLSLSyntax = true;
    sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_ROW_MAJOR;
    sessionDesc.searchPathCount = static_cast<SlangInt>(includes.size());
    sessionDesc.searchPaths = includes.data();
    sessionDesc.preprocessorMacroCount = static_cast<SlangInt>(defines.size());
    sessionDesc.preprocessorMacros = reinterpret_cast<const slang::PreprocessorMacroDesc*>(defines.data());

    if (SLANG_FAILED(globalSession->createSession(sessionDesc, _p->session.writeRef()))) throw std::runtime_error("slang: error creating session");
}

Slangc::Slangc(const Slangc& other)
{
    _p = std::make_shared<SlangComposerPrivate>(other._p->session, other._p->components);
}

Slangc& Slangc::add_module(const std::string_view module_name, const std::vector<std::string_view>& entry_points)
{
    slang::IModule* mod = _p->session->loadModule(module_name.data(), _p->diagnosticsBlob.writeRef());
    checkError(_p->diagnosticsBlob);
    if (!mod) throw std::runtime_error("slang: module null");
    _p->add_component(mod);

    for (const auto& entry_point : entry_points) {
        slang::IEntryPoint* entryPoint;
        const SlangResult result = mod->findEntryPointByName(entry_point.data(), &entryPoint);
        if (SLANG_FAILED(result)) throw std::runtime_error("slang: entrypoint");
        if (!entryPoint) throw std::runtime_error("slang: entrypoint null");
        _p->add_component(entryPoint);
    }

    return *this;
}

Slangc& Slangc::add_module(std::string_view module_name, std::string_view module_path, const void* blob) {
    slang::IModule* mod = _p->session->loadModuleFromIRBlob(module_name.data(), module_path.data(), (slang::IBlob*)blob, _p->diagnosticsBlob.writeRef());
    checkError(_p->diagnosticsBlob);
    if (!mod) throw std::runtime_error("slang: module null");
    _p->add_component(mod);
    return *this;
}

Slangc& Slangc::add_type_conformance(std::string_view interface_type, std::string_view conformance_type,
	int64_t id_override)
{
    slang::ProgramLayout* layout = _p->components.front()->getLayout();
    slang::TypeReflection* t = layout->findTypeByName(conformance_type.data());
    slang::TypeReflection* it = layout->findTypeByName(interface_type.data());

    slang::ITypeConformance* tc;
    const SlangResult result = _p->session->createTypeConformanceComponentType(
        t, it, &tc, id_override, _p->diagnosticsBlob.writeRef());
    checkError(_p->diagnosticsBlob);
    if (SLANG_FAILED(result)) throw std::runtime_error("slang: type conformance error");
    _p->add_component(tc);
    return *this;
}

Slangc Slangc::compose() const
{
    return Slangc { _p->compose() };
}

Slangc& Slangc::hash(uint32_t entry, Hash& hash) {
    Slang::ComPtr<slang::IBlob> hashBlob;
    _p->components.front()->getEntryPointHash(entry, 0, hashBlob.writeRef());
    hash = bytesToString(hashBlob->getBufferPointer(), hashBlob->getBufferSize());
    return *this;
}

std::vector<uint32_t> Slangc::spv() const
{
    Slang::ComPtr<slang::IBlob> blob;
    const SlangResult result = _p->components.front()->getTargetCode(1, blob.writeRef(), _p->diagnosticsBlob.writeRef());
    checkError(_p->diagnosticsBlob);
    if (result != 0) throw std::runtime_error("slang: target code error");
    std::vector<uint32_t> out(blob->getBufferSize() / sizeof(uint32_t), 0);
    std::memcpy(out.data(), blob->getBufferPointer(), blob->getBufferSize());
    return out;
}

std::vector<uint8_t> Slangc::glsl() const
{
    Slang::ComPtr<slang::IBlob> blob;
    const SlangResult result = _p->components.front()->getTargetCode(0, blob.writeRef(), _p->diagnosticsBlob.writeRef());
    checkError(_p->diagnosticsBlob);
    if (result != 0) throw std::runtime_error("slang: target code error");
    std::vector<uint8_t> out(blob->getBufferSize(), 0);
    std::memcpy(out.data(), blob->getBufferPointer(), blob->getBufferSize());
    return out;
}

std::vector<uint32_t> Slangc::entry(uint32_t entry) const
{
    Slang::ComPtr<slang::IBlob> code;
    const SlangResult result = _p->components.front()->getEntryPointCode(
        entry, 0, code.writeRef(), _p->diagnosticsBlob.writeRef()
    );
    checkError(_p->diagnosticsBlob);
    if (result != 0) throw std::runtime_error("slang: entrypoint code error");
    std::vector<uint32_t> out(code->getBufferSize() / sizeof(uint32_t), 0);
    std::memcpy(out.data(), code->getBufferPointer(), code->getBufferSize());
    return out;
}

std::array<uint32_t, 6> Slangc::get_rtti_bytes(std::string_view interface_type, std::string_view conformance_type) const
{
    std::array<uint32_t, 6> rtti = {};
    auto layout = _p->components.front()->getLayout(0);
    slang::TypeReflection* t = layout->findTypeByName(conformance_type.data());
    slang::TypeReflection* it = layout->findTypeByName(interface_type.data());
    auto result = _p->session->getDynamicObjectRTTIBytes(t, it, rtti.data(), sizeof(uint32_t) * rtti.size());
    if (SLANG_FAILED(result)) throw std::runtime_error("slang: rtti bytes error");
    return rtti;
}

Slangc& Slangc::get_global_resource_array_binding(uint32_t& binding, uint32_t& space)
{
    auto layout = _p->components.front()->getLayout(0, _p->diagnosticsBlob.writeRef());
    checkError(_p->diagnosticsBlob);
    if (!layout) throw std::runtime_error("slang: layout null");
    const unsigned parameterCount = layout->getParameterCount();
    for (unsigned pp = 0; pp < parameterCount; pp++)
    {
        slang::VariableLayoutReflection* parameter = layout->getParameterByIndex(pp);
        const std::string name = parameter->getName();
        if (name == "__global_resource_array")
        {
            binding = parameter->getBindingIndex();
            space = parameter->getBindingSpace();
            return *this;
        }
    }
    throw std::runtime_error("slang: __global_resource_array not found");
}
