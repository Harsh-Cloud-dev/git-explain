#include "detector.hpp"
#include <algorithm>
#include <sstream>

namespace git_explain {



static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}





static std::string extract_after(const std::string& text, const std::string& prefix) {
    auto pos = text.find(prefix);
    if (pos == std::string::npos) return {};
    pos += prefix.size();

    auto line_end = text.find('\n', pos);
    if (line_end == std::string::npos) line_end = text.size();

    char open_quote = prefix.empty() ? '\0' : prefix.back();
    size_t end = line_end;
    if (open_quote == '\'' || open_quote == '"') {
        auto close = text.find(open_quote, pos);
        if (close != std::string::npos && close <= line_end) end = close;
    }

    std::string val = text.substr(pos, end - pos);
    
    while (!val.empty() && (val.front() == ' ' || val.front() == '\'' || val.front() == '"')) val.erase(val.begin());
    while (!val.empty() && (val.back()  == ' ' || val.back()  == '\'' || val.back()  == '"' || val.back() == '.')) val.pop_back();
    return val;
}



GitError Detector::detect(const ExecuteResult& result,
                          const std::vector<std::string>& ) const {
    
    std::string combined = result.stderr_output + "\n" + result.stdout_output;
    return detect_from_stderr(combined, result.exit_code);
}

GitError Detector::detect_from_stderr(const std::string& stderr_text, int exit_code) const {
    GitError err;
    err.message   = stderr_text;
    err.exit_code = exit_code;

    
    if (try_merge_conflict(stderr_text, err))            return err;
    if (try_rebase_merge_in_progress(stderr_text, err))  return err;
    if (try_push_rejected(stderr_text, err))             return err;
    if (try_not_a_repo(stderr_text, err))                return err;
    if (try_detached_head(stderr_text, err))             return err;
    if (try_user_not_configured(stderr_text, err))       return err;
    if (try_branch_errors(stderr_text, err))             return err;
    
    
    
    if (try_auth_errors(stderr_text, err))               return err;
    if (try_remote_errors(stderr_text, err))             return err;
    if (try_working_tree_errors(stderr_text, err))       return err;
    if (try_stash_errors(stderr_text, err))              return err;
    if (try_commit_not_found(stderr_text, err))          return err;
    if (try_nothing_to_commit(stderr_text, err))         return err;
    if (try_unknown_command(stderr_text, err))           return err;

    err.type = ErrorType::Unknown;
    return err;
}



bool Detector::try_unknown_command(const std::string& s, GitError& err) const {
    if (contains(s, "is not a git command")) {
        err.type    = ErrorType::UnknownCommand;
        err.context = extract_after(s, "git: '");
        return true;
    }
    if (contains(s, "unknown switch") || contains(s, "unknown option") ||
        contains(s, "invalid option") || contains(s, "unrecognized argument")) {
        err.type = ErrorType::InvalidFlag;
        return true;
    }
    if (contains(s, "did you mean") && contains(s, "git")) {
        err.type = ErrorType::AmbiguousCommand;
        return true;
    }
    return false;
}

bool Detector::try_not_a_repo(const std::string& s, GitError& err) const {
    if (contains(s, "not a git repository") || contains(s, "Not a git repository")) {
        err.type = ErrorType::NotAGitRepo;
        return true;
    }
    return false;
}

bool Detector::try_merge_conflict(const std::string& s, GitError& err) const {
    if (contains(s, "CONFLICT") && (contains(s, "Merge conflict") || contains(s, "content:"))) {
        err.type           = ErrorType::MergeConflict;
        err.affected_files = extract_files(s, "CONFLICT (content): Merge conflict in ");
        
        auto del_files = extract_files(s, "CONFLICT (modify/delete): ");
        err.affected_files.insert(err.affected_files.end(), del_files.begin(), del_files.end());
        return true;
    }
    if (contains(s, "error: could not apply") && contains(s, "cherry-pick")) {
        err.type = ErrorType::CherryPickConflict;
        return true;
    }
    if (contains(s, "you have unmerged files") || contains(s, "unmerged paths")) {
        err.type = ErrorType::MergeInProgress;
        return true;
    }
    return false;
}

bool Detector::try_push_rejected(const std::string& s, GitError& err) const {
    if (contains(s, "failed to push") ||
        (contains(s, "rejected") && contains(s, "non-fast-forward"))) {
        err.type = ErrorType::PushRejected;
        return true;
    }
    return false;
}

bool Detector::try_branch_errors(const std::string& s, GitError& err) const {
    if (contains(s, "already exists") && contains(s, "branch")) {
        err.type    = ErrorType::BranchAlreadyExists;
        err.context = extract_after(s, "named '");
        return true;
    }
    if (contains(s, "cannot delete branch") && contains(s, "checked out")) {
        err.type = ErrorType::CannotDeleteCurrent;
        return true;
    }
    if ((contains(s, "did not match any") || contains(s, "pathspec")) &&
        (contains(s, "branch") || contains(s, "known to git"))) {
        err.type    = ErrorType::BranchNotFound;
        err.context = extract_after(s, "pathspec '");
        return true;
    }
    return false;
}

bool Detector::try_remote_errors(const std::string& s, GitError& err) const {
    if (contains(s, "does not appear to be a git repository") ||
        contains(s, "Could not read from remote repository") ||
        contains(s, "could not read from remote repository")) {
        
        if (contains(s, "does not appear")) {
            err.type    = ErrorType::RemoteNotFound;
            err.context = extract_after(s, "fatal: '");
        } else {
            err.type = ErrorType::FetchFailed;
        }
        return true;
    }
    if (contains(s, "no tracking information") ||
        contains(s, "no upstream configured") ||
        contains(s, "There is no tracking information")) {
        err.type = ErrorType::NoTrackingBranch;
        return true;
    }
    return false;
}

bool Detector::try_auth_errors(const std::string& s, GitError& err) const {
    if (contains(s, "Permission denied (publickey)") ||
        contains(s, "Permission denied (publickey,gssapi")) {
        err.type = ErrorType::PermissionDenied;
        return true;
    }
    if (contains(s, "Authentication failed") || contains(s, "authentication failed")) {
        err.type    = ErrorType::AuthFailed;
        err.context = extract_after(s, "Authentication failed for '");
        return true;
    }
    return false;
}

bool Detector::try_working_tree_errors(const std::string& s, GitError& err) const {
    if (contains(s, "untracked working tree files would be overwritten") ||
        contains(s, "untracked files would be overwritten")) {
        err.type           = ErrorType::UntrackedFileWouldBeOverwritten;
        err.affected_files = extract_files(s, "\t");
        return true;
    }
    if (contains(s, "local changes") &&
        (contains(s, "please commit") || contains(s, "would be overwritten"))) {
        err.type = ErrorType::UncommittedChanges;
        return true;
    }
    if (contains(s, "did not match any file") || contains(s, "pathspec did not match")) {
        err.type    = ErrorType::StagingFailed;
        err.context = extract_after(s, "pathspec '");
        return true;
    }
    return false;
}

bool Detector::try_stash_errors(const std::string& s, GitError& err) const {
    if (contains(s, "No stash entries found") || contains(s, "No stash found")) {
        err.type = ErrorType::StashEmpty;
        return true;
    }
    if (contains(s, "CONFLICT") && contains(s, "stash")) {
        err.type           = ErrorType::StashConflict;
        err.affected_files = extract_files(s, "CONFLICT (content): Merge conflict in ");
        return true;
    }
    return false;
}

bool Detector::try_rebase_merge_in_progress(const std::string& s, GitError& err) const {
    if (contains(s, "cannot rebase") && contains(s, "uncommitted changes")) {
        err.type = ErrorType::RebaseInProgress;
        return true;
    }
    if (contains(s, "rebase in progress") || contains(s, "You are currently rebasing")) {
        err.type = ErrorType::RebaseInProgress;
        return true;
    }
    if (contains(s, "merge in progress") || contains(s, "You have not concluded your merge")) {
        err.type = ErrorType::MergeInProgress;
        return true;
    }
    return false;
}

bool Detector::try_detached_head(const std::string& s, GitError& err) const {
    if (contains(s, "HEAD detached") || contains(s, "detached HEAD")) {
        err.type    = ErrorType::DetachedHead;
        err.context = extract_after(s, "HEAD detached at ");
        return true;
    }
    return false;
}

bool Detector::try_user_not_configured(const std::string& s, GitError& err) const {
    if (contains(s, "Please tell me who you are") ||
        contains(s, "user.email") || contains(s, "user.name")) {
        err.type = ErrorType::UserNotConfigured;
        return true;
    }
    return false;
}

bool Detector::try_commit_not_found(const std::string& s, GitError& err) const {
    if ((contains(s, "unknown revision") || contains(s, "ambiguous argument") ||
         contains(s, "bad revision")) && contains(s, "fatal")) {
        err.type    = ErrorType::CommitNotFound;
        err.context = extract_after(s, "ambiguous argument '");
        if (err.context.empty()) err.context = extract_after(s, "unknown revision '");
        return true;
    }
    return false;
}

bool Detector::try_nothing_to_commit(const std::string& s, GitError& err) const {
    if (contains(s, "nothing to commit") || contains(s, "working tree clean")) {
        err.type = ErrorType::NothingToCommit;
        return true;
    }
    return false;
}



std::vector<std::string> Detector::extract_files(const std::string& text,
                                                   const std::string& prefix) const {
    std::vector<std::string> files;
    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line)) {
        auto pos = line.find(prefix);
        if (pos != std::string::npos) {
            std::string fname = line.substr(pos + prefix.size());
            
            while (!fname.empty() && fname.front() == ' ') fname.erase(fname.begin());
            while (!fname.empty() && (fname.back() == ' ' || fname.back() == '\r')) fname.pop_back();
            if (!fname.empty()) files.push_back(fname);
        }
    }
    return files;
}

} 
