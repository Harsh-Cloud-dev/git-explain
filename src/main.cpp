#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cstdlib>

#ifndef _WIN32
  #include <unistd.h>
#endif

#include "executor/executor.hpp"
#include "detection/detector.hpp"
#include "recovery/recovery.hpp"
#include "history/history.hpp"
#include "ui/ui.hpp"

using namespace git_explain;

static void print_version() {
    std::cout << "git-explain version 0.1.0\n";
}

int main(int argc, char* argv[]) {
    
    std::vector<std::string> args(argv + 1, argv + argc);

    bool color = UI::detect_color_support();
    UI   ui(std::cout, color);

    
    if (args.empty() || args[0] == "--help" || args[0] == "-h") {
        ui.print_usage();
        return 0;
    }
    if (args[0] == "--version" || args[0] == "-v") {
        print_version();
        return 0;
    }

    History history;
    history.load();

    if (args[0] == "--history") {
        ui.print_history(history.recent(30));
        return 0;
    }

    
    char cwd_buf[4096] = {};
    std::string working_dir;
#ifndef _WIN32
    if (getcwd(cwd_buf, sizeof(cwd_buf))) working_dir = cwd_buf;
#endif

    
    
    
    
    
    if (Executor::needs_passthrough(args)) {
        Executor executor;
        ExecuteResult result = executor.run_passthrough(args);

        HistoryEntry entry;
        entry.command     = Executor::build_command(args);
        entry.exit_code   = result.exit_code;
        entry.had_error   = result.exit_code != 0;
        entry.working_dir = working_dir;
        entry.timestamp   = std::chrono::system_clock::now();
        if (entry.had_error) entry.error_type = "(interactive command — not diagnosed)";
        history.add(entry);
        history.save();

        ui.print_passthrough_banner(result.exit_code == 0, entry.command);
        return result.exit_code;
    }

    
    Executor executor;
    ExecuteResult result = executor.git(args);

    
    HistoryEntry entry;
    entry.command     = Executor::build_command(args);
    entry.exit_code   = result.exit_code;
    entry.had_error   = result.exit_code != 0;
    entry.working_dir = working_dir;
    entry.timestamp   = std::chrono::system_clock::now();

    
    
    
    
    if (result.exit_code == 0) {
        ui.print_banner(true, entry.command);
        history.add(entry);
        history.save();
        return 0;
    }

    
    
    
    ui.print_banner(false, entry.command);

    Detector   detector;
    Recovery   recovery;

    GitError          error      = detector.detect(result, args);
    RecoverySuggestion suggestion = recovery.suggest(error);

    entry.error_type = error_type_name(error.type);
    history.add(entry);
    history.save();

    ui.print_analysis(error, suggestion);

    return result.exit_code;
}
