#include <filesystem>
#include <iostream>
#include <slang.h>
#include <dyslang/dyslang.h>

int main(int argc, char* argv[]) {
    std::filesystem::path cwd = std::filesystem::current_path();
    std::cout << "Working Dir: " << cwd << '\n';
    std::filesystem::path path = dyslang::platform::executable_filepath().parent_path();
    std::cout << "Executable Dir: " << path << '\n';

    const char* variant = "float_rgb";
    using Real = float;
    const uint32_t conformance_id = 420;

    // load plugin
    dyslang::Plugin plugin{ (path / "plugins/point").string() };
    std::cout << plugin.to_string() << '\n';

	// set properties 
    dyslang::Properties props_in;
	props_in.set("id", 777);
    props_in.set("position", std::array<Real, 3>{ 10.0f, 10.0f, 13.0f });
    props_in.set("color", std::array<Real, 3>{ 3.0f, 4.0, 1000 });
    props_in.set("intensity", Real(15.0f));
    props_in.set("transform", dyslang::matrix<Real, 3, 3>{
        1.0f, 0.0, 0.0,
    	0.0f, 1.0, 0.0,
    	0.0f, 0.0, 1.0
    });
    //props_in.set("texture", dyslang::ResourceRef{ 1 });
    std::cout << "IN:" << props_in.to_string() << '\n';

    // create object
    std::unique_ptr<dyslang::Object<void>> light = plugin.create<void>(props_in, variant);
    light->data.set_type_conformance_id(conformance_id);
    std::cout << light->to_string() << '\n';

    // read back obj data
    dyslang::Properties props_out;
    light->traverse(props_out);
    std::cout << "OUT:" << props_out.to_string() << '\n';

    // compile
    std::vector<const char*> includes;
    std::vector<dyslang::Slangc::ArgPair> defines;
    dyslang::Slangc slangc{ includes, defines };
    slangc.add_module("tests/load_plugins/interface_test", { "main" });
    slangc.add_module(plugin.implementation_name, plugin.implementation_name, plugin.slang_module_blob());
    slangc = slangc.compose();
	uint32_t binding = 0, space = 0;
    slangc.get_global_resource_array_binding(binding, space);
	std::cout << "Global Resource Array: Binding: " << binding << ", Set: " << space << "\n\n";
    slangc.add_type_conformance(light->interface_name, light->implementation_name, conformance_id);
    dyslang::Slangc::Hash hash;
    slangc = slangc.compose().hash(0, hash);
    std::vector<uint8_t> output = slangc.glsl();

    for (const auto& o : output) {
        std::cout << o;
    }
    std::cout << '\n';

	return 0;
}
