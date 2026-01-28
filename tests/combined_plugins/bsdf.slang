module bsdf;
import dyslang;

[anyValueSize(128)]
public interface IBsdf<Real : __BuiltinFloatingPointType> : dyslang::IConstructible {
    property dyslang::DynamicArray<float> data;
}
