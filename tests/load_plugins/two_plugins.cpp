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

    // load plugin
    dyslang::CompPlugin point_plugin{ (path / "plugins/point").string() };
    dyslang::CompPlugin spot_plugin{ (path / "plugins/spot").string() };
    std::cout << point_plugin.to_string() << '\n';
    std::cout << spot_plugin.to_string() << '\n';

	// set properties 
    dyslang::Properties props_in;
    auto id = 777;
    auto position = std::array<Real, 3>{ 10.0f, 17.0f, 13.0f };
    auto color = std::array<Real, 3>{ 3.0f, 4.0, 1000 };
    auto intensity = static_cast<Real>(15.0f);
    auto transform = dyslang::matrix<Real, 3, 3>{
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    };

    props_in.set("id", id);
    props_in.set("position", position);
    props_in.set("color", color);
    props_in.set("intensity", intensity);
    props_in.set("transform", transform);
    //props_in.set("texture", dyslang::ResourceRef{ 1 });
    std::cout << "IN:" << props_in.to_string() << '\n';

    // create object
    std::unique_ptr<dyslang::CompObject<void>> point_light = point_plugin.create<void>(props_in, variant);
    std::unique_ptr<dyslang::CompObject<void>> spot_light = spot_plugin.create<void>(props_in, variant);
    std::cout << point_light->to_string() << '\n';

    // read back obj data
    dyslang::Properties props_out;
    point_light->traverse(props_out);
    std::cout << "OUT:" << props_out.to_string() << '\n';

    std::vector<const char*> includes;
    std::vector<dyslang::Slangc::ArgPair> defines;
    // compile
    dyslang::Slangc slangc{ includes, defines };
    slangc.add_module("tests/load_plugins/interface_test", { "main" });
    slangc.add_module(point_plugin.implementation_name, point_plugin.implementation_name, point_plugin.slang_module_blob());
    slangc.add_module(spot_plugin.implementation_name, spot_plugin.implementation_name, spot_plugin.slang_module_blob());
    slangc = slangc.compose();
	uint32_t binding = 0, space = 0;
    slangc.get_global_resource_array_binding(binding, space);
	std::cout << "Global Resource Array: Binding: " << binding << ", Set: " << space << "\n\n";
    slangc.add_type_conformance(point_light->interface_name, point_light->implementation_name);
    slangc.add_type_conformance(spot_light->interface_name, spot_light->implementation_name);
    dyslang::Slangc::Hash hash;
    slangc = slangc.compose().hash(0, hash);
    auto output = slangc.spv();

    for (const auto& o : output) {
        std::cout << o;
    }
    std::cout << '\n';

	return 0;
}
