public interface IEmitter<Real : __BuiltinFloatingPointType> {
    [Differentiable] vector<Real, 3> illuminate(vector<Real, 3>, vector<Real, 3>);
    [Differentiable] vector<Real, 3> sample();
}