module diffuse;
import bsdf;
import shape;
import cube;
import dyslang;

public struct Diffuse<Real : __BuiltinFloatingPointType> : IBsdf<Real> {
    dyslang::DynamicArray<float> data;
    IShape<Real> shape;
    vector<Real, 3> color;

    [require(cpp)] public __init(dyslang::IProperties p) {
        if (p.has("color")) {
            color = p.get<vector<Real, 3>>("color");
        }
        shape = Cube<Real>(p);
    }

    [require(cpp), mutating] public void traverse(dyslang::IProperties p) {
        p.set("color", &color);
        shape.traverse(p);
    }
};

