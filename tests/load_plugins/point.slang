import emitter;
#include "../../include/dyslang/properties.h"

using namespace dyslang;
struct PointLight<Real : floating_point> : IEmitter<Real> {
    v3<Real> color;
    Real intensity;

    __init(Properties properties) {
        if (properties.has("color")) {
            color = properties.get<v3<Real>>("color");
        }
        if (properties.has("intensity")) {
            intensity = properties.get<Real>("intensity");
        }
    }

    void traverse(Properties properties) {
        properties.set("intensity", intensity);
        properties.set("color", color);
    }

    [[Differentiable]]
    v3<Real> illuminate(v3<Real>) {
        return v3<Real>(0);
    }

    [[Differentiable]]
    v3<Real> sample() {
        return v3<Real>(0);
    }
};

IMPLEMENT_PLUGIN(IEmitter, PointLight)