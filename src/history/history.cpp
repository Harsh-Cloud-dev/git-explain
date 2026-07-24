#include "history.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

namespace git_explain {

namespace {
    
    std::string sanitize_field(std::string s) {
        std::replace(s.begin(), s.end(), '\t', ' ');
        std::replace(s.begin(), s.end(), '\n', ' ');
        return s;
    }
}

History::History() = default;

void History::add(const HistoryEntry& entry) {
    entries_.push_back(entry);
    while (entries_.size() > kMaxEntries) {
        entries_.pop_front();
    }
}

std::vector<HistoryEntry> History::recent(size_t n) const {
    std::vector<HistoryEntry> result;
    size_t count = std::min(n, entries_.size());
    result.reserve(count);
    
    for (auto it = entries_.rbegin(); it != entries_.rend() && result.size() < count; ++it) {
        result.push_back(*it);
    }
    return result;
}

std::vector<HistoryEntry> History::error_entries() const {
    std::vector<HistoryEntry> result;
    for (const auto& e : entries_) {
        if (e.had_error) result.push_back(e);
    }
    return result;
}

std::string History::default_path() {
    const char* home = std::getenv("HOME");
    std::string home_dir = home ? home : ".";
    return home_dir + "/.git-explain/history.log";
}

std::string History::format_entry(const HistoryEntry& e) {
    
    auto epoch = std::chrono::duration_cast<std::chrono::seconds>(
        e.timestamp.time_since_epoch()).count();

    std::ostringstream oss;
    oss << epoch << '\t'
        << e.exit_code << '\t'
        << (e.had_error ? 1 : 0) << '\t'
        << sanitize_field(e.error_type) << '\t'
        << sanitize_field(e.working_dir) << '\t'
        << sanitize_field(e.command);
    return oss.str();
}

std::optional<HistoryEntry> History::parse_entry(const std::string& line) {
    if (line.empty()) return std::nullopt;

    std::vector<std::string> fields;
    std::istringstream iss(line);
    std::string field;
    while (std::getline(iss, field, '\t')) {
        fields.push_back(field);
    }
    
    if (fields.size() < 6) return std::nullopt;

    HistoryEntry e;
    try {
        long long epoch_secs = std::stoll(fields[0]);
        e.timestamp = std::chrono::system_clock::time_point(std::chrono::seconds(epoch_secs));
        e.exit_code = std::stoi(fields[1]);
        e.had_error = fields[2] == "1";
    } catch (const std::exception&) {
        return std::nullopt;
    }
    e.error_type  = fields[3];
    e.working_dir = fields[4];

    
    
    
    e.command = fields[5];
    for (size_t i = 6; i < fields.size(); ++i) {
        e.command += '\t' + fields[i];
    }

    return e;
}

bool History::save(const std::string& path) const {
    std::string target = path.empty() ? default_path() : path;

    
    auto slash = target.find_last_of('/');
    if (slash != std::string::npos) {
        std::string dir = target.substr(0, slash);
        std::string cmd = "mkdir -p '" + dir + "'";
        std::system(cmd.c_str());
    }

    std::ofstream out(target, std::ios::out | std::ios::trunc);
    if (!out.is_open()) return false;

    for (const auto& e : entries_) {
        out << format_entry(e) << '\n';
    }
    return out.good();
}

bool History::load(const std::string& path) {
    std::string target = path.empty() ? default_path() : path;

    std::ifstream in(target);
    if (!in.is_open()) {
        
        return true;
    }

    entries_.clear();
    std::string line;
    while (std::getline(in, line)) {
        auto parsed = parse_entry(line);
        if (parsed) {
            entries_.push_back(*parsed);
        }
    }
    while (entries_.size() > kMaxEntries) {
        entries_.pop_front();
    }
    return true;
}

} 
