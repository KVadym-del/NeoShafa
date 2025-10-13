#pragma once

#include <cstdint>
#include <string_view>

namespace NeoShafa::Core {

#define STRINGIFY_DETAIL(x) #x
#define STRINGIFY(x) STRINGIFY_DETAIL(x)

#define DEFINE_VERSION(VarName, Major, Minor, Patch)                                \
    constexpr const struct {                                                        \
        const uint32_t major{ Major };                                              \
        const uint32_t minor{ Minor };                                              \
        const uint32_t patch{ Patch };                                              \
                                                                                    \
        constexpr operator const char*() const noexcept {                           \
            return STRINGIFY(Major) "." STRINGIFY(Minor) "." STRINGIFY(Patch);      \
        }                                                                           \
    } VarName {}
    DEFINE_VERSION(NEOSHAFA_VERSION, 0, 0, 1);

    /*
    enum class BasicErrors : int32_t {
        PointerIsNull = -1,
        CannotReadFileError,
        CannotWriteFileError,
        CannotOpenFileError,

        CurlInitError,
        CurlDownloadError,
    };

    enum class RouterErrors : int32_t {
        GenericError = 1
    };

    enum class ProjectDataScraperErrors : int32_t {
        GenericProjectSetupError = 100,
        ProjectConfigFileNotExist,
        UnexpectedParsingError,

        MissingProjectName,
        MissingProjectVersion,
        MissingProjectLanguage,
        MissingProjectType,
        MissingProjectCVersion,
        MissingProjectCppVersion,

        UnexpectedProjectTypeError,
    };

    enum class ProjectConfigureErrors : int32_t {
        GenericProjectConfigureError = 200,
        InvalidEnvironmentError,
        DirectoryIterationError,

        GeneratinFileHashError,
        GetFileAttributesError,
        SetFileAttributesError,

        ReadingProjecCahedSourceError,

        InvalidInstallationDirectoryError,
        InvalidMsvcVersionError,
        InvalidClPathError,
    };

    enum class ProjectBuildErrors : int32_t {
        GenericBuildError = 300,
    };

    enum class LuaError : int32_t {
		CannotReadFileError = 400,
		FileNotFoundError,
        ExecutionError
    };
    */

    enum class ErrorCode : int32_t {
        Unknown = -2,

        PointerIsNull = -1,

        CurlInitError,
        CurlDownloadError,

        GenericError = 50,

        GenericProjectSetupError = 100,
        ProjectConfigFileNotExist,
        UnexpectedParsingError,

        MissingProjectName,
        MissingProjectVersion,
        MissingProjectLanguage,
        MissingProjectType,
        MissingProjectCVersion,
        MissingProjectCppVersion,

        UnexpectedProjectTypeError,

        GenericProjectConfigureError = 200,
        InvalidEnvironmentError,
        DirectoryIterationError,

        GeneratinFileHashError,
        GetFileAttributesError,
        SetFileAttributesError,

        ReadingProjectCahedSourceError,

        InvalidInstallationDirectoryError,
        InvalidMsvcVersionError,
        InvalidClPathError,

        GenericBuildError = 300,

		RunningCommandError,

        CannotReadFileError = 400,
        CannotWriteFileError,
        CannotOpenFileError,
        FileNotFoundError,
        ExecutionError
    };

    struct Error {
        ErrorCode code{ ErrorCode::Unknown };
        std::string message{};
    };

    constexpr inline Error make_error(const ErrorCode& code, std::string_view msg) {
        return { code, std::string(msg) };
    }

    template <typename T>
    using Expected = std::expected<T, Error>;

    using ExpectedVoid = std::expected<void, Error>;

    /*==================================================*/

    enum class SupportedCompilers : int32_t {
        Unknown = 0,
        MSVC,
        Clang,
        GCC
    };
    inline constexpr static std::string_view to_string(const SupportedCompilers& supportedCompilers)
    {
        switch (supportedCompilers)
        {
        case SupportedCompilers::Unknown:   return "Unknown";
        case SupportedCompilers::MSVC:      return "MSVC";
        case SupportedCompilers::Clang:     return "Clang";
        case SupportedCompilers::GCC:       return "GCC";
        default:
            return "Unknown";
        }
    }
    inline constexpr static SupportedCompilers to_supportedCompiler(std::string_view string) {  
        if (string == "Unknown")  
            return SupportedCompilers::Unknown;  
        else if (string == "MSVC")  
            return SupportedCompilers::MSVC;  
        else if (string == "Clang")  
            return SupportedCompilers::Clang;  
        else if (string == "GCC")  
            return SupportedCompilers::GCC;  
        return SupportedCompilers::Unknown;  
    }

    enum class SupportedTargets : int32_t {
        Unknown = 0,
        Windows,
        Linux,
	};
    inline constexpr static std::string_view to_string(const SupportedTargets& supportedCompilers)
    {
        switch (supportedCompilers)
        {
        case SupportedTargets::Unknown:   return "Unknown";
        case SupportedTargets::Windows:   return "MSVC";
        case SupportedTargets::Linux:     return "Clang";
        default:
            return "Unknown";
        }
    }
    inline constexpr static SupportedTargets to_supportedTargets(std::string_view string) {
        if (string == "Unknown")
            return SupportedTargets::Unknown;
        else if (string == "Windows")
            return SupportedTargets::Windows;
        else if (string == "Linux")
            return SupportedTargets::Linux;
        else if (string == "GCC")
        return SupportedTargets::Unknown;
    }
}
