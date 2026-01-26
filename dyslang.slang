module dyslang;

namespace dyslang
{
    public struct DynamicArray<T> {
        public typedef T Element;
        public T *data;
        public uint64_t count;
    };

    public struct _DescriptorHandle<T : IOpaqueDescriptor> {
        public typedef vector<uint32_t, 2> Type;
        public vector<uint32_t, 2> id;
        public DescriptorHandle<T> get() {
            return { id };
        }
    };

    public interface IProperties {
        public bool has(NativeString key);
        public T get<T>(NativeString key);
        public void set<T>(NativeString key, T *value);
        public void set<T>(NativeString key, T **value);
        public void set<T : IOpaqueDescriptor>(NativeString key, _DescriptorHandle<T> *value);
        public void set<T : IArithmetic>(NativeString key, DynamicArray<T> *value);
    };
}
