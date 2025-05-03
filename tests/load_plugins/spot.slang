#include "../../include/dyslang/dyslang.h"
import emitter;

struct SpotLight<Real : dyslang::floating_point> : IEmitter<Real> {
    Real test;

    __init(dyslang::Properties properties) {
        if (properties.has("test")) {
            test = properties.get<Real>("test");
        } else test = Real(0.0);
    }

    void traverse(dyslang::Properties properties) {}

    [Differentiable] vector<Real, 3> illuminate(vector<Real, 3> hit_pos_ws, vector<Real, 3> normal_ws) {
        return {};
    }

    [Differentiable] vector<Real, 3> sample() { return {}; }
};

IMPLEMENT_PLUGIN(IEmitter, SpotLight)