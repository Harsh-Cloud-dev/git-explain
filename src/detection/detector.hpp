#pragma once
#include "errors/error_types.hpp"
#include "executor/executor.hpp"
#include <string>
#include <vector>

namespace git_explain {

class Detector {
public:
    
    GitError detect(const ExecuteResult& result,
                    const std::vector<std::string>& original_args = {}) const;

    
    GitError detect_from_stderr(const std::string& stderr_text,
                                int exit_code = 1) const;

private:
    
    bool try_unknown_command(const std::string& s, GitError& err) const;
    bool try_not_a_repo(const std::string& s, GitError& err) const;
    bool try_merge_conflict(const std::string& s, GitError& err) const;
    bool try_push_rejected(const std::string& s, GitError& err) const;
    bool try_branch_errors(const std::string& s, GitError& err) const;
    bool try_remote_errors(const std::string& s, GitError& err) const;
    bool try_auth_errors(const std::string& s, GitError& err) const;
    bool try_working_tree_errors(const std::string& s, GitError& err) const;
    bool try_stash_errors(const std::string& s, GitError& err) const;
    bool try_rebase_merge_in_progress(const std::string& s, GitError& err) const;
    bool try_detached_head(const std::string& s, GitError& err) const;
    bool try_user_not_configured(const std::string& s, GitError& err) const;
    bool try_commit_not_found(const std::string& s, GitError& err) const;
    bool try_nothing_to_commit(const std::string& s, GitError& err) const;

    
    std::vector<std::string> extract_files(const std::string& text,
                                           const std::string& prefix) const;
};

} 
