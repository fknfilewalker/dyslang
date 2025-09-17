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

    DynamicClass c("tests/dyslang2/script.slang", "Point<float>");
    return 0;
}