#include <filesystem>
#include <dyslang/dyslang2.h>
using namespace dyslang2;

void checkError(slang::IBlob *diagnostics_blob)
{
    if (diagnostics_blob != nullptr)
    {
        std::printf("%s", static_cast<const char *>(diagnostics_blob->getBufferPointer()));
    }
}

int main(int argc, char *argv[])
{
    std::filesystem::path path = std::filesystem::current_path();
    std::printf("Working Dir: %s\n", path.string().c_str());

    int x = 47;
    std::vector<float> b = { 1.0f, 2.0f, 3.0f };
    auto p_in = Properties().set("x", &x).set("b", &b).print();

    DynamicClass c("tests/dyslang2/script.slang", "Point<float>", "IEmitter<float>");
    DynamicObject o = c.init(p_in);
    Properties p_out = o.traverse();
    p_out.print();

    auto vi = p_out.get<int>("x");
    auto v = p_out.get<std::vector<float>>("b");

    int res = o.loadFunction<int>("emit")();
    return 0;
}