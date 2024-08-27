import emitter;

StructuredBuffer<IEmitter<float>> emitters;
RWStructuredBuffer<float> outputBuffer;

// ./slangc -stage compute -target glsl -o interface_from_buffer.glsl interface_from_buffer.slang
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    outputBuffer[0] = emitters[0].sample().x;
}
