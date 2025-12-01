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

struct Entry
{
    template<typename T>
    Entry(std::span<uint8_t> heap, const size_t offset, std::string name, const T& entry, const size_t length = 1)
        : name{ std::move(name) }, size{ sizeof(T) }, length{ length }, offset{
        offset
        } {
        std::memcpy(heap.data() + offset, &entry, size);
        value = reinterpret_cast<T*>(heap.data() + offset);
    }
    //Entry(std::span<uint8_t> heap, const size_t offset, std::string name, const std::vector<Entry>& entries, const size_t size, const size_t length = 1)
    //    : name{ std::move(name) }, size{ size }, length{ length }, offset{
    //    offset
    //    } {
    //    value = entries;
    //}

    std::string name;
    std::variant<void*, int*, float* /*, std::vector<Entry>*/> value;
    size_t size;
    size_t length;
    size_t offset;
};

//struct CompositEntry
//{
//    std::string name;
//    std::vector<Entry> entries;
//    size_t size;
//    size_t length;
//    size_t offset;
//};

struct DynamicObject
{
    DynamicObject(size_t size)
    {
        data.resize(size);
    }

    // copy constructor
    DynamicObject(const DynamicObject& other)
        : entries{ other.entries }, data{ other.data }
    {
        update_backing();
    }

    // move constructor
    DynamicObject(DynamicObject&& other) noexcept
        : entries{ std::move(other.entries) }, data{ std::move(other.data) }
    {
        update_backing();
    }

    //void add_struct_member(const std::string& name, size_t size, size_t offset)
    //{
    //    entries.emplace_back(data, offset, name, std::vector<Entry>(), size, 1);
    //}

    template<typename E>
    void add_member(const std::string& name, const E& value, size_t offset)
    {
        if (has_member(name))
        {
            throw std::runtime_error("Member already exists: " + name);
        }
        entries.emplace_back(data, offset, name, value, 1);
    }

    bool has_member(const std::string& name) const
    {
        for (const auto& entry : entries)
        {
            if (entry.name == name)
            {
                return true;
            }
        }
        return false;
    }

    template<typename E>
    E& access_member(const std::string& name)
    {
        for (auto& entry : entries)
        {
            if (std::holds_alternative<E*>(entry.value) && entry.name == name)
            {
                return *std::get<E*>(entry.value);
            }
        }
        throw std::runtime_error("Member not found: " + name);
    }

    std::vector<uint8_t>& payload()
    {
        return data;
    }

    std::vector<Entry> entries;
    std::vector<uint8_t> data;

private:
    void update_backing()
    {
        for (auto& entry : entries)
        {
            // Update the heap pointer in each entry to point to the new underlying data
            std::visit([&]<typename V>(V & v) {
                //if constexpr (std::is_same_v<V, std::vector<Entry>>)
                //{
                //} else
                if constexpr (std::is_pointer_v<V>)
                {
                    v = reinterpret_cast<std::remove_pointer_t<V>*>(data.data() + entry.offset);
                }
            }, entry.value);
        }
    }
};

struct Entry2
{
    template<typename T>
    Entry2(const size_t offset, std::string name, const size_t length = 1)
        : name{ std::move(name) }, size{ sizeof(T) }, length{ length }
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
            size_t offset = reinterpret_cast<size_t>(std::get<E*>(_offset));
            return *reinterpret_cast<E*>(heap.data() + offset);
        }
    }

    std::string name;
    std::variant<void*, int*, float*, double*> _offset;
    size_t size;
    size_t length;
};

struct DynamicObject2;
struct DynamicClass
{
    bool has_member(const std::string& name) const
    {
        for (const auto& entry : entries)
        {
            if (entry.name == name)
            {
                return true;
            }
        }
        return false;
    }

    std::vector<Entry2> entries;
};

struct DynamicObject2
{
    DynamicObject2(const DynamicClass& cls)
        : dynamic_class{ cls }
    {

    }

    template<typename E>
    E& access_member(const std::string& name)
    {
        for (auto& entry : dynamic_class.entries)
        {
            return entry.get<E>(data);
        }
        throw std::runtime_error("Member not found: " + name);
    }

    std::vector<uint8_t>& payload()
    {
        return data;
    }

    DynamicClass dynamic_class;
    std::vector<uint8_t> data;
};

int main(int argc, char* argv[]) {
	{
        DynamicObject dc(8);
        int x = 3;
        int y = 4;
        dc.add_member("x", x, 0);
        dc.add_member("y", y, 4);
	}
	{
        DynamicObject dynObj(32);
        int aa = 1;
        float bb = 2.0f;
        double cc = 3.067;
        dynObj.add_member("int_member", aa, 0);
        dynObj.add_member("float_member", bb, 4);
        dynObj.add_member("double_member", cc, 8);
        dynObj.add_member("my_struct::int", aa, 16);
        dynObj.access_member<int>("my_struct::int") += 20;
        //auto& struct_member = dynObj.access_member<std::vector<Entry>>("struct_member");

        Entry2 e2(dynObj.payload(), 8, "double_member", cc);
        double& double_member = e2.get<double>(dynObj.payload());

        DynamicObject dynObj2 = std::move(dynObj);
        dynObj.data.resize(16);
        dynObj.add_member("int_member", aa, 0);
        int& int_member = dynObj.access_member<int>("int_member");
        int_member += 10;
	}
}