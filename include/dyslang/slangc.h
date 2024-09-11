#pragma once
#include <string>
#include <memory>
#include <vector>

namespace dyslang {
	struct Slangc
	{
		using ArgPair = std::pair<const char*, const char*>; // <Name, Value>
		using Hash = std::string;

		Slangc(const std::vector<const char*>& includes, const std::vector<ArgPair>& defines);
		~Slangc();

		Slangc(const Slangc&) = delete;
		Slangc& operator=(Slangc& other) = delete;
		Slangc(Slangc&& other) = delete;
		Slangc& operator=(Slangc&& other) = delete;

		void addModule(const std::string_view module_name) const;
		void addModule(std::string_view module_name, std::string_view module_path, const void* blob) const;
		void addEntryPoint(const std::string_view module_name, const std::string_view entryPointName) const;
		void finalizeModulesAndEntryPoints() const;
		std::pair<unsigned, unsigned> globalResourceArrayBinding() const;
		void addTypeConformance(const std::string_view conformance_type, const std::string_view interface_type, int64_t id_override = -1) const;
		Hash compose() const;

		[[nodiscard]] std::vector<uint8_t> compile() const;
	private:

		struct SlangcPrivate;
		SlangcPrivate* _p; // pimpl did not work with unique ptr
	};
}
