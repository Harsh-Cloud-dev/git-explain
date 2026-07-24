#include "recovery.hpp"

namespace git_explain {

RecoverySuggestion Recovery::suggest(const GitError& error) const {
    switch (error.type) {
        case ErrorType::UnknownCommand:
        case ErrorType::AmbiguousCommand:
        case ErrorType::InvalidFlag:
            return for_unknown_command(error);
        case ErrorType::NotAGitRepo:
            return for_not_a_repo(error);
        case ErrorType::MergeConflict:
        case ErrorType::CherryPickConflict:
        case ErrorType::StashConflict:
            return for_merge_conflict(error);
        case ErrorType::MergeInProgress:
            return for_merge_in_progress(error);
        case ErrorType::RebaseInProgress:
            return for_rebase_in_progress(error);
        case ErrorType::PushRejected:
            return for_push_rejected(error);
        case ErrorType::BranchAlreadyExists:
            return for_branch_already_exists(error);
        case ErrorType::BranchNotFound:
            return for_branch_not_found(error);
        case ErrorType::CannotDeleteCurrent:
        {
            RecoverySuggestion s;
            s.cause       = "You can't delete the branch you're currently on.";
            s.explanation = "Git refuses to delete the currently checked-out branch "
                            "because that would leave your working tree in an inconsistent state. "
                            "Switch to a different branch first, then delete the target branch.";
            s.steps = {
                {"Switch to another branch (e.g. main)", "git checkout main"},
                {"Now delete the branch", "git branch -d <branch-name>"},
            };
            return s;
        }
        case ErrorType::DetachedHead:
            return for_detached_head(error);
        case ErrorType::UncommittedChanges:
            return for_uncommitted_changes(error);
        case ErrorType::UntrackedFileWouldBeOverwritten:
            return for_untracked_overwrite(error);
        case ErrorType::StagingFailed:
        {
            RecoverySuggestion s;
            s.cause       = "The path you gave git does not match any tracked or staged file.";
            s.explanation = "Git couldn't find '" + error.context + "' in the working tree. "
                            "Check for typos, make sure you're in the right directory, or use "
                            "`git status` to see which files are actually changed.";
            s.steps = {
                {"List changed files to find the correct path", "git status"},
                {"Stage all changed files if that's your intent", "git add ."},
            };
            return s;
        }
        case ErrorType::NothingToCommit:
            return for_nothing_to_commit(error);
        case ErrorType::RemoteNotFound:
            return for_remote_not_found(error);
        case ErrorType::FetchFailed:
        {
            RecoverySuggestion s;
            s.cause       = "Git could not reach the remote repository.";
            s.explanation = "The connection to the remote failed. This is usually a network "
                            "issue, a wrong URL, or the remote server is down.";
            s.steps = {
                {"Verify the remote URL", "git remote -v"},
                {"Test basic connectivity", "ping github.com"},
                {"If the URL is wrong, fix it", "git remote set-url origin <correct-url>"},
            };
            return s;
        }
        case ErrorType::NoTrackingBranch:
            return for_no_tracking_branch(error);
        case ErrorType::PermissionDenied:
            return for_permission_denied(error);
        case ErrorType::AuthFailed:
            return for_auth_failed(error);
        case ErrorType::StashEmpty:
            return for_stash_empty(error);
        case ErrorType::EmptyRepository:
        {
            RecoverySuggestion s;
            s.cause       = "The repository has no commits yet.";
            s.explanation = "You've initialised a git repo but haven't made any commits. "
                            "Some operations (like push or log) require at least one commit.";
            s.steps = {
                {"Create an initial commit", "git commit --allow-empty -m \"Initial commit\""},
                {"Or add a file and commit it", "git add README.md && git commit -m \"Initial commit\""},
            };
            return s;
        }
        case ErrorType::UserNotConfigured:
            return for_user_not_configured(error);
        case ErrorType::CommitNotFound:
        {
            RecoverySuggestion s;
            s.cause       = "Git can't find the commit or ref '" + error.context + "'.";
            s.explanation = "The SHA, branch name, or tag you referenced doesn't exist in this "
                            "repository. You may have a typo, or the object exists only on the remote.";
            s.steps = {
                {"List local branches", "git branch"},
                {"List all tags", "git tag"},
                {"Fetch from remote to get missing objects", "git fetch --all"},
                {"Search the reflog for lost commits", "git reflog"},
            };
            return s;
        }
        case ErrorType::TagAlreadyExists:
        {
            RecoverySuggestion s;
            s.cause       = "A tag with that name already exists.";
            s.explanation = "Git tags are unique. If you want to move the tag to a different "
                            "commit, you must delete it first (or force-update it).";
            s.steps = {
                {"Delete the existing tag locally", "git tag -d <tag-name>"},
                {"Recreate it at the right commit", "git tag <tag-name> <commit>"},
                {"Force-push to update the remote tag (careful!)", "git push origin :refs/tags/<tag-name> && git push origin <tag-name>", true},
            };
            return s;
        }
        default:
            return for_unknown(error);
    }
}



RecoverySuggestion Recovery::for_unknown_command(const GitError& e) const {
    RecoverySuggestion s;
    if (e.type == ErrorType::UnknownCommand) {
        s.cause = "'" + e.context + "' is not a valid git sub-command.";
        s.explanation = "Git doesn't recognise the command you typed. "
                        "Check for typos — common ones include 'comit' for 'commit', "
                        "'chekout' for 'checkout', 'statsu' for 'status'. "
                        "Run `git help` to see all available commands.";
    } else if (e.type == ErrorType::InvalidFlag) {
        s.cause       = "One of the flags/options you passed is not valid for this command.";
        s.explanation = "You used an option that the git sub-command doesn't support. "
                        "Run `git <command> --help` to see which flags are accepted.";
    } else {
        s.cause       = "Git found multiple commands that match your input.";
        s.explanation = "Your input is ambiguous. Git showed you a list of candidates above. "
                        "Type the full command name to disambiguate.";
    }
    s.steps = {
        {"See all available git commands", "git help --all"},
        {"Get help for a specific command", "git <command> --help"},
    };
    return s;
}

RecoverySuggestion Recovery::for_not_a_repo(const GitError& ) const {
    RecoverySuggestion s;
    s.cause       = "The current directory is not inside a git repository.";
    s.explanation = "Git commands only work inside a directory that has been initialised as a "
                    "repository (one that contains a .git folder). Either navigate into an "
                    "existing repo, or initialise a new one here.";
    s.steps = {
        {"Check your current directory", "pwd"},
        {"Navigate into your project", "cd /path/to/your/project"},
        {"Or initialise a new repository here", "git init"},
        {"Or clone an existing repository", "git clone <url>"},
    };
    return s;
}

RecoverySuggestion Recovery::for_merge_conflict(const GitError& e) const {
    RecoverySuggestion s;
    s.cause       = "Git could not automatically merge all changes.";
    s.explanation = "A merge conflict occurs when the same lines were changed differently in "
                    "the two branches being merged. Git has marked the conflicting sections "
                    "inside the affected files with <<<<<<, =======, and >>>>>>> markers. "
                    "You must edit each file to choose the correct content, then stage it.";

    if (!e.affected_files.empty()) {
        s.steps.push_back({"Open and resolve each conflicted file:", ""});
        for (const auto& f : e.affected_files) {
            s.steps.push_back({"  Edit: " + f, "git diff " + f});
        }
    } else {
        s.steps.push_back({"Find all conflicted files", "git diff --name-only --diff-filter=U"});
    }
    s.steps.push_back({"After editing, mark each file as resolved", "git add <file>"});
    s.steps.push_back({"Complete the merge", "git commit"});
    s.steps.push_back({"Or abort the merge entirely", "git merge --abort", true});
    return s;
}

RecoverySuggestion Recovery::for_merge_in_progress(const GitError& ) const {
    RecoverySuggestion s;
    s.cause       = "A merge is in progress with unresolved conflicts.";
    s.explanation = "You started a merge but left it unfinished. Resolve the conflicting "
                    "files, stage them, then complete the merge with `git commit`, "
                    "or abort with `git merge --abort`.";
    s.steps = {
        {"See which files still have conflicts", "git status"},
        {"After resolving, stage the files", "git add <file>"},
        {"Complete the merge", "git commit"},
        {"Or abort and go back to the state before the merge", "git merge --abort", true},
    };
    return s;
}

RecoverySuggestion Recovery::for_rebase_in_progress(const GitError& ) const {
    RecoverySuggestion s;
    s.cause       = "A rebase is in progress or your working tree has uncommitted changes.";
    s.explanation = "If a rebase is ongoing, resolve any conflicts and continue, "
                    "or abort to return to the pre-rebase state. "
                    "If you have uncommitted changes that are blocking a new rebase, "
                    "stash or commit them first.";
    s.steps = {
        {"Check the repository state", "git status"},
        {"After resolving conflicts, continue the rebase", "git rebase --continue"},
        {"Skip the problematic commit", "git rebase --skip"},
        {"Abort the rebase entirely", "git rebase --abort", true},
        {"Stash changes before rebasing", "git stash"},
    };
    return s;
}

RecoverySuggestion Recovery::for_push_rejected(const GitError& ) const {
    RecoverySuggestion s;
    s.cause       = "Your local branch is behind the remote branch.";
    s.explanation = "Someone else pushed commits to the remote since you last pulled. "
                    "Git will not overwrite those commits with a plain push. "
                    "Pull the remote changes first, resolve any conflicts, then push again. "
                    "Force-pushing is an option only if you are certain the remote commits "
                    "can be discarded (e.g. you own the branch and no one else uses it).";
    s.steps = {
        {"Pull the latest changes and rebase your work on top", "git pull --rebase"},
        {"Then push again", "git push"},
        {"Force-push if you own the branch and want to overwrite remote (DANGEROUS)", "git push --force-with-lease", true},
    };
    return s;
}

RecoverySuggestion Recovery::for_branch_already_exists(const GitError& e) const {
    RecoverySuggestion s;
    s.cause       = "A branch named '" + e.context + "' already exists.";
    s.explanation = "You tried to create a branch that already exists. "
                    "Either switch to the existing branch, delete it first, "
                    "or choose a different name.";
    s.steps = {
        {"Switch to the existing branch", "git checkout " + e.context},
        {"Delete the branch and recreate it", "git branch -D " + e.context, true},
        {"Or just pick a new branch name", "git checkout -b <new-name>"},
    };
    return s;
}

RecoverySuggestion Recovery::for_branch_not_found(const GitError& e) const {
    RecoverySuggestion s;
    s.cause       = "No branch named '" + e.context + "' was found.";
    s.explanation = "The branch you referenced does not exist locally. "
                    "It might exist on the remote but not been fetched yet, "
                    "or you may have a typo.";
    s.steps = {
        {"List local branches", "git branch"},
        {"List remote branches", "git branch -r"},
        {"Fetch from remote to get new branches", "git fetch"},
        {"Checkout a remote branch locally", "git checkout -b " + e.context + " origin/" + e.context},
    };
    return s;
}

RecoverySuggestion Recovery::for_detached_head(const GitError& e) const {
    RecoverySuggestion s;
    std::string ref = e.context.empty() ? "a commit" : e.context;
    s.cause       = "HEAD is pointing directly at " + ref + " rather than a branch.";
    s.explanation = "In detached HEAD state, any commits you make won't belong to any branch "
                    "and could be lost once you switch away. "
                    "If you just want to inspect things, that's fine. "
                    "If you want to make commits, create a new branch first.";
    s.steps = {
        {"Create a new branch here to keep your work", "git checkout -b <new-branch-name>"},
        {"Or return to your previous branch", "git checkout -"},
        {"Or return to main/master", "git checkout main"},
    };
    return s;
}

RecoverySuggestion Recovery::for_uncommitted_changes(const GitError& ) const {
    RecoverySuggestion s;
    s.cause       = "You have uncommitted changes that would be overwritten.";
    s.explanation = "The operation you tried to run (e.g. checkout, pull, merge) would "
                    "overwrite your local modifications. Git refuses to do this by default "
                    "to protect your work. Commit or stash your changes first.";
    s.steps = {
        {"See which files are changed", "git status"},
        {"Stash your changes temporarily", "git stash"},
        {"Or commit them", "git add . && git commit -m \"WIP\""},
        {"After the operation, restore your stash", "git stash pop"},
    };
    return s;
}

RecoverySuggestion Recovery::for_untracked_overwrite(const GitError& e) const {
    RecoverySuggestion s;
    s.cause       = "Untracked files in your working tree would be overwritten.";
    s.explanation = "The files listed above exist in your working tree but are not tracked by "
                    "git. The operation you tried would replace them. "
                    "Back them up, remove them, or add them to .gitignore before continuing.";
    s.steps = {};
    if (!e.affected_files.empty()) {
        for (const auto& f : e.affected_files) {
            s.steps.push_back({"Back up or delete: " + f, "rm " + f, true});
        }
    }
    s.steps.push_back({"Clean untracked files (dry-run first)", "git clean -n"});
    s.steps.push_back({"Actually remove untracked files", "git clean -f", true});
    return s;
}

RecoverySuggestion Recovery::for_no_tracking_branch(const GitError& ) const {
    RecoverySuggestion s;
    s.cause       = "The current branch has no upstream (tracking) branch set.";
    s.explanation = "Git doesn't know which remote branch to push to or pull from. "
                    "Set an upstream with `--set-upstream` the first time you push.";
    s.steps = {
        {"Push and set the upstream in one go", "git push --set-upstream origin <branch-name>"},
        {"Or set the upstream separately", "git branch --set-upstream-to=origin/<branch-name>"},
    };
    return s;
}

RecoverySuggestion Recovery::for_remote_not_found(const GitError& e) const {
    RecoverySuggestion s;
    std::string remote = e.context.empty() ? "origin" : e.context;
    s.cause       = "Remote '" + remote + "' does not exist or is misconfigured.";
    s.explanation = "The remote name you used isn't defined in this repo. "
                    "Check your remotes and add or fix the URL.";
    s.steps = {
        {"List all configured remotes", "git remote -v"},
        {"Add a remote", "git remote add " + remote + " <url>"},
        {"Fix an existing remote's URL", "git remote set-url " + remote + " <url>"},
    };
    return s;
}

RecoverySuggestion Recovery::for_auth_failed(const GitError& e) const {
    RecoverySuggestion s;
    s.cause       = "Authentication failed for remote: " + e.context;
    s.explanation = "Your credentials were rejected by the remote server. "
                    "This could mean your password has changed, your personal access "
                    "token has expired, or you're using the wrong credentials.";
    s.steps = {
        {"Clear cached credentials (macOS keychain)", "git credential-osxkeychain erase"},
        {"Set a new remote URL with token embedded (HTTPS)", "git remote set-url origin https://<token>@github.com/<user>/<repo>.git"},
        {"Or switch to SSH-based authentication", "git remote set-url origin git@github.com:<user>/<repo>.git"},
    };
    return s;
}

RecoverySuggestion Recovery::for_permission_denied(const GitError& ) const {
    RecoverySuggestion s;
    s.cause       = "SSH key authentication failed.";
    s.explanation = "The server rejected your SSH key. Either you don't have an SSH key "
                    "added to your account, the key isn't loaded in the SSH agent, or "
                    "the key file has wrong permissions.";
    s.steps = {
        {"Check which keys are loaded in the SSH agent", "ssh-add -l"},
        {"Generate a new SSH key if you don't have one", "ssh-keygen -t ed25519 -C \"your@email.com\""},
        {"Add your key to the SSH agent", "ssh-add ~/.ssh/id_ed25519"},
        {"Test the SSH connection to GitHub", "ssh -T git@github.com"},
    };
    return s;
}

RecoverySuggestion Recovery::for_stash_empty(const GitError& ) const {
    RecoverySuggestion s;
    s.cause       = "The stash is empty — there are no stashed changes to apply.";
    s.explanation = "You tried to pop, apply, or inspect a stash entry, but there are none. "
                    "Your changes might already be applied, or they were never stashed.";
    s.steps = {
        {"Verify the stash is really empty", "git stash list"},
        {"Check your current working tree for changes", "git status"},
    };
    return s;
}

RecoverySuggestion Recovery::for_user_not_configured(const GitError& ) const {
    RecoverySuggestion s;
    s.cause       = "Git doesn't know your name and email address.";
    s.explanation = "Every git commit records the author's name and email. "
                    "You need to configure these before you can commit. "
                    "Use --global to set them for all repositories on this machine.";
    s.steps = {
        {"Set your name globally", "git config --global user.name \"Your Name\""},
        {"Set your email globally", "git config --global user.email \"you@example.com\""},
    };
    s.has_autofix = false;
    return s;
}

RecoverySuggestion Recovery::for_nothing_to_commit(const GitError& ) const {
    RecoverySuggestion s;
    s.cause       = "There are no staged changes to commit.";
    s.explanation = "Your working tree is clean — no files have been modified, or "
                    "changed files haven't been staged yet. "
                    "Use `git status` to check, and `git add` to stage files.";
    s.steps = {
        {"See the current state", "git status"},
        {"Stage all changes", "git add ."},
        {"Or stage specific files", "git add <file>"},
    };
    return s;
}

RecoverySuggestion Recovery::for_unknown(const GitError& e) const {
    RecoverySuggestion s;
    s.cause       = "An unrecognised git error occurred (exit code " +
                    std::to_string(e.exit_code) + ").";
    s.explanation = "git-explain couldn't automatically classify this error. "
                    "The raw git output is shown above. Some things to try:";
    s.steps = {
        {"Check the repository state", "git status"},
        {"Look at the recent log", "git log --oneline -10"},
        {"Check the reflog for lost commits", "git reflog"},
        {"Search the git documentation", "git help <command>"},
    };
    return s;
}

} 
