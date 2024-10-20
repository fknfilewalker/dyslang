#include "../../include/dyslang/dyslang.h"
import emitter;

struct PointLight<Real : __BuiltinFloatingPointType> : IEmitter<Real> {
    vector<Real, 3> position;
    Real range;
    vector<Real, 3> color;
    Real intensity;
    dyslang::Texture2DRef<Real> texture;

    __init(dyslang::Properties properties) {
        if (properties.has("position")) {
            position = properties.get<vector<Real, 3>>("position");
        } else position = vector<Real, 3>(Real(0.0));

        if (properties.has("range")) {
            range = properties.get<Real>("range");
        } else range = Real(0.0);

        if (properties.has("color")) {
            color = properties.get<vector<Real, 3>>("color");
        } else color = vector<Real, 3>(Real(1.0));

        if (properties.has("intensity")) {
            intensity = properties.get<Real>("intensity");
        } else intensity = Real(1.0);

        if (properties.has("texture")) {
            texture = properties.getTexture2DRef<Real>("texture");
        }
    }

    void traverse(dyslang::Properties properties) {
        properties.set("position", position);
        properties.set("range", range);
        properties.set("color", color);
        properties.set("intensity", intensity);
        properties.setResourceRef("texture", texture);
    }

    [[Differentiable]] vector<Real, 3> illuminate(vector<Real, 3> hit_pos_ws, vector<Real, 3> normal_ws) {
        vector<Real, 3> dir = position - hit_pos_ws;
        const Real dist = length(dir);
        const Real att = Real(1.0) / (Real(1.0) + dist * dist);
        const vector<Real, 3> emission = no_diff texture.get().Load(int3(0, 0, 0));
        return color * intensity * att * emission * max(dot(normal_ws, normalize(dir)), Real(0.0));
    }

    [[Differentiable]] vector<Real, 3> sample() { return position; }
};

IMPLEMENT_PLUGIN(IEmitter, PointLight)