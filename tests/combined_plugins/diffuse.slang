import bsdf;
import shape;

public struct Diffuse<Real : __BuiltinFloatingPointType> : IBsdf<Real> {
    IShape<Real> *shape;

    [require(cpp)] public __init() {}
    [require(cpp)] public void traverse() {}
};
