#pragma once

#include <cstdlib>
#include <filesystem>

#define ERR_STR(literal_str) "[ERROR]" literal_str
#define INF_STR(literal_str) "[INFO]" literal_str
#define WARN_STR(literal_str) "[WARNING]" literal_str

namespace tmr {

/// Checks the list of variables in the environment for paths, and appends
/// the given `tail` to it. If none have valid values, uses `fallback` to merge
/// with `tail` instead.
inline std::filesystem::path create_path_from_vars(
    std::filesystem::path fallback, std::filesystem::path tail,
    std::initializer_list<const char *> vars
) {
    for (auto &var : vars) {
        const char *varpath = std::getenv(var);
        if (!varpath || *varpath == '\0') {
            continue;
        }
        return std::filesystem::path(varpath) / tail;
    }
    return fallback / tail;
}

}  // namespace tmr