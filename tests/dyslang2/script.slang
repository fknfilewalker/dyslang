

interface IProperties {
    bool has(NativeString);
    T get<T>(NativeString);
    void set<T>(NativeString, Ptr<T>);
};

[anyValueSize(64)]
interface IEmitter<Real : __BuiltinFloatingPointType> {
    property uint bla;
    int emit();
};

struct Point<Real : __BuiltinFloatingPointType>  {
    int x;
    int y;
    float a;
    Ptr<float> ptr;
    vector<Real, 3> b;
    int arr[3];
    //StaticArray<float> profile;

    [require(cpp)]
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
    uint bla;
    int x;
    int y;
    float a;
    vector<Real, 3> b;
    vector<int, 3> ints;
    int arr[3];
    // StaticArray<float> profile;

    [require(cpp)]
    __init(IProperties p) {
        x = 2;
        y = 3;
        a = 4.0;
        b = vector<Real, 3>(Real(1.0), Real(4.0), Real(9.0));
        arr[0] = 1;
        arr[1] = 2;
        arr[2] = 3;

        x = p.get<int>("x");
        //y = p.get<int>("y");
        b = p.get<vector<Real, 3>>("b");
        bla =  __int_cast<uint>(p.get<vector<Real, 3>>("b").z);
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
        if (!p.has("bla")) {
            p.set("bla", &bla);
        }
    }

    [mutating]
    void copy(Point2<Real> other) {
        x = other.x;
    }

    int emit() {
        return 87;
    }
}

export __extern_cpp int emit() {
    return 34;
}
