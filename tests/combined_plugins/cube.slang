module cube;
import shape;
import bsdf;
import dyslang;

public struct Cube<Real : __BuiltinFloatingPointType> : IShape<Real> {
    IBsdf<Real> *bsdf;

    [require(cpp)] public __init(dyslang::IProperties p) {

    }

    [require(cpp), mutating] public void traverse(dyslang::IProperties p) {

    }

    Real get_float() {
        return Real(42.0f);
    }
};
