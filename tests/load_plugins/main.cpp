#include <filesystem>
#include <iostream>
#include <slang.h>
#include <dyslang/dyslang.h>\

int main(int argc, char* argv[]) {
    std::filesystem::path path = std::filesystem::current_path();
    std::cout << "Working Dir: " << path << '\n';

    const char* variant = "float_rgb";

    // load plugin
    dyslang::Plugin plugin{ "plugins/point" };
    std::cout << plugin.to_string() << '\n';

	// set properties 
    dyslang::Properties props_in;
    props_in.properties["position"] = dyslang::f64v3{ 10.0f, 10.0f, 10.0f };
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
    dyslang::Slangc slangc{ includes, defines };
    std::string_view moduleName = "load_plugins/interface_test";
    slangc.addModule(moduleName);
    slangc.addModule(light->implementation_name, light->implementation_name, plugin.slang_module_blob());
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
