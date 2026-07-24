#pragma once
#include "errors/error_types.hpp"
#include "recovery/recovery.hpp"
#include "history/history.hpp"
#include <string>
#include <vector>
#include <ostream>

namespace git_explain {



namespace ansi {
    inline constexpr const char* reset   = "\033[0m";
    inline constexpr const char* bold    = "\033[1m";
    inline constexpr const char* dim     = "\033[2m";
    inline constexpr const char* red     = "\033[31m";
    inline constexpr const char* green   = "\033[32m";
    inline constexpr const char* yellow  = "\033[33m";
    inline constexpr const char* blue    = "\033[34m";
    inline constexpr const char* magenta = "\033[35m";
    inline constexpr const char* cyan    = "\033[36m";
    inline constexpr const char* white   = "\033[37m";
    inline constexpr const char* b_red   = "\033[91m";
    inline constexpr const char* b_green = "\033[92m";
    inline constexpr const char* b_cyan  = "\033[96m";
} 



class UI {
public:
    explicit UI(std::ostream& out, bool color = true);

    
    void print_raw_output(const std::string& stdout_out,
                          const std::string& stderr_out) const;

    
    void print_analysis(const GitError& error,
                        const RecoverySuggestion& suggestion) const;

    
    void print_banner(bool success, const std::string& command) const;

    
    
    
    void print_passthrough_banner(bool success, const std::string& command) const;

    
    void print_history(const std::vector<HistoryEntry>& entries) const;

    
    void print_usage() const;

    
    static bool detect_color_support();

private:
    std::ostream& out_;
    bool          color_;

    void section(const std::string& title) const;
    void step(size_t n, const FixStep& step) const;
    std::string colorize(const std::string& text, const char* code) const;
    std::string bold(const std::string& text) const;
};

} 
