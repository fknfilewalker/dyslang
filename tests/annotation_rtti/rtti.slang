[DllExport] int test(int x) {
    return 42 * x;
}

[__AttributeUsage(_AttributeTargets.Struct)]
struct RTTIAttribute{};

interface IEmitter {
    [Differentiable]
    int add(int z);
    // int sub();
}

[RTTI]
struct Point/*<Real : __BuiltinFloatingPointType>*/ : IEmitter {
    //Real a;
    int x;
    int y;

    [DllExport, Differentiable]
    int add(int z) {
        return x + y + z;
    }
};

