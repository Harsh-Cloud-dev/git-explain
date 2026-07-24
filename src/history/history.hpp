#pragma once
#include <string>
#include <vector>
#include <deque>
#include <chrono>
#include <optional>

namespace git_explain {

struct HistoryEntry {
    std::string              command;      
    std::string              working_dir;
    int                      exit_code = 0;
    bool                     had_error = false;
    std::string              error_type;  
    std::chrono::system_clock::time_point timestamp;
};

class History {
public:
    static constexpr size_t kMaxEntries = 200;

    History();

    void add(const HistoryEntry& entry);

    const std::deque<HistoryEntry>& entries() const { return entries_; }

    
    std::vector<HistoryEntry> recent(size_t n = 20) const;

    
    std::vector<HistoryEntry> error_entries() const;

    
    bool save(const std::string& path = "") const;

    
    bool load(const std::string& path = "");

    
    static std::string default_path();

private:
    std::deque<HistoryEntry> entries_;
    static std::string format_entry(const HistoryEntry& e);
    static std::optional<HistoryEntry> parse_entry(const std::string& line);
};

} 
