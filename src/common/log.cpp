#include "common/log.hpp"

#include "common/config.hpp"

namespace hydra {

namespace {

constexpr usize MAX_LOG_FILES = 3;

}

void Logger::EnsureOutputStream() {
    if (ofs.has_value())
        return;

    const auto logs_path = CONFIG_INSTANCE.GetLogsPath();

    // List log files
    std::vector<std::filesystem::directory_entry> logs;
    for (const auto& entry : std::filesystem::directory_iterator(logs_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".log")
            logs.push_back(entry);
    }

    // Delete oldest logs if needed
    if (logs.size() >= MAX_LOG_FILES) {
        std::sort(logs.begin(), logs.end(), [](const auto& a, const auto& b) {
            return std::filesystem::last_write_time(a) <
                   std::filesystem::last_write_time(b);
        });

        const usize to_delete = logs.size() - (MAX_LOG_FILES - 1);
        for (usize i = 0; i < to_delete; ++i)
            std::filesystem::remove(logs[i]);
    }

    // Create a new log file
    // TODO: version
    const auto path = fmt::format("{}/" APP_NAME "_{:%Y-%m-%d_%H-%M-%S}.log",
                                  logs_path, std::chrono::system_clock::now());
    ofs = std::ofstream(path);

    // Get start time
    start_time = clock_t::now();
}

} // namespace hydra
