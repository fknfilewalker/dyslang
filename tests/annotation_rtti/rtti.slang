__generic<T>
struct StaticArray {
    T* data;
    uint64_t size;
};

[DllExport] int test(int x) {
    return 42 * x;
}

[__AttributeUsage(_AttributeTargets.Struct)]
struct RTTIAttribute{};

[anyValueSize(200)]
interface IEmitter {
   [Differentiable]
   int add(int z);
}

// type_param Real : __BuiltinFloatingPointType;

[RTTI]
struct Point<Real : __BuiltinFloatingPointType> : IEmitter {
    int x;
    int y;
    Real a;
    vector<Real, 3> b;
    int arr[3];
    StaticArray<Real> profile;

    [DllExport, Differentiable]
    int add(int z) {
        return x + y + z;
    }
};

