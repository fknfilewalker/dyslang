#include <filesystem>
#include <iostream>
#include <slang-com-ptr.h>
#include <slang.h>
#include <dyslang/dyslang.h>\

class BinaryBlob : public ISlangBlob
{
public:
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE { return SLANG_E_NOT_IMPLEMENTED; }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }
    // ISlangBlob
    SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() SLANG_OVERRIDE { return _data.data(); }
    SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() SLANG_OVERRIDE { return _data.size(); }

    static inline Slang::ComPtr<ISlangBlob> create(const std::vector<uint8_t>& data)
    {
        return Slang::ComPtr<ISlangBlob>(new BinaryBlob(data));
    }

protected:
    BinaryBlob(const std::vector<uint8_t>& data) : _data(data) {}
    BinaryBlob() = default;

    std::vector<uint8_t> _data;
};

int main(int argc, char* argv[]) {
    std::filesystem::path path = std::filesystem::current_path();
    std::cout << "Working Dir: " << path << '\n';

    const char* variant = "float_rgb";

    // load plugin
    dyslang::Plugin plugin{ "plugins/point" };
    std::cout << plugin.to_string() << '\n';

	// set properties 
    dyslang::Properties props_in;
    props_in.properties["color"] = dyslang::f64v3{ 3.0f, 4.0, 1000 };
    props_in.properties["intensity"] = 15.0f;

    // create object
    auto light = plugin.create<void>(props_in, variant);
    std::cout << light->to_string() << '\n';

    // read back obj data
    dyslang::Properties props_out;
    light->traverse(props_out);
    std::cout << props_out.to_string() << '\n';

    std::vector<const char*> includes;
    std::vector<dyslang::Slangc::ArgPair> defines;

    // compile
    auto blob = BinaryBlob::create(plugin.slang_module);

    dyslang::Slangc slangc{ includes, defines };
    std::string_view moduleName = "load_plugins/interface_test";
    slangc.addModule(moduleName);
    slangc.addModule(light->implementation_name, light->implementation_name, blob);
    slangc.addEntryPoint(moduleName, "main");
    slangc.finalizeModulesAndEntryPoints();
    slangc.addTypeConformance(light->interface_name, light->implementation_name);
    dyslang::Slangc::Hash hash = slangc.compose();
    std::vector<uint8_t> output = slangc.compile();

    for (const auto& o : output) {
        std::cout << o;
    }
    std::cout << '\n';

	return 0;
}
