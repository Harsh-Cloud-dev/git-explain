#pragma once
#include "errors/error_types.hpp"
#include <string>
#include <vector>

namespace git_explain {

struct FixStep {
    std::string description;   
    std::string command;       
    bool        is_dangerous = false; 
};

struct RecoverySuggestion {
    std::string              explanation;  
    std::string              cause;        
    std::vector<FixStep>     steps;        
    bool                     has_autofix = false;
};

class Recovery {
public:
    RecoverySuggestion suggest(const GitError& error) const;

private:
    RecoverySuggestion for_unknown_command(const GitError& e) const;
    RecoverySuggestion for_not_a_repo(const GitError& e) const;
    RecoverySuggestion for_merge_conflict(const GitError& e) const;
    RecoverySuggestion for_push_rejected(const GitError& e) const;
    RecoverySuggestion for_branch_already_exists(const GitError& e) const;
    RecoverySuggestion for_branch_not_found(const GitError& e) const;
    RecoverySuggestion for_detached_head(const GitError& e) const;
    RecoverySuggestion for_uncommitted_changes(const GitError& e) const;
    RecoverySuggestion for_no_tracking_branch(const GitError& e) const;
    RecoverySuggestion for_remote_not_found(const GitError& e) const;
    RecoverySuggestion for_auth_failed(const GitError& e) const;
    RecoverySuggestion for_permission_denied(const GitError& e) const;
    RecoverySuggestion for_stash_empty(const GitError& e) const;
    RecoverySuggestion for_rebase_in_progress(const GitError& e) const;
    RecoverySuggestion for_merge_in_progress(const GitError& e) const;
    RecoverySuggestion for_user_not_configured(const GitError& e) const;
    RecoverySuggestion for_untracked_overwrite(const GitError& e) const;
    RecoverySuggestion for_nothing_to_commit(const GitError& e) const;
    RecoverySuggestion for_unknown(const GitError& e) const;
};

} 
