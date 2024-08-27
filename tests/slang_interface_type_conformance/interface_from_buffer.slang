interface IFoo
{
    float method();
}

struct Impl1 : IFoo
{
    //float a;
    float method() { return 1.0f + 20.0f; }
}

struct Impl2 : IFoo
{
    float data2;
    float method() { return 1.0f + data2; }
}

RWStructuredBuffer<IFoo> inBuffer;
RWStructuredBuffer<float> outputBuffer;

// ./slangc -stage compute -target glsl -o interface_from_buffer.glsl interface_from_buffer.slang
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    outputBuffer[0] = inBuffer[0].method();
}
