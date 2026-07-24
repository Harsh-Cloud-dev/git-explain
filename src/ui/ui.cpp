#include "ui.hpp"
#include "history/history.hpp"
#include <iostream>
#include <sstream>
#include <cstdlib>

#ifndef _WIN32
  #include <unistd.h>
#endif

namespace git_explain {



UI::UI(std::ostream& out, bool color)
    : out_(out), color_(color) {}

bool UI::detect_color_support() {
#ifdef _WIN32
    return false;
#else
    
    if (std::getenv("NO_COLOR")) return false;
    if (!isatty(STDOUT_FILENO))  return false;
    const char* term = std::getenv("TERM");
    if (!term) return false;
    std::string t(term);
    return t != "dumb";
#endif
}



std::string UI::colorize(const std::string& text, const char* code) const {
    if (!color_) return text;
    return std::string(code) + text + ansi::reset;
}

std::string UI::bold(const std::string& text) const {
    if (!color_) return text;
    return std::string(ansi::bold) + text + ansi::reset;
}

namespace {
    
    std::string repeat_utf8(const std::string& unit, int n) {
        std::string result;
        result.reserve(unit.size() * static_cast<size_t>(std::max(0, n)));
        for (int i = 0; i < n; ++i) result += unit;
        return result;
    }
}

void UI::section(const std::string& title) const {
    out_ << "\n" << colorize("── " + title + " ", ansi::cyan)
         << colorize(repeat_utf8("─", std::max(0, 60 - (int)title.size() - 4)), ansi::dim)
         << "\n\n";
}

void UI::step(size_t n, const FixStep& step) const {
    std::string num   = colorize(std::to_string(n) + ".", ansi::b_cyan);
    std::string desc  = bold(step.description);
    out_ << "  " << num << " " << desc << "\n";
    if (!step.command.empty()) {
        std::string prefix = step.is_dangerous
            ? colorize("  ⚠  $ ", ansi::yellow)
            : colorize("     $ ", ansi::dim);
        out_ << prefix << colorize(step.command, ansi::b_green) << "\n";
    }
    out_ << "\n";
}



void UI::print_banner(bool success, const std::string& command) const {
    if (success) {
        out_ << colorize("✓ ", ansi::b_green) << bold(command)
             << colorize(" completed successfully.", ansi::dim) << "\n";
    } else {
        out_ << "\n" << colorize("✗ ", ansi::b_red) << bold("git command failed")
             << colorize(" — analyzing error…", ansi::dim) << "\n";
    }
}

void UI::print_passthrough_banner(bool success, const std::string& command) const {
    if (success) {
        out_ << colorize("✓ ", ansi::b_green) << bold(command)
             << colorize(" completed successfully.", ansi::dim) << "\n";
    } else {
        out_ << colorize("✗ ", ansi::b_red) << bold(command)
             << colorize(" failed.", ansi::dim) << "\n"
             << colorize("  (this command needed direct terminal control, "
                          "so git-explain couldn't capture output to diagnose — "
                          "see git's own message above.)", ansi::dim) << "\n";
    }
}

void UI::print_raw_output(const std::string& stdout_out,
                          const std::string& stderr_out) const {
    if (!stdout_out.empty()) {
        section("git output");
        out_ << colorize(stdout_out, ansi::white);
    }
    if (!stderr_out.empty()) {
        section("git error output");
        out_ << colorize(stderr_out, ansi::b_red);
    }
}

void UI::print_analysis(const GitError& error,
                        const RecoverySuggestion& suggestion) const {
    
    section("diagnosis");
    out_ << "  " << bold("Error type : ")
         << colorize(error_type_name(error.type), ansi::magenta) << "\n";

    if (!suggestion.cause.empty()) {
        out_ << "  " << bold("Root cause : ")
             << suggestion.cause << "\n";
    }

    if (!error.affected_files.empty()) {
        out_ << "  " << bold("Affected   : ") << "\n";
        for (const auto& f : error.affected_files) {
            out_ << "      " << colorize("• ", ansi::yellow) << f << "\n";
        }
    }

    
    if (!suggestion.explanation.empty()) {
        section("what happened");
        
        std::istringstream words(suggestion.explanation);
        std::string word, line;
        while (words >> word) {
            if (line.size() + word.size() + 1 > 72 && !line.empty()) {
                out_ << "  " << line << "\n";
                line.clear();
            }
            if (!line.empty()) line += ' ';
            line += word;
        }
        if (!line.empty()) out_ << "  " << line << "\n";
    }

    
    if (!suggestion.steps.empty()) {
        section("how to fix it");
        for (size_t i = 0; i < suggestion.steps.size(); ++i) {
            step(i + 1, suggestion.steps[i]);
        }
    }

    out_ << colorize(repeat_utf8("─", 64), ansi::dim) << "\n";
    out_ << colorize("  git-explain | ", ansi::dim)
         << colorize("run `git-explain --help` for more options\n", ansi::dim);
}

void UI::print_history(const std::vector<HistoryEntry>& entries) const {
    section("command history");
    if (entries.empty()) {
        out_ << "  (no history yet)\n";
        return;
    }
    out_ << "  " << colorize("  #  STATUS  COMMAND\n", ansi::dim);
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& e = entries[i];
        std::string status = e.had_error
            ? colorize("✗ error", ansi::b_red)
            : colorize("✓ ok   ", ansi::b_green);
        out_ << "  " << colorize(std::to_string(i + 1), ansi::dim) << "  "
             << status << "  " << e.command << "\n";
        if (e.had_error && !e.error_type.empty()) {
            out_ << "             " << colorize("↳ " + e.error_type, ansi::yellow) << "\n";
        }
    }
}

void UI::print_usage() const {
    out_ << "\n"
         << bold("git-explain") << " — understand and fix git errors\n\n"
         << bold("USAGE\n")
         << "  git-explain [git-command] [args...]\n"
         << "  git-explain --history\n"
         << "  git-explain --version\n"
         << "  git-explain --help\n\n"
         << bold("EXAMPLES\n")
         << "  git-explain push origin main\n"
         << "  git-explain merge feature-branch\n"
         << "  git-explain stash pop\n\n"
         << bold("HOW IT WORKS\n")
         << "  git-explain runs the git command you provide,\n"
         << "  intercepts any errors, classifies them, and shows\n"
         << "  you a clear explanation plus step-by-step fix instructions.\n\n";
}

} 
