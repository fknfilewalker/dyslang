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

    auto a = dyslang::detail::get_dims_v<double>; // 0 0 0
	auto aa = dyslang::detail::get_dims_v<std::array<double, 3>>; // 3 0 0
	auto aaa = dyslang::detail::get_dims_v<std::array<std::array<double, 3>, 4>>; // 4 3 0
	auto aaaa = dyslang::detail::get_dims_v<std::array<std::array<std::array<double, 3>, 4>, 5>>; // 5 4 3
	auto aaaaa = dyslang::detail::get_dims_v<std::array<dyslang::matrix<double, 4, 3>, 5>>; // 5 4 3
    auto aaaaaa = dyslang::detail::get_dims_v<dyslang::matrix<double, 3, 3>>; // 3 3 0

    auto b = dyslang::detail::get_stride_v<double>; // 0 0 0
    auto bb = dyslang::detail::get_stride_v<std::array<double, 3>>; // 8 0 0
    auto bbb = dyslang::detail::get_stride_v<std::array<std::array<double, 3>, 4>>; // 24 8 0
    auto bbbb = dyslang::detail::get_stride_v<std::array<std::array<std::array<double, 3>, 4>, 5>>;
    auto bbbbb = dyslang::detail::get_stride_v<std::array<dyslang::matrix<double, 4, 3>, 5>>;

    // for dynamic array the first dim is zero and needs to be replaced by runtime count
	auto c = dyslang::detail::get_dims_v<dyslang::DynamicArray<double>>;
    auto cc = dyslang::detail::get_dims_v<dyslang::DynamicArray<std::array<double, 3>>>;
    auto ccc = dyslang::detail::get_dims_v<dyslang::DynamicArray<dyslang::matrix<double, 3, 4>>>;
    // for dynamic array the first entry needs to be multiplied by runtime count
	auto d = dyslang::detail::get_stride_v<dyslang::DynamicArray<double>>;
    auto dd = dyslang::detail::get_stride_v<dyslang::DynamicArray<std::array<double, 3>>>;
	using DyT = dyslang::detail::get_type_t<dyslang::DynamicArray<std::array<double, 3>>>;

    // set properties
	auto id = 777;
	auto position = std::array<Real, 3>{ 10.0f, 17.0f, 13.0f };
	auto color = std::array<Real, 3>{ 3.0f, 4.0, 1000 };
	auto intensity = static_cast<Real>(15.0f);
    auto transform = dyslang::matrix<Real, 3, 3>{
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
	};
    auto dynamic_transform = dyslang::DynamicArray{ &transform, 1 };

    dyslang::Properties props_in;
    props_in.set("id", id);
    props_in.set("position", position);
	props_in.set("color", color);
	props_in.set("intensity", intensity);
	props_in.set("transform", transform);
	props_in.set("dynamic_transform", dynamic_transform);
    auto& g = props_in.get<std::array<Real, 3>>("color");
    g[0] = 11199;
    auto dynamic_transform_back = props_in.get<dyslang::DynamicArray<dyslang::matrix<Real, 3, 3>>>("dynamic_transform");
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
    auto output = slangc.spv();

    for (const auto& o : output) {
        std::cout << o;
    }
    std::cout << '\n';

	return 0;
}
