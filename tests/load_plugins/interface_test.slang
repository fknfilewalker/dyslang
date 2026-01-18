import emitter;

[vk::binding(0, 0)]
__DynamicResource<__DynamicResourceKind::General> __global_resource_array[];

StructuredBuffer<IEmitter<float>> emitters;
RWStructuredBuffer<float> outputBuffer;

// ./slangc -stage compute -target glsl -o interface_from_buffer.glsl interface_from_buffer.slang
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    vector<float, 3> P = { 0.0, 0.0, 0.0 };
    vector<float, 3> N = { 0.0, 1.0, 0.0 };
    outputBuffer[0] = emitters[0].illuminate(P, N).x;
}
