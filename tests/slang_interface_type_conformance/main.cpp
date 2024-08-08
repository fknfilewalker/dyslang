#include <iostream>
#include <vector>
#include <dyslang/slangc.h>

using namespace dyslang;

int main(int argc, char* argv[]) {
    std::vector<const char*> includes;
    std::vector<Slangc::ArgPair> defines;

    Slangc slangc(includes, defines);
    std::string_view moduleName = "slang_interface_type_conformance/generic-interface-conformance";
    slangc.addModule(moduleName);
    slangc.addEntryPoint(moduleName, "computeMain");
    slangc.finalizeModulesAndEntryPoints();
    slangc.addTypeConformance("TestInterfaceImpl<float>", "ITestInterface<float>");
    Slangc::Hash hash = slangc.compose();
    std::vector<uint8_t> output = slangc.compile();

    for (const auto& o : output) {
        std::cout << o;
    }
    std::cout << '\n';
    
	return 0;
}
