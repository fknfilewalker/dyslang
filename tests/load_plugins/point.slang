#include "../../include/dyslang/dyslang.h"
import emitter;

struct PointLight<Real : __BuiltinFloatingPointType> : IEmitter<Real> {
    int id;
    vector<Real, 3> position;
    Real range;
    vector<Real, 3> color;
    Real intensity;
    matrix<Real, 3, 3> transform;
    Real arr[10];
    IEmitter<Real>* ptr;
    dyslang::DynamicArray<matrix<Real, 3, 3>> dynamic_transform;
    dyslang::_DescriptorHandle<Texture2D<float3>> texture;

    __init(dyslang::Properties properties) {
        if (properties.has("id")) {
            id = properties.get<int>("id");
        } else id = -1;

        if (properties.has("position")) {
            position = properties.get<vector<Real, 3>>("position");
        } else position = vector<Real, 3>(0.0);

        if (properties.has("range")) {
            range = properties.get<Real>("range");
        } else range = Real(0.0);

        if (properties.has("color")) {
            color = properties.get<vector<Real, 3>>("color");
        } else color = vector<Real, 3>(1.0);

        if (properties.has("intensity")) {
            intensity = properties.get<Real>("intensity");
        } else intensity = Real(1.0);

        if (properties.has("transform")) {
            transform = properties.get<matrix<Real, 3, 3>>("transform");
        } else transform = matrix<Real, 3, 3>(1.0);

        if (properties.has("dynamic_transform")) {
            dynamic_transform = properties.get<dyslang::DynamicArray<matrix<Real, 3, 3>>>("dynamic_transform");
        } else dynamic_transform = dyslang::DynamicArray<matrix<Real, 3, 3>>();

        if (properties.has("ptr")) {
            ptr = properties.get<IEmitter<Real>*>("ptr");
        } else ptr = nullptr;

        if (properties.has("texture")) {
            texture = properties.get<dyslang::_DescriptorHandle<Texture2D<float3>>>("texture");
        }

        arr = { Real(1.0), Real(2.0), Real(3.0), Real(4.0), Real(5.0), Real(6.0), Real(7.0), Real(8.0), Real(9.0), Real(10.0) };
    }

    [mutating]
    void traverse(dyslang::Properties properties) {
        properties.set("id", &id);
        properties.set("position", &position);
        properties.set("range", &range);
        properties.set("color", &color);
        properties.set("intensity", &intensity);
        properties.set("transform", &transform);
        properties.set("dynamic_transform", &dynamic_transform);
        properties.set("arr_test", &arr);
        properties.set("ptr", &ptr);
        properties.set("texture", &texture);
    }

    vector<Real, 3> illuminate(vector<Real, 3> hit_pos_ws, vector<Real, 3> normal_ws) {
        vector<Real, 3> dir = position - hit_pos_ws;
        const Real dist = length(dir);
        const Real att = Real(1.0) / (Real(1.0) + dist * dist);
        const vector<Real, 3> emission = vector<Real, 3>(texture.get().Load(int3(0, 0, 0)));
        return color * intensity * att * emission * max(dot(normal_ws, normalize(dir)), Real(0.0));
    }

    [Differentiable, NoDiffThis] vector<Real, 3> sample() { return position; }
};

IMPLEMENT_PLUGIN(IEmitter, PointLight)