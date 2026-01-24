module bsdf;
import dyslang;

[anyValueSize(128)]
public interface IBsdf<Real : __BuiltinFloatingPointType> {
    property dyslang::DynamicArray<float> data;
    [require(cpp), mutating] public void traverse(dyslang::IProperties p);
}
