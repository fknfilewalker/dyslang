#include <iostream>
#include <vector>
#include <dyslang/dyslang.h>

using namespace dyslang;

int main(int argc, char* argv[]) {
    std::vector<const char*> includes;
    std::vector<Slangc::ArgPair> defines;

    Slangc slangc{ includes, defines };
    std::string_view moduleName = "tests/slang_interface_type_conformance/interface_from_buffer";
    slangc.add_module(moduleName, { "main" });
    slangc = slangc.compose();
    slangc.add_type_conformance("IFoo", "Impl1");
    slangc.add_type_conformance("IFoo", "Impl2");
    dyslang::Slangc::Hash hash;
    slangc = slangc.compose().hash(0, hash);
    std::vector<uint8_t> output = slangc.glsl();

    for (auto& o : output) {
        std::cout << o;
    }
    std::cout << '\n';

    return 0;
}
