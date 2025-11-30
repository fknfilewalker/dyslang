#include <vector>
#include <string>
#include <slang-com-ptr.h>
#include <slang-com-helper.h>
using namespace Slang;

void checkError(slang::IBlob* diagnostics_blob)
{
    if (diagnostics_blob != nullptr)
    {
        printf("%s", static_cast<const char*>(diagnostics_blob->getBufferPointer()));
    }
}

const char* decl_kind_to_string(const slang::DeclReflection::Kind kind)
{
	switch (kind)
	{
	case slang::DeclReflection::Kind::Struct: return "Struct";
	case slang::DeclReflection::Kind::Func: return "Func";
	case slang::DeclReflection::Kind::Module: return "Module";
	case slang::DeclReflection::Kind::Generic: return "Generic";
	case slang::DeclReflection::Kind::Variable: return "Variable";
	case slang::DeclReflection::Kind::Namespace: return "Namespace";
	case slang::DeclReflection::Kind::Unsupported: return "Unsupported";
	}
}

const char* type_kind_to_string(const slang::TypeReflection::Kind kind)
{
    switch (kind) {
    case slang::TypeReflection::Kind::None: return "None";
    case slang::TypeReflection::Kind::Struct: return "Struct";
    case slang::TypeReflection::Kind::Array: return "Array";
    case slang::TypeReflection::Kind::Matrix: return "Matrix";
    case slang::TypeReflection::Kind::Vector: return "Vector";
    case slang::TypeReflection::Kind::Scalar: return "Scalar";
    case slang::TypeReflection::Kind::ConstantBuffer: return "ConstantBuffer";
    case slang::TypeReflection::Kind::Resource: return "Resource";
    case slang::TypeReflection::Kind::SamplerState: return "SamplerState";
    case slang::TypeReflection::Kind::TextureBuffer: return "TextureBuffer";
    case slang::TypeReflection::Kind::ShaderStorageBuffer: return "ShaderStorageBuffer";
    case slang::TypeReflection::Kind::ParameterBlock: return "ParameterBlock";
    case slang::TypeReflection::Kind::GenericTypeParameter: return "GenericTypeParameter";
    case slang::TypeReflection::Kind::Interface: return "Interface";
    case slang::TypeReflection::Kind::OutputStream: return "OutputStream";
    case slang::TypeReflection::Kind::Specialized: return "Specialized";
    case slang::TypeReflection::Kind::Feedback: return "Feedback";
    case slang::TypeReflection::Kind::Pointer: return "Pointer";
    case slang::TypeReflection::Kind::DynamicResource: return "DynamicResource";
    }
    return "Unknown";
};

const char* scalar_type_to_string(const slang::TypeReflection::ScalarType kind)
{
	switch (kind) {
	case slang::TypeReflection::ScalarType::None: return "None";
	case slang::TypeReflection::ScalarType::Void: return "Void";
	case slang::TypeReflection::ScalarType::Bool: return "Bool";
	case slang::TypeReflection::ScalarType::Int32: return "Int32";
	case slang::TypeReflection::ScalarType::UInt32: return "UInt32";
	case slang::TypeReflection::ScalarType::Int64: return "Int64";
	case slang::TypeReflection::ScalarType::UInt64: return "UInt64";
	case slang::TypeReflection::ScalarType::Float16: return "Float16";
	case slang::TypeReflection::ScalarType::Float32: return "Float32";
	case slang::TypeReflection::ScalarType::Float64: return "Float64";
	case slang::TypeReflection::ScalarType::Int8: return "Int8";
	case slang::TypeReflection::ScalarType::UInt8: return "UInt8";
	case slang::TypeReflection::ScalarType::Int16: return "Int16";
	case slang::TypeReflection::ScalarType::UInt16: return "UInt16";
	}
}

//std::string type_to_string(slang::TypeReflection* type)
//{
//    const char* type_kind = type_kind_to_string(type->getKind());
//    const char* scalar_type = scalar_type_to_string(type->getScalarType());
//
//    std::string out = std::string() + type->getName() + " " + type_kind + " " + scalar_type;
//	for (uint32_t i = 0; i < type->getFieldCount(); i++)
//	{
//        const char* name = scalar_type_to_string(type->getFieldByIndex(i)->getType()->getScalarType());
//		out += " " + std::string(name);
//	}
//    
//    return out;
//}

void print_type(slang::TypeLayoutReflection* type, const size_t depth);

void print_variable(slang::VariableLayoutReflection* var, const size_t depth = 0)
{
	std::string indent(depth * 2, ' ');
	const char* name = var->getName();
    printf("%s- %s offset: %zu\n", indent.c_str(), name, var->getOffset());
    print_type(var->getTypeLayout(), depth + 2);
}

void print_type(slang::TypeLayoutReflection* type, const size_t depth = 0)
{
	std::string indent(depth * 2, ' ');
	const char* name = type->getName();
	const char* kind = type_kind_to_string(type->getKind());

    auto element_type_name = type->getElementTypeLayout()->getName();
    auto element_type = scalar_type_to_string(type->getElementTypeLayout()->getScalarType());
	const char* scalar_type = scalar_type_to_string(type->getScalarType());
	printf("%s- %s %s (scalar: %s element: %s elements: %zu size: %zu alignment: %d)\n", indent.c_str(), 
        kind, name, scalar_type, element_type, type->getElementCount(), type->getSize(), type->getAlignment()
    );
	const char* scalar_type = scalar_type_to_string(type->getScalarType());
	printf("%s- %s %s (%s elements: %zu size: %zu alignment: %d)\n", indent.c_str(), kind, name, scalar_type, type->getElementCount(), type->getSize(), type->getAlignment());
	for (uint32_t i = 0; i < type->getFieldCount(); i++)
	{
        print_variable(type->getFieldByIndex(i), depth + 2);
	}
	if (type->getKind() == slang::TypeReflection::Kind::Array)
	{
		print_type(type->getElementTypeLayout(), depth + 2);
	}
	/*if (type->getKind() == slang::TypeReflection::Kind::Pointer)
	{
        auto aa = type->getGenericContainer();
        print_type(aa->getConcreteType(type->get), depth + 2);
       
        int x = 2;
	}*/
}

struct ReflectRTTI
{
	ReflectRTTI(slang::ProgramLayout* layout, slang::TypeReflection* type, const std::vector<slang::TypeReflection*>& gparams = {}) : _layout(layout), _type(type), _gparams(gparams)
	{
        auto gen = type->getGenericContainer();
        if (gen) {
            std::vector<slang::GenericArgType> argTypes;
            std::vector<slang::GenericArgReflection> args;
            for (auto& g : gparams)
            {
                argTypes.push_back(slang::GenericArgType::SLANG_GENERIC_ARG_TYPE);
                args.push_back({ g });
            }
            ComPtr<slang::IBlob> diagnosticBlob;
            auto* s_gen = layout->specializeGeneric(gen, 1, argTypes.data(), args.data(), diagnosticBlob.writeRef());
            checkError(diagnosticBlob);

            auto stype = type->applySpecializations(s_gen);
            
            print_type(layout->getTypeLayout(stype));
        }
        else print_type(layout->getTypeLayout(type));
	}

    slang::ProgramLayout* _layout;
    slang::TypeReflection* _type;
	std::vector<slang::TypeReflection*> _gparams;
};

void reflect_on(const std::string& m)
{
    ComPtr<slang::IGlobalSession> slangSession;
    if (SLANG_FAILED(slang::createGlobalSession(slangSession.writeRef()))) return;

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SHADER_HOST_CALLABLE;
    targetDesc.flags = SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    ComPtr<slang::ISession> session;
    if (SLANG_FAILED(slangSession->createSession(sessionDesc, session.writeRef()))) return;

    ComPtr<slang::IBlob> diagnosticBlob;
    slang::IModule* slangModule = session->loadModule(m.c_str(), diagnosticBlob.writeRef());
    checkError(diagnosticBlob);
    if (!slangModule) return;

    printf("Reflecting on: %s\n", m.c_str());
    auto layout = slangModule->getLayout();
	ReflectRTTI reflectRTTI(layout, layout->findTypeByName("Point"), { layout->findTypeByName("float") });
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
		//printf("%s", json);
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

    ComPtr<slang::IBlob> diagnosticsBlob;
    slang::ProgramLayout* layout = (slang::ProgramLayout*)request->getReflection();
    
    ComPtr<ISlangSharedLibrary> sharedLibrary;
    SLANG_RETURN_ON_FAIL(request->getTargetHostCallable(0, sharedLibrary.writeRef()))
    SlangFuncPtr ptr = sharedLibrary->findFuncByName("test");
    SlangFuncPtr add_ptr = sharedLibrary->findFuncByName("add");
    SlangFuncPtr add_ptr2 = sharedLibrary->findFuncByName("Test<float>::add"); // does not work

	typedef int (*FuncType)(int);
	FuncType func = (FuncType)ptr;
	int result = func(42);

    /*std::pair<int, int> data;
	data.first = 6;
	data.second = 2;
	typedef int (*FuncType2)(std::pair<int, int>, int);
	FuncType2 func2 = (FuncType2)add_ptr;
	int result2 = func2(data, 3);*/

    return 0;
}

int main(int argc, char* argv[]) {
	reflect_on("tests/annotation_rtti/rtti");
    return slang_test();
}