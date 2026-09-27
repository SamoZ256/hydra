// Based on yuzu's IPSwitchCompiler

#include "core/horizon/filesystem/patch/pchtxt.hpp"

namespace hydra::horizon::filesystem::patch {

namespace {

constexpr std::string_view NSOBID_DIRECTIVE = "@nsobid-";
constexpr std::string_view OFFSET_SHIFT_DIRECTIVE = "@flag offset_shift";

bool isSpace(const char c) {
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}

std::string_view trim(std::string_view str) {
    usize begin = 0;
    while (begin < str.size() && isSpace(str[begin]))
        begin++;

    usize end = str.size();
    while (end > begin && isSpace(str[end - 1]))
        end--;

    return str.substr(begin, end - begin);
}

std::optional<std::vector<u8>> parseHexBytes(const std::string_view str,
                                             const bool little_endian) {
    if (str.empty() || str.size() % 2 != 0)
        return std::nullopt;

    std::vector<u8> bytes(str.size() / 2);
    for (usize i = 0; i < bytes.size(); i++) {
        const auto byte = fromChars<u8>(str.substr(i * 2, 2), 16);
        if (!byte)
            return std::nullopt;

        bytes[i] = *byte;
    }

    // IPS records are big-endian
    if (little_endian)
        std::ranges::reverse(bytes);

    return bytes;
}

std::vector<u8> parseEscapedString(const std::string_view str) {
    std::vector<u8> bytes;
    bytes.reserve(str.size());
    for (usize i = 0; i < str.size(); i++) {
        if (str[i] != '\\' || i + 1 == str.size()) {
            bytes.push_back(static_cast<u8>(str[i]));
            continue;
        }

        i++;
        char escaped;
        switch (str[i]) {
        case 'a':
            escaped = '\a';
            break;
        case 'b':
            escaped = '\b';
            break;
        case 'f':
            escaped = '\f';
            break;
        case 'n':
            escaped = '\n';
            break;
        case 'r':
            escaped = '\r';
            break;
        case 't':
            escaped = '\t';
            break;
        case 'v':
            escaped = '\v';
            break;
        default:
            escaped = str[i];
            break;
        }
        bytes.push_back(static_cast<u8>(escaped));
    }

    return bytes;
}

std::optional<i64> parseOffsetShift(const std::string_view str) {
    auto value = trim(str.substr(OFFSET_SHIFT_DIRECTIVE.size()));
    i32 base = 10;
    if (value.starts_with("0x") || value.starts_with("0X")) {
        value = value.substr(2);
        base = 16;
    }

    const auto shift = fromChars<i64>(value, base);
    if (!shift)
        return std::nullopt;

    return *shift;
}

// Strips a trailing comment
std::string_view stripComment(const std::string_view line,
                              std::string_view& out_comment) {
    char quote = '\0';
    for (usize i = 0; i < line.size(); i++) {
        const char c = line[i];
        if (c == '\\') {
            i++;
        } else if (c == '"' || c == '\'') {
            quote = (c == quote) ? '\0' : c;
        } else if (quote == '\0' && (c == '/' || c == '#')) {
            auto comment = line.substr(i);
            while (!comment.empty() &&
                   (comment.front() == '/' || comment.front() == '#'))
                comment = comment.substr(1);

            out_comment = trim(comment);
            return trim(line.substr(0, i));
        }
    }

    return trim(line);
}

class Parser {
  public:
    explicit Parser(const std::string_view name_) : name{name_} {}

    bool parseLine(const std::string_view line);

    std::optional<Patch> finish() {
        if (!build_id) {
            LOG_ERROR(Loader, "Patch \"{}\" has no @nsobid directive", name);
            return std::nullopt;
        }

        if (!blocks.empty())
            return Patch(*build_id, name, std::move(blocks));

        LOG_WARN(Loader, "Patch \"{}\" contains no patches", name);
        return std::nullopt;
    }

    void setComment(const std::string_view comment) { last_comment = comment; }

  private:
    std::string name;
    std::optional<BuildId> build_id;
    std::vector<Block> blocks;

    std::string last_comment;
    bool little_endian{true};
    i64 offset_shift{0};

    // Helpers
    bool parseDirective(std::string_view line);
    void parseRecord(std::string_view line);

    void beginBlock(const bool enabled) {
        blocks.push_back({.name = std::move(last_comment),
                          .enabled = enabled,
                          .records = {}});
        last_comment.clear();
    }
};

bool Parser::parseDirective(const std::string_view line) {
    if (line.size() < 2) {
        LOG_WARN(Loader, "Empty patch directive");
        return true;
    }

    switch (std::tolower(static_cast<unsigned char>(line[1]))) {
    case 'n': {
        if (!line.starts_with(NSOBID_DIRECTIVE))
            break;

        build_id = buildIdFromString(line.substr(NSOBID_DIRECTIVE.size()));
        if (!build_id)
            LOG_ERROR(Loader, "Invalid build id ID \"{}\"", line);
        break;
    }
    case 's':
        return false;
    case 'e':
        beginBlock(true);
        break;
    case 'd':
        beginBlock(false);
        break;
    case 'f': {
        if (line.starts_with(OFFSET_SHIFT_DIRECTIVE)) {
            if (const auto shift = parseOffsetShift(line))
                offset_shift = *shift;
            else
                LOG_WARN(Loader, "Invalid offset shift in \"{}\"", line);
        }
        break;
    }
    case 'l':
        little_endian = true;
        break;
    case 'b':
        little_endian = false;
        break;
    default:
        LOG_WARN(Loader, "Unknown patch directive \"{}\"", line);
        break;
    }

    return true;
}

void Parser::parseRecord(const std::string_view line) {
    if (blocks.empty()) {
        LOG_WARN(Loader, "Patch record outside of a block: \"{}\"", line);
        return;
    }

    const auto separator = line.find_first_of(" \t");
    if (separator == std::string_view::npos) {
        LOG_WARN(Loader, "Malformed patch record: \"{}\"", line);
        return;
    }

    const auto offset = fromChars<u64>(trim(line.substr(0, separator)), 16);
    if (!offset) {
        LOG_WARN(Loader, "Invalid patch offset in \"{}\"", line);
        return;
    }

    const auto shifted = static_cast<i64>(*offset) + offset_shift;
    if (shifted < 0 || shifted > static_cast<i64>(allOnes<u32>())) {
        LOG_WARN(Loader, "Patch offset out of range in \"{}\"", line);
        return;
    }

    const auto value = trim(line.substr(separator + 1));
    std::vector<u8> data;
    if (value.size() >= 2 && (value.front() == '"' || value.front() == '\'') &&
        value.back() == value.front()) {
        data = parseEscapedString(value.substr(1, value.size() - 2));
    } else if (const auto bytes = parseHexBytes(value, little_endian)) {
        data = *bytes;
    } else {
        LOG_WARN(Loader, "Invalid patch value in \"{}\"", line);
        return;
    }

    blocks.back().records.push_back(
        {.offset = static_cast<u32>(shifted), .data = std::move(data)});
}

bool Parser::parseLine(const std::string_view line) {
    if (line.starts_with('@'))
        return parseDirective(line);

    parseRecord(line);
    return true;
}

} // namespace

std::optional<Patch> parsePchtxt(const std::string_view text,
                                 const std::string_view name) {
    Parser parser(name);
    for (const auto& raw_line : split<std::string_view>(text, '\n')) {
        std::string_view comment;
        const auto line = stripComment(trim(raw_line), comment);
        if (!comment.empty())
            parser.setComment(comment);

        if (line.empty())
            continue;

        if (!parser.parseLine(line))
            break;
    }

    return parser.finish();
}

} // namespace hydra::horizon::filesystem::patch
