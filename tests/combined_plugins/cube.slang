import shape;
import bsdf;

public struct Cube<Real : __BuiltinFloatingPointType> : IShape<Real> {
    IBsdf<Real> *bsdf;

    [require(cpp)] public __init() {}
    [require(cpp)] public void traverse() {}

    Real get_float() {
        return Real(42.0f);
    }
};
