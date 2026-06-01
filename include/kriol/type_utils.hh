#ifndef _KRIOL_TYPE_UTILS_HEADER
#define _KRIOL_TYPE_UTILS_HEADER

#include <cctype>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace kriol {
namespace typeutils {

struct ParsedType {
    std::string Base;
    std::vector<std::size_t> ArrayDims;

    bool isArray() const { return !ArrayDims.empty(); }
};

// Parses a type string like "int[10][20]" into its base type and array dimensions.
inline std::optional<ParsedType> parseType(const std::string& type) {
    ParsedType out;

    const std::size_t firstBracket = type.find('[');
    if (firstBracket == std::string::npos) {
        out.Base = type;
        return out;
    }

    if (firstBracket == 0) return std::nullopt;
    out.Base = type.substr(0, firstBracket);

    std::size_t pos = firstBracket;
    while (pos < type.size()) {
        if (type[pos] != '[') return std::nullopt;
        const std::size_t close = type.find(']', pos + 1);
        if (close == std::string::npos || close == pos + 1) return std::nullopt;

        std::size_t dim = 0;
        for (std::size_t i = pos + 1; i < close; ++i) {
            unsigned char ch = static_cast<unsigned char>(type[i]);
            if (!std::isdigit(ch)) return std::nullopt;
            dim = dim * 10 + static_cast<std::size_t>(ch - '0');
        }

        out.ArrayDims.push_back(dim);
        pos = close + 1;
    }

    return out;
}

// Checks if the given type string represents an array type.
inline bool isArrayType(const std::string& type) {
    auto parsed = parseType(type);
    return parsed && parsed->isArray();
}

// Returns the size of the first array dimension if the type is an array, or std::nullopt otherwise.
inline std::optional<std::size_t> firstArrayDim(const std::string& type) {
    auto parsed = parseType(type);
    if (!parsed || parsed->ArrayDims.empty()) return std::nullopt;
    return parsed->ArrayDims.front();
}

// Returns the element type of an array type. If the input is not an array, returns the input type.
inline std::string arrayElementType(const std::string& type) {
    auto parsed = parseType(type);
    if (!parsed || parsed->ArrayDims.empty()) return type;

    if (parsed->ArrayDims.size() == 1) return parsed->Base;

    std::string rebuilt = parsed->Base;
    for (std::size_t i = 1; i < parsed->ArrayDims.size(); ++i)
        rebuilt += "[" + std::to_string(parsed->ArrayDims[i]) + "]";
    return rebuilt;
}

} // namespace typeutils
} // namespace kriol

#endif // _KRIOL_TYPE_UTILS_HEADER