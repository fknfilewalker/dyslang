#include <vector>
#include <slang-com-ptr.h>
#include <slang-com-helper.h>
using namespace Slang;

void checkError(slang::IBlob* diagnostics_blob)
{
    if (diagnostics_blob != nullptr)
    {
        printf("%s", static_cast<const char*>(diagnostics_blob->getBufferPointer()));
    }
    diagnostics_blob = nullptr;
}

int slang_test()
{
	{
		ComPtr<slang::IGlobalSession> slangSession;
	    SLANG_RETURN_ON_FAIL(slang::createGlobalSession(slangSession.writeRef()));

		slang::SessionDesc sessionDesc = {};
		slang::TargetDesc targetDesc = {};
		targetDesc.format = SLANG_SHADER_HOST_CALLABLE;
		targetDesc.flags = SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM;

		sessionDesc.targets = &targetDesc;
		sessionDesc.targetCount = 1;

		ComPtr<slang::ISession> session;
		SLANG_RETURN_ON_FAIL(slangSession->createSession(sessionDesc, session.writeRef()));

        ComPtr<slang::IBlob> diagnosticBlob;
		slang::IModule* slangModule = session->loadModule("tests/annotation_rtti/rtti", diagnosticBlob.writeRef());
        checkError(diagnosticBlob);
        if (!slangModule) return -1;

        slang::DeclReflection* declReflection = slangModule->getModuleReflection();
		uint32_t children = declReflection->getChildrenCount();
        for (uint32_t i = 0; i < children; i++)
        {
            slang::DeclReflection* child = declReflection->getChild(i);
            //child->asGeneric()->applySpecializations()
			const char* name = child->getName();
            slang::DeclReflection::Kind kind = child->getKind();
            int a = 2;
        }

		std::vector<slang::IComponentType*> componentTypes { slangModule };

		ComPtr<slang::IComponentType> composedProgram;
	    {
	        ComPtr<slang::IBlob> diagnosticsBlob;
	        SlangResult result = session->createCompositeComponentType(
	            componentTypes.data(),
	            componentTypes.size(),
	            composedProgram.writeRef(),
	            diagnosticsBlob.writeRef());
            checkError(diagnosticsBlob);
	        SLANG_RETURN_ON_FAIL(result)
	    }

        SlangInt spezCount = composedProgram->getSpecializationParamCount();
		slang::ProgramLayout* layout = composedProgram->getLayout();
        ComPtr<ISlangBlob> blob;
        layout->toJson(blob.writeRef());
		const char* json = (const char*)blob->getBufferPointer();
		printf("%s", json);
        const unsigned parameterCount = layout->getParameterCount();
		const unsigned typeParameterCount = layout->getTypeParameterCount();
        SlangInt spec = composedProgram->getSpecializationParamCount();
        auto* float_type = layout->findTypeByName("float");
        auto *type = layout->findTypeByName("Point");
        auto s = type->getFieldByIndex(0)->getType()->getName();
        auto *type_cont = type->getGenericContainer();
        slang::GenericArgType argType = slang::GenericArgType::SLANG_GENERIC_ARG_TYPE;
		slang::GenericArgReflection arg = { float_type };
        ComPtr<slang::IBlob> diagnosticsBlob;
        auto* g = layout->specializeGeneric(type_cont, 1, &argType, &arg, diagnosticsBlob.writeRef());
        checkError(diagnosticsBlob);
        auto spec_type = type->applySpecializations(g);
        auto s2 = spec_type->getFieldByIndex(0)->getType()->getName();
        const unsigned fieldCount = type->getFieldCount();
        auto *attrib = type->findUserAttributeByName("RTTI");
        auto name = attrib->getName();

        int x = 2;
    }

    ComPtr<slang::IGlobalSession> slangSession;
    slangSession.attach(spCreateSession(NULL));

    Slang::ComPtr<slang::ICompileRequest> request;
    SLANG_RETURN_ON_FAIL(slangSession->createCompileRequest(request.writeRef()));

    const int targetIndex = request->addCodeGenTarget(SLANG_SHADER_HOST_CALLABLE);
    request->setTargetFlags(targetIndex, SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM);

    const int translationUnitIndex = request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, nullptr);

    // Set the source file for the translation unit
    //Slang::String path = resourceBase.resolveResource("shader.slang");
    request->addTranslationUnitSourceFile(translationUnitIndex, "tests/annotation_rtti/rtti.slang");

    const SlangResult compileRes = request->compile();
    if (auto diagnostics = request->getDiagnosticOutput()) printf("%s", diagnostics);

    slang::ProgramLayout* reflection = (slang::ProgramLayout*)request->getReflection();
    auto* float_type = reflection->findTypeByName("float");
    auto* type = reflection->findTypeByName("Point");

    /*auto* type_cont = type->getGenericContainer();
    slang::GenericArgType argType = slang::GenericArgType::SLANG_GENERIC_ARG_TYPE;
    slang::GenericArgReflection arg = { float_type };
    ComPtr<slang::IBlob> diagnosticsBlob;
    auto* g = reflection->specializeGeneric(type_cont, 1, &argType, &arg, diagnosticsBlob.writeRef());
    checkError(diagnosticsBlob);
    type->applySpecializations(g);*/
    //ComPtr<slang::IBlob> diagnosticsBlob;
    //reflection->specializeType(type, 1, &float_type, diagnosticsBlob.writeRef());
    
    ComPtr<ISlangSharedLibrary> sharedLibrary;
    SLANG_RETURN_ON_FAIL(request->getTargetHostCallable(0, sharedLibrary.writeRef()))
    void* ptr = sharedLibrary->findFuncByName("test");
    void* add_ptr = sharedLibrary->findFuncByName("add");
    void* add_ptr2 = sharedLibrary->findFuncByName("Test<float>::add"); // does not work

	typedef int (*FuncType)(int);
	FuncType func = (FuncType)ptr;
	int result = func(42);

    std::pair<int, int> data;
	data.first = 6;
	data.second = 2;
	typedef int (*FuncType2)(std::pair<int, int>, int);
	FuncType2 func2 = (FuncType2)add_ptr;
	int result2 = func2(data, 3);

    return 0;
}

int main(int argc, char* argv[]) {
    return slang_test();
}