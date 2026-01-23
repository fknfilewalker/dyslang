import bsdf;
import shape;

public struct Diffuse<Real : __BuiltinFloatingPointType> : IBsdf<Real> {
    IShape<Real>* shape;
};