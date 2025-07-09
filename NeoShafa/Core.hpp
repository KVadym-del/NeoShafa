#pragma once

#include <cstdint>

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
    
    enum class BasicErrors : int32_t {
        PointerIsNull = -1,
        CannotReadFileError,
        CannotWriteFileError,
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
        MissingProjectLanguage
    };

    enum class ProjectConfigureErrors : int32_t {
        GenericProjectConfigureError = 200,
        InvalidEnvironmentError,
        DirectoryIterationError,
        GeneratinFileHashError,
        GetFileAttributesError,
        SetFileAttributesError,
    };
}
