#include <iostream>
#include <vector>
#include <dyslang/dyslang.h>

using namespace dyslang;

int main(int argc, char* argv[]) {
    std::vector<const char*> includes;
    std::vector<Slangc::ArgPair> defines;

    Slangc slangc{ includes, defines };
    std::string_view moduleName = "slang_interface_type_conformance/interface_from_buffer";
    slangc.addModule(moduleName);
    slangc.addEntryPoint(moduleName, "main");
    slangc.finalizeModulesAndEntryPoints();
    slangc.addTypeConformance("IFoo", "Impl1");
    Slangc::Hash hash = slangc.compose();
    std::vector<uint8_t> output = slangc.compile();

    for (auto& o : output) {
        std::cout << o;
    }
    std::cout << '\n';

    return 0;
}
