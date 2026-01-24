module dyslang;

namespace dyslang
{
    public struct DynamicArray<T> {
        public T *data;
        public uint64_t count;
    };

    public struct _DescriptorHandle<T : IOpaqueDescriptor> {
        public vector<uint32_t, 2> id;
        public DescriptorHandle<T> get() {
            return { id };
        }
    };

    public interface IProperties {
        public bool has(NativeString key);
        public T get<T>(NativeString key);
        public void set<T>(NativeString key, T value);
    };
}
