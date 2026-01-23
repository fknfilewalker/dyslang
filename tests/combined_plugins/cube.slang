import shape;
import bsdf;

public struct Cube<Real : __BuiltinFloatingPointType> : IShape<Real> {
    IBsdf<Real>* bsdf;

    Real get_float() {
        return Real(42.0f);
    }
};