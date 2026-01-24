module bsdf;
import dyslang;

[anyValueSize(128)]
public interface IBsdf<Real : __BuiltinFloatingPointType> {
    property dyslang::DynamicArray<float> data;
    // [require(cpp)] public void traverse(dyslang::IProperties p);
}
