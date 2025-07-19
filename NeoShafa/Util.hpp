#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <expected>  
#include <filesystem>  
#include <string_view>  
#include <print>  
#include <fstream>  

#include <curl/curl.h>

#include "Core.hpp"
#include <cstdio>

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
        std::string_view content,
		bool append = false
    )
    {
        std::ios_base::openmode mode{ std::ios::out };
        if (append)
            mode |= std::ios::app;
        
        std::ofstream file{ path, mode };
        if (!file)
            return std::unexpected(Core::BasicErrors::CannotWriteFileError);

        file << content << "\n";

        return {};
    }

    static inline std::expected<std::vector<std::string>, BasicErrors> read(const std::filesystem::path& path)
    {
        std::ifstream file{ path };
        if (!file)
            return std::unexpected(Core::BasicErrors::CannotReadFileError);

        std::vector<std::string> lines{};
        std::string line{};
        while (std::getline(file, line))
        {
            lines.push_back(line);
        }

        return lines;
    }

    static inline size_t write_callback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
        return fwrite(ptr, size, nmemb, stream);
    }

    static inline std::expected<void, BasicErrors> download_file(
        std::string_view url,
        const std::filesystem::path& path
    ) {
        auto curl_deleter = [](CURL* c) { if (c) curl_easy_cleanup(c); };
        std::unique_ptr<CURL, decltype(curl_deleter)> curl(curl_easy_init(), curl_deleter);

        if (!curl) {
            return std::unexpected(BasicErrors::CurlInitError);
        }

        auto file_deleter = [](FILE* f) { if (f) fclose(f); };
        std::unique_ptr<FILE, decltype(file_deleter)> file(
            fopen(path.string().c_str(), "wb"),
            file_deleter
        );

        if (!file) return std::unexpected(BasicErrors::CannotOpenFileError);
        std::string url_str{ url };

        curl_easy_setopt(curl.get(), CURLOPT_URL, url_str.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, file.get());
        curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L);

        const CURLcode res = curl_easy_perform(curl.get());

        if (res != CURLE_OK) {
            std::filesystem::remove(path);
            return std::unexpected(BasicErrors::CurlDownloadError);
        }

        return {};
    }
}