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
        : name{ std::move(name) }, 
	//size{ sizeof(T) }, 
	elements{ elements }
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
    //size_t size;
    size_t elements;
};

struct DynamicClass
{
	DynamicClass(std::string name, size_t entries = 1) : _name{ std::move(name) }
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
    void add_member(const std::string& name, size_t offset, size_t elements = 1)
    {
        if (has_member(name))
        {
            throw std::runtime_error("Member already exists: " + name);
        }
        E* d = nullptr;
        _entries.emplace_back(d, name, offset, elements);
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

    std::string _name;
    std::vector<Entry> _entries;
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

    std::vector<uint8_t>& payload() { return _data; }
    uint8_t* data() { return _data.data(); }

    DynamicClass dynamic_class;
    std::vector<uint8_t> _data;
};

int main(int argc, char* argv[]) {
	{
        DynamicClass dynVecClass("ivec2", 2);
        dynVecClass.add_member<int>("x", 0);
        dynVecClass.add_member<int>("y", 0);
        DynamicObject dynVecObj(dynVecClass, 8);
		dynVecObj.access_member<int>("x") = 4;
		dynVecObj.access_member<int>("y") = 8;

		DynamicClass dynClass("MyClass", 5);
        dynClass.add_member<int>("int_member", 0);
        dynClass.add_member<float>("float_member", 4);
        dynClass.add_member<double>("double_member", 8);
        dynClass.add_member<int>("int_array_member", 16, 4);
        dynClass.add_member<DynamicClass>("vec_member", 32, 1);
		DynamicObject dynObj(dynClass, 32);
        dynObj.access_member<int>("int_member") = 1;
		dynObj.access_member<float>("float_member") = 2.0f;
		dynObj.access_member<double>("double_member") = 3.067;
        int* ptr = dynObj.access_member<int*>("int_array_member");
        dynObj.access_member<int*>("int_array_member")[0] = 1;

        auto dm = dynObj.access_member<double>("double_member");
		printf("double_member: %f\n", dm);
        printf("has int_member: %d\n", dynObj.has_member("int_member"));
        printf("has foo_member: %d\n", dynObj.has_member("foo_member"));

		printf("%s\n", dynClass.to_string().c_str());
	}
}