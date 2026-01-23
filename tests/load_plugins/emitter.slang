// #include "../../include/dyslang/dyslang.h"
[anyValueSize(256)]
public interface IEmitter<Real : __BuiltinFloatingPointType> {

    // [mutating, require(cpp)] void traverse(dyslang::Properties properties);

    vector<Real, 3> illuminate(vector<Real, 3>, vector<Real, 3>);
    [Differentiable] vector<Real, 3> sample();
}