#include <filesystem>
#include <iostream>
#include <slang.h>
#include <dyslang/dyslang.h>

int main(int argc, char* argv[]) {
    std::filesystem::path path = std::filesystem::current_path();
    std::cout << "Working Dir: " << path << '\n';

    const char* variant = "float_rgb";
    using Real = float;

    // load plugin
    dyslang::Plugin point_plugin{ "plugins/point" };
    dyslang::Plugin spot_plugin{ "plugins/spot" };
    std::cout << point_plugin.to_string() << '\n';
    std::cout << spot_plugin.to_string() << '\n';

	// set properties 
    dyslang::Properties props_in;
	props_in.set("id", 777);
    props_in.set("position", dyslang::v3<Real>{ 10.0f, 10.0f, 13.0f });
    props_in.set("color", dyslang::v3<Real>{ 3.0f, 4.0, 1000 });
    props_in.set("intensity", Real(15.0f));
    props_in.set("transform", dyslang::m3x3<Real>{
        1.0f, 0.0, 0.0,
    	0.0f, 1.0, 0.0,
    	0.0f, 0.0, 1.0
    });
    props_in.set("texture", dyslang::ResourceRef{ 1 });
    std::cout << "IN:" << props_in.to_string() << '\n';

    // create object
    std::unique_ptr<dyslang::Object<void>> point_light = point_plugin.create<void>(props_in, variant);
    std::unique_ptr<dyslang::Object<void>> spot_light = spot_plugin.create<void>(props_in, variant);
    std::cout << point_light->to_string() << '\n';

    // read back obj data
    dyslang::Properties props_out;
    point_light->traverse(props_out);
    std::cout << "OUT:" << props_out.to_string() << '\n';

    std::vector<const char*> includes;
    std::vector<dyslang::Slangc::ArgPair> defines;
    // compile
    dyslang::Slangc slangc{ includes, defines };
    std::string_view moduleName = "tests/load_plugins/interface_test";
    slangc.addModule(moduleName);
    slangc.addModule(point_plugin.implementation_name, point_plugin.implementation_name, point_plugin.slang_module_blob());
    slangc.addModule(spot_plugin.implementation_name, spot_plugin.implementation_name, spot_plugin.slang_module_blob());
    slangc.addEntryPoint(moduleName, "main");
    slangc.finalizeModulesAndEntryPoints();
	auto bindings = slangc.globalResourceArrayBinding();
	std::cout << "Global Resource Array: Binding: " << bindings.first << ", Set: " << bindings.second << "\n\n";
    slangc.addTypeConformance(point_light->interface_name, point_light->implementation_name);
    slangc.addTypeConformance(spot_light->interface_name, spot_light->implementation_name);
    dyslang::Slangc::Hash hash = slangc.compose();
    std::vector<uint8_t> output = slangc.compile();

    for (const auto& o : output) {
        std::cout << o;
    }
    std::cout << '\n';

	return 0;
}
