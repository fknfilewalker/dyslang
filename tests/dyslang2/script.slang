

interface IProperties {
    bool has(NativeString);
    T get<T>(NativeString);
    void set<T>(NativeString, Ptr<T>);
};

[anyValueSize(64)]
interface IEmitter<Real : __BuiltinFloatingPointType> {
    int emit();
};

struct Point<Real : __BuiltinFloatingPointType>  {
    int x;
    int y;
    float a;
    vector<Real, 3> b;
    int arr[3];
    //StaticArray<float> profile;

    __init(IProperties p) {
        x = 2;
        y = 3;
        a = 4.0;
        b = vector<Real, 3>(Real(1.0), Real(4.0), Real(9.0));
        arr[0] = 1;
        arr[1] = 2;
        arr[2] = 3;

        x = p.get<int>("x");
        b = p.get<vector<Real, 3>>("b");
    }

    [mutating, require(cpp)]
    void traverse(IProperties p) {
        if (!p.has("x")) {
            p.set("x", &x);
        }
        if (!p.has("a")) {
            p.set("a", &a);
        }
        if (!p.has("b")) {
            p.set("b", &b);
        }
    }

    int emit() {
        return 2;
    }
};

struct Point2<Real : __BuiltinFloatingPointType> : IEmitter<Real> {
    int x;
    int y;
    int emit() {
        return 87;
    }
}

//[DllExport] void __traverse(Ptr<void> ptr, Ref<IProperties> props) { 
//    Point<float>().traverse(props); 
//}
