#include "../../include/dyslang/dyslang.h"
public interface IEmitter<Real : dyslang::floating_point> {
    annotations(Differentiable) vector<Real, 3> illuminate(vector<Real, 3>, vector<Real, 3>);
    annotations(Differentiable) vector<Real, 3> sample();
}