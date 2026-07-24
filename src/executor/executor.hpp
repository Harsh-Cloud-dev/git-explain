#pragma once
#include <string>
#include <vector>

namespace git_explain {

struct ExecuteResult {
    
    
    
    
    
    
    std::string stdout_output;
    std::string stderr_output;
    int         exit_code = 0;
    bool        timed_out = false;
};

class Executor {
public:
    explicit Executor(int timeout_seconds = 30);

    
    
    
    
    ExecuteResult run(const std::string& command) const;

    
    ExecuteResult git(const std::vector<std::string>& args) const;

    
    
    
    
    
    
    
    static bool needs_passthrough(const std::vector<std::string>& args);

    
    
    
    
    
    ExecuteResult run_passthrough(const std::vector<std::string>& args) const;

    
    static std::string build_command(const std::vector<std::string>& args);

private:
    int timeout_seconds_;
};

} 
