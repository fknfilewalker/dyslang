module shape;
import dyslang;

[anyValueSize(64)]
public interface IShape<Real : __BuiltinFloatingPointType> {
    [require(cpp), mutating] public void traverse(dyslang::IProperties p);
    Real get_float();
}
