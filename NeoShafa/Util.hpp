#pragma once

#include <expected>  
#include <filesystem>  
#include <string_view>  
#include <print>  
#include <fstream>  

#include "Core.hpp"

namespace NeoShafa::Util {
    using namespace Core;
    template <typename T>
    static inline std::expected<void, BasicErrors> is_null(const T* pointer, std::string_view errorMessage = "") noexcept
    {
        if (pointer == nullptr) {
            if (!errorMessage.empty())
                std::println(
                    "ERROR(Custom): {}({})", 
                    errorMessage, 
                    static_cast<int32_t>(BasicErrors::PointerIsNull)
                );
            return std::unexpected(BasicErrors::PointerIsNull);
        }
        else
            return {};
    }

    static inline size_t hash(std::string_view string) {
        return std::hash<std::string>{}(string.data());
    }

    static inline  std::expected<size_t, BasicErrors> hash(const std::filesystem::path& path) {
        std::ifstream file{ path, std::ios::binary };
        if (!file)
            return std::unexpected(Core::BasicErrors::CannotReadFileError);

        std::string contents{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        };

        std::hash<std::string_view> hasher;
        return hasher(std::string_view{ contents });
    }

    static inline std::expected<void, BasicErrors> write(
        const std::filesystem::path& path,
        std::string_view content
    )
    {
        std::ofstream file{ path, std::ios::app };
        if (!file)
            return std::unexpected(Core::BasicErrors::CannotWriteFileError);

        file << content << "\n";

        return {};
    }
}