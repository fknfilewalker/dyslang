module shape;
import dyslang;

[anyValueSize(64)]
public interface IShape<Real : __BuiltinFloatingPointType> : dyslang::IConstructible {
    Real get_float();
}
