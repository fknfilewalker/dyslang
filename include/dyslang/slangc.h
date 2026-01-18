#pragma once
#include <string>
#include <memory>
#include <slang-com-ptr.h>
#include <slang.h>
#include <vector>

namespace dyslang {
	thread_local static Slang::ComPtr<slang::IGlobalSession> slangGlobalSession;

	struct SlangComposerPrivate;
	struct Slangc
	{
		using Hash = std::string;
		using ArgPair = std::pair<const char*, const char*>; // <Name, Value>

		Slangc(const std::vector<const char*>& includes, const std::vector<ArgPair>& defines);
		Slangc(const std::shared_ptr<SlangComposerPrivate>& p) : _p{ p } {}
        Slangc(const Slangc& other);
		~Slangc() = default;

		Slangc copy() { return Slangc(*this); }

		Slangc& add_module(std::string_view module_name, const std::vector<std::string_view>& entry_points = {});
		Slangc& add_module(std::string_view module_name, std::string_view module_path, const void* blob);
		Slangc compose() const;
		Slangc& hash(uint32_t entry, Hash& hash);
		// V use compose before using these to reflect on full source V
		Slangc& add_type_conformance(std::string_view interface_type, std::string_view conformance_type, int64_t id_override = -1);
		[[nodiscard]] std::array<uint32_t, 6> get_rtti_bytes(std::string_view interface_type, std::string_view conformance_type) const;
		Slangc& get_global_resource_array_binding(uint32_t& binding, uint32_t& space);
		// V use compose before using these to get full source V
		[[nodiscard]] std::vector<uint32_t> spv() const;
		[[nodiscard]] std::vector<uint8_t> glsl() const;
		[[nodiscard]] std::vector<uint32_t> entry(uint32_t entry) const;

	private:
		std::shared_ptr<SlangComposerPrivate> _p;
	};
}
