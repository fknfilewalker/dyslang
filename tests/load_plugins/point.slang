#include "../../include/dyslang/dyslang.h"
import emitter;

struct PointLight<Real : __BuiltinFloatingPointType> : IEmitter<Real> {
    dyslang::i64 id;
    vector<Real, 3> position;
    Real range;
    vector<Real, 3> color;
    Real intensity;
    dyslang::m3x3<Real> transform;
    dyslang::Texture2DRef<vector<Real, 3>> texture;

    __init(dyslang::Properties properties) {
        if (properties.has("id")) {
            id = properties.get<dyslang::i64>("id");
        } else id = -1;

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

        if (properties.has("transform")) {
            transform = properties.get<dyslang::m3x3<Real>>("transform");
        } else transform = dyslang::m3x3<Real>(1.0);

        if (properties.has("texture")) {
            texture = properties.get<Texture2D<vector<Real, 3>>>("texture");
        }
    }

    void traverse(dyslang::Properties properties) {
        properties.set("id", id);
        properties.set("position", position);
        properties.set("range", range);
        properties.set("color", color);
        properties.set("intensity", intensity);
        properties.set("transform", transform);
        properties.set("texture", texture);
        Real arr[10] = { Real(1.0), Real(2.0), Real(3.0), Real(4.0), Real(5.0), Real(6.0), Real(7.0), Real(8.0), Real(9.0), Real(10.0) };
        properties.set("arr_test", arr);
    }

    [Differentiable] vector<Real, 3> illuminate(vector<Real, 3> hit_pos_ws, vector<Real, 3> normal_ws) {
        vector<Real, 3> dir = position - hit_pos_ws;
        const Real dist = length(dir);
        const Real att = Real(1.0) / (Real(1.0) + dist * dist);
        const vector<Real, 3> emission = no_diff texture.get().Load(int3(0, 0, 0));
        return color * intensity * att * emission * max(dot(normal_ws, normalize(dir)), Real(0.0));
    }

    [Differentiable] vector<Real, 3> sample() { return position; }
};

IMPLEMENT_PLUGIN(IEmitter, PointLight)