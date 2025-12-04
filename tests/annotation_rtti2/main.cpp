#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <slang-com-ptr.h>
#include <slang-com-helper.h>
#include <stdexcept>
#include <any>
#include <functional>
#include <map>
#include <variant>
#include <span>
using namespace Slang;

struct DynamicClass;
struct Entry
{
    template<typename T>
    Entry(T* ptr, std::string name, const size_t offset, const size_t elements = 1)
        : name{ std::move(name) }, _bytes{ sizeof(T) * elements }, elements{ elements }
    {
        uint8_t* v = nullptr;
        v += offset;
        _offset = reinterpret_cast<T*>(v);
    }

    template<typename E>
    E& get(std::span<uint8_t> heap)
    {
        if (std::holds_alternative<E*>(_offset))
        {
            // add offet to heap base
            E* ptr = std::get<E*>(_offset);
            auto offset = reinterpret_cast<size_t>(std::get<E*>(_offset));
            return *reinterpret_cast<E*>(heap.data() + offset);
        }
        throw std::runtime_error("Wrong Type");
    }

    std::string name;
    std::variant<void*, int*, float*, double*, DynamicClass*> _offset;
    size_t _bytes;
    size_t elements;
};

struct DynamicClass
{
	DynamicClass(std::string name, size_t entries = 1) : _bytes{ 0 }, _name{ std::move(name) }
    {
        this->_entries.reserve(entries);
    }

    bool has_member(const std::string& name) const
    {
        for (const auto& entry : _entries)
        {
            if (entry.name == name)
            {
                return true;
            }
        }
        return false;
    }

    template<typename E>
    void add_member(const std::string& name, size_t elements = 1)
    {
        if (has_member(name))
        {
            throw std::runtime_error("Member already exists: " + name);
        }
        E* d = nullptr;
        _entries.emplace_back(d, name, _bytes, elements);
		_bytes += sizeof(E) * elements;
    }

    std::string to_string() const
	{
		std::string out = "DynamicClass: " + _name + "\n";
        for (auto& e : _entries)
        {
			out += "  Member: " + e.name + "\n";
        }
        return out;
	}

    size_t _bytes;
    std::string _name;
    std::vector<Entry> _entries;
};

struct DynamicObjectView
{
    DynamicObjectView(DynamicClass cls, const std::span<uint8_t> span) : dynamic_class{ std::move(cls) }, _data{ span } {}

    template<typename E>
    std::conditional_t<std::is_pointer_v<E>, E, E&> access_member(const std::string& name)
    {
        using NonPtrType = std::remove_pointer_t<E>;
        for (auto& entry : dynamic_class._entries)
        {
            if (std::holds_alternative<NonPtrType*>(entry._offset) && entry.name == name)
            {
                if constexpr (std::is_pointer_v<E>) return &entry.get<NonPtrType>(_data);
                else return entry.get<NonPtrType>(_data);
            }
        }
        throw std::runtime_error("Member not found: " + name);
    }

    DynamicClass dynamic_class;
    std::span<uint8_t> _data;
};

struct DynamicObject
{
    DynamicObject(DynamicClass cls, const size_t size)
		: dynamic_class{ std::move(cls) }, _data(size, 0) {}

    // copy constructor
    DynamicObject(const DynamicObject& other)
        : dynamic_class{ other.dynamic_class }, _data{ other._data } {}

    // move constructor
    DynamicObject(DynamicObject&& other) noexcept
        : dynamic_class{ std::move(other.dynamic_class) }, _data{ std::move(other._data) } {}

    bool has_member(const std::string& name) const
    {
        return dynamic_class.has_member(name);
    }

    template<typename E>
    std::conditional_t<std::is_pointer_v<E>, E, E&> access_member(const std::string& name)
    {
        using NonPtrType = std::remove_pointer_t<E>;
        for (auto& entry : dynamic_class._entries)
        {
            if (std::holds_alternative<NonPtrType*>(entry._offset) && entry.name == name)
            {
                if constexpr (std::is_pointer_v<E>) return &entry.get<NonPtrType>(_data);
                else return entry.get<NonPtrType>(_data);
            }
        }
        throw std::runtime_error("Member not found: " + name);
    }

    DynamicObjectView access_member(DynamicClass& cls, const std::string& name)
    {
        for (auto& entry : dynamic_class._entries)
        {
            if (std::holds_alternative<DynamicClass*>(entry._offset) && entry.name == name)
            {
                auto& e = std::get<DynamicClass*>(entry._offset);
                return { cls, std::span<uint8_t>{ 
                	_data.data() + reinterpret_cast<size_t>(e), cls._bytes  } };
            }
        }
        throw std::runtime_error("Member not found: " + name);
    }

    std::vector<uint8_t>& payload() { return _data; }
    uint8_t* data() { return _data.data(); }

    DynamicClass dynamic_class;
    std::vector<uint8_t> _data;
};

struct GroundTruth {
    int a;
    float b;
    double c;
};

struct Vec2 {
    int x;
    int y;
};
struct GroundTruth2 {
    int a;
    uint8_t b;
    double c;
    Vec2 v;
};
int main(int argc, char* argv[]) {
    {
		printf("GroundTruth: size %zu alignof %zu\n", sizeof(GroundTruth), alignof(GroundTruth));
		printf("  a: offset %zu size %zu alignof %zu\n", offsetof(GroundTruth, a), sizeof(GroundTruth::a), alignof(decltype(GroundTruth::a)));
		printf("  b: offset %zu size %zu alignof %zu\n", offsetof(GroundTruth, b), sizeof(GroundTruth::b), alignof(decltype(GroundTruth::b)));
		printf("  c: offset %zu size %zu alignof %zu\n", offsetof(GroundTruth, c), sizeof(GroundTruth::c), alignof(decltype(GroundTruth::c)));
        printf("\n");
        printf("GroundTruth2: size %zu alignof %zu\n", sizeof(GroundTruth2), alignof(GroundTruth2));
		printf("  a: offset %zu size %zu alignof %zu\n", offsetof(GroundTruth2, a), sizeof(GroundTruth2::a), alignof(decltype(GroundTruth2::a)));
		printf("  b: offset %zu size %zu alignof %zu\n", offsetof(GroundTruth2, b), sizeof(GroundTruth2::b), alignof(decltype(GroundTruth2::b)));
		printf("  c: offset %zu size %zu alignof %zu\n", offsetof(GroundTruth2, c), sizeof(GroundTruth2::c), alignof(decltype(GroundTruth2::c)));
		printf("  v: offset %zu size %zu alignof %zu\n", offsetof(GroundTruth2, v), sizeof(GroundTruth2::v), alignof(decltype(GroundTruth2::v)));
        printf("\n");
        printf("Vec2: size %zu alignof %zu\n", sizeof(Vec2), alignof(Vec2));
        printf("\n");
    }
	{
        DynamicClass dynVecClass("ivec2", 2);
        dynVecClass.add_member<int>("x");
        dynVecClass.add_member<int>("y");
        DynamicObject dynVecObj(dynVecClass, 8);
		dynVecObj.access_member<int>("x") = 4;
		dynVecObj.access_member<int>("y") = 8;

		DynamicClass dynClass("MyClass", 5);
        dynClass.add_member<int>("int_member");
        dynClass.add_member<float>("float_member");
        dynClass.add_member<double>("double_member");
        dynClass.add_member<int>("int_array_member", 4);
        dynClass.add_member<DynamicClass>("vec_member", 1);
		DynamicObject dynObj(dynClass, 64);
        dynObj.access_member<int>("int_member") = 1;
		dynObj.access_member<float>("float_member") = 2.0f;
		dynObj.access_member<double>("double_member") = 3.067;
        int* ptr = dynObj.access_member<int*>("int_array_member");
        dynObj.access_member<int*>("int_array_member")[0] = 1;
        dynObj.access_member(dynVecClass, "vec_member").access_member<int>("x") = 1;
        dynObj.access_member(dynVecClass, "vec_member").access_member<int>("y") = 8;
        auto vvv = dynObj.access_member(dynVecClass, "vec_member");

        auto dm = dynObj.access_member<double>("double_member");
		printf("double_member: %f\n", dm);
        printf("has int_member: %d\n", dynObj.has_member("int_member"));
        printf("has foo_member: %d\n", dynObj.has_member("foo_member"));

		printf("%s\n", dynClass.to_string().c_str());
	}
}