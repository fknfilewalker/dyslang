#include "../../include/dyslang/dyslang.h"
[anyValueSize(256)]
public interface IEmitter<Real : __BuiltinFloatingPointType> {
    annotations(Differentiable) vector<Real, 3> illuminate(vector<Real, 3>, vector<Real, 3>);
    annotations(Differentiable) vector<Real, 3> sample();
}