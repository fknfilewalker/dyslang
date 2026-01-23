
[anyValueSize(128)]
public interface IShape<Real : __BuiltinFloatingPointType> {
    [require(cpp)] public void traverse();
    Real get_float();
}