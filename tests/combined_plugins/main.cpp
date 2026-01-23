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

    dyslang::Plugins plugins({ "tests/combined_plugins/" }, {});
    plugins.add_interface("shape.slang", "IShape" VARIANT);
    plugins.add_interface("bsdf.slang", "IBsdf" VARIANT);
    plugins.interfaces["IShape" VARIANT].add_implementation("cube.slang", "Cube" VARIANT);
    plugins.interfaces["IBsdf" VARIANT].add_implementation("diffuse.slang", "Diffuse" VARIANT);
    plugins.prepare();

    plugins.add_module("main", { "main" });
    plugins.compose();
    auto output = plugins.spv();

    for (const auto& o : output) {
        std::cout << o;
    }
    std::cout << '\n';

	return 0;
}
