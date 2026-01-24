#include <filesystem>
#include <iostream>
#include <dyslang/dyslang.h>

#define VARIANT "<float>"
using Real = float;

int main(int argc, char* argv[]) {
    std::filesystem::path cwd = std::filesystem::current_path();
    std::cout << "Working Dir: " << cwd << '\n';
    std::filesystem::path path = dyslang::platform::executable_filepath().parent_path();
    std::cout << "Executable Dir: " << path << '\n';

    dyslang::Plugins plugins({ "", "tests/combined_plugins/" }, {});
    plugins.add_interface("shape.slang", "IShape" VARIANT);
    plugins.add_interface("bsdf.slang", "IBsdf" VARIANT);
    plugins.interfaces["IShape" VARIANT].add_implementation("cube.slang", "Cube" VARIANT);
    plugins.interfaces["IBsdf" VARIANT].add_implementation("diffuse.slang", "Diffuse" VARIANT);
    plugins.prepare();

    {
        std::vector<uint32_t> data;
        data.resize(plugins.f_size_of("IBsdf" VARIANT) / 4);
        dyslang::Properties props_in;
        std::array<float, 3> input_color = { 0.8f, 0.1f, 0.3f };
        props_in.set("color", input_color);
        plugins.f_create(&props_in, "Diffuse" VARIANT, data.data());
        std::cout << props_in.to_string() << "\n";

        auto diffuse = plugins.create("Diffuse" VARIANT, "IBsdf" VARIANT, props_in);

        dyslang::Properties props_out;
        plugins.f_traverse(&props_out, "Diffuse" VARIANT, data.data());
        std::cout << props_out.to_string() << "\n";

        size_t bsdf_size = plugins.f_size_of("IBsdf" VARIANT);
        size_t diffuse_size = plugins.f_size_of("Diffuse" VARIANT);
        size_t shape_size = plugins.f_size_of("IShape" VARIANT);
        size_t cube_size = plugins.f_size_of("Cube" VARIANT);
    }

    plugins.add_module("main", { "main" });
    plugins.compose();
    auto output = plugins.spv();

    for (const auto& o : output) {
        std::cout << o;
    }
    std::cout << '\n';

	return 0;
}
