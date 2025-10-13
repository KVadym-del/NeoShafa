#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <expected>  
#include <filesystem>  
#include <string_view>  
#include <print>  
#include <format>  
#include <fstream>  

#include <curl/curl.h>

#include <boost/process.hpp>

#include "Core.hpp"
#include <cstdio>
#include <iostream>

namespace NeoShafa::Util {
    using namespace Core;

    namespace BoostProcess = boost::process;

    template <typename T>
    static inline ExpectedVoid is_null(const T* pointer, std::string_view errorMessage = "")
    {
        if (pointer == nullptr) {
            if (!errorMessage.empty())
            return std::unexpected(make_error(ErrorCode::PointerIsNull, std::format(
                "{}({})",
                errorMessage,
                static_cast<int32_t>(ErrorCode::PointerIsNull)
            )));
        }
        else
            return {};
    }

    static inline size_t hash(std::string_view string) {
        return std::hash<std::string>{}(string.data());
    }

    static inline Expected<size_t> hash(const std::filesystem::path& path) {
        std::ifstream file{ path, std::ios::binary };
        if (!file)
            return std::unexpected(make_error(ErrorCode::CannotReadFileError, std::format("Cannot open file {} to create hash.", path.string())));

        std::string contents{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        };

        std::hash<std::string_view> hasher;
        return hasher(std::string_view{ contents });
    }

    static inline ExpectedVoid write(
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
            return std::unexpected(make_error(ErrorCode::CannotWriteFileError, ""));

        file << content;

        return {};
    }

    static inline Expected<std::vector<std::string>> read(const std::filesystem::path& path)
    {
        std::ifstream file{ path };
        if (!file)
            return std::unexpected(make_error(ErrorCode::CannotReadFileError, std::format("Cannot open file {}.", path.string())));

        std::vector<std::string> lines{};
        std::string line{};
        while (std::getline(file, line))
        {
            lines.push_back(line);
        }

        return lines;
    }

    static inline size_t write_callback(void* ptr, size_t size, size_t nmemb, FILE* stream) noexcept {
        return fwrite(ptr, size, nmemb, stream);
    }

    static inline ExpectedVoid download_file(
        std::string_view url,
        const std::filesystem::path& path
    ) {
        auto curl_deleter = [](CURL* c) { if (c) curl_easy_cleanup(c); };
        std::unique_ptr<CURL, decltype(curl_deleter)> curl(curl_easy_init(), curl_deleter);

        if (!curl) {
            return std::unexpected(make_error(ErrorCode::CurlInitError, std::format("Cannot init curl {}.", path.string())));
        }

        auto file_deleter = [](FILE* f) { if (f) fclose(f); };
        std::unique_ptr<FILE, decltype(file_deleter)> file(
            fopen(path.string().c_str(), "wb"),
            file_deleter
        );

        if (!file) return std::unexpected(make_error(ErrorCode::CannotOpenFileError, std::format("Cannot open file for this: {}.", path.string())));
        std::string url_str{ url };

        curl_easy_setopt(curl.get(), CURLOPT_URL, url_str.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, file.get());
        curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L);

        const CURLcode res = curl_easy_perform(curl.get());

        if (res != CURLE_OK) {
            std::filesystem::remove(path);
            return std::unexpected(make_error(ErrorCode::CurlDownloadError, std::format("Error trying to access: {}.", path.string())));
        }

        return {};
    }

    inline static Expected<std::string> run_command(
        const std::filesystem::path& executable,
        const std::vector<std::string>& args,
		int32_t& exitCode
    ) {
        if (!std::filesystem::exists(executable))
            return std::unexpected(
                Core::make_error(
                    Core::ErrorCode::RunningCommandError,
                    std::format("Executable not found at {}", executable.string())
                )
            );

        BoostProcess::ipstream pipeStream{};
        BoostProcess::ipstream errorStream{};
        std::string out{};
        try {
            BoostProcess::child child{
                executable.string(),
                args,
                BoostProcess::std_out > pipeStream,
                BoostProcess::std_err > errorStream
            };

            std::stringstream sstream{};
            sstream << pipeStream.rdbuf();
            sstream << errorStream.rdbuf();
            out = sstream.str();

            child.wait();
			exitCode = child.exit_code();
            if (child.exit_code() != 0) {
                std::println(
                    std::cerr,
                    "ERROR: {} exited with an error code: {}",
                    executable.string(),
                    child.exit_code()
                );
            }
        }
        catch (const boost::process::process_error& error) {
            return std::unexpected(
                Core::make_error(
                    Core::ErrorCode::RunningCommandError,
					std::format("Error executing command: {}", error.what())
                )
		    );
        }
        if (!out.empty()) {
            out.erase(out.find_last_not_of("\r\n") + 1);
        }
        return out;
    }
}