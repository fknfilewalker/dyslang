module shape;
import dyslang;

[anyValueSize(128)]
public interface IShape<Real : __BuiltinFloatingPointType> {
    // [require(cpp)] public void traverse(dyslang::IProperties p);
    Real get_float();
}
