import emitter;
#include "../../include/dyslang/dyslang.h"

struct PointLight<Real : __BuiltinFloatingPointType> : IEmitter<Real> {
    vector<Real, 3> color;
    Real intensity;

    __init(dyslang::Properties properties) {
        if (properties.has("color")) {
            color = properties.get<vector<Real, 3>>("color");
        }
        if (properties.has("intensity")) {
            intensity = properties.get<Real>("intensity");
        }
    }

    void traverse(dyslang::Properties properties) {
        properties.set("intensity", intensity);
        properties.set("color", color);
    }

    [[Differentiable]]
    vector<Real, 3> illuminate(vector<Real, 3>) {
        return vector<Real, 3>(0);
    }

    [[Differentiable]]
    vector<Real, 3> sample() {
        return vector<Real, 3>(0);
    }
};

IMPLEMENT_PLUGIN(IEmitter, PointLight)