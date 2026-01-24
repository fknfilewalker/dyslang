module diffuse;
import bsdf;
import shape;
import dyslang;

public struct Diffuse<Real : __BuiltinFloatingPointType> : IBsdf<Real> {
    IShape<Real> *shape;
    vector<Real, 3> color;
    dyslang::DynamicArray<float> data;

    [require(cpp)] public __init(dyslang::IProperties p) {
        if (p.has("color")) {
            color = p.get<vector<Real, 3>>("color");
        }
    }
    [require(cpp), mutating] public void traverse(dyslang::IProperties p) {
        p.set("color", &color);
    }
};

