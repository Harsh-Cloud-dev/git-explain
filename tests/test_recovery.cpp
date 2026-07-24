#include "test_framework.hpp"
#include "recovery/recovery.hpp"

using namespace git_explain;

static Recovery rec;

static GitError make_error(ErrorType t, const std::string& ctx = "",
                           const std::vector<std::string>& files = {}) {
    GitError e;
    e.type           = t;
    e.context        = ctx;
    e.affected_files = files;
    return e;
}

TEST(recovery_not_empty_for_all_known_types) {
    
    const std::vector<ErrorType> types = {
        ErrorType::UnknownCommand, ErrorType::NotAGitRepo,
        ErrorType::MergeConflict,  ErrorType::PushRejected,
        ErrorType::BranchAlreadyExists, ErrorType::BranchNotFound,
        ErrorType::DetachedHead,   ErrorType::UncommittedChanges,
        ErrorType::NoTrackingBranch, ErrorType::RemoteNotFound,
        ErrorType::AuthFailed,     ErrorType::PermissionDenied,
        ErrorType::StashEmpty,     ErrorType::RebaseInProgress,
        ErrorType::MergeInProgress, ErrorType::UserNotConfigured,
        ErrorType::NothingToCommit, ErrorType::Unknown,
    };
    for (auto t : types) {
        auto s = rec.suggest(make_error(t));
        ASSERT_TRUE(!s.cause.empty() || !s.steps.empty());
    }
}

TEST(recovery_merge_conflict_lists_files) {
    GitError e = make_error(ErrorType::MergeConflict, "", {"src/main.cpp", "README.md"});
    auto s = rec.suggest(e);
    
    bool found_add = false;
    for (const auto& step : s.steps) {
        if (step.command.find("git add") != std::string::npos) found_add = true;
    }
    ASSERT_TRUE(found_add);
}

TEST(recovery_push_rejected_has_pull_rebase) {
    auto s = rec.suggest(make_error(ErrorType::PushRejected));
    bool found = false;
    for (const auto& step : s.steps) {
        if (step.command.find("--rebase") != std::string::npos) found = true;
    }
    ASSERT_TRUE(found);
}

TEST(recovery_branch_already_exists_uses_context) {
    auto s = rec.suggest(make_error(ErrorType::BranchAlreadyExists, "feature/login"));
    ASSERT_TRUE(s.cause.find("feature/login") != std::string::npos);
    bool found_checkout = false;
    for (const auto& step : s.steps) {
        if (step.command.find("feature/login") != std::string::npos) found_checkout = true;
    }
    ASSERT_TRUE(found_checkout);
}

TEST(recovery_user_not_configured_no_autofix) {
    auto s = rec.suggest(make_error(ErrorType::UserNotConfigured));
    
    ASSERT_TRUE(!s.steps.empty());
    for (const auto& step : s.steps) {
        ASSERT_FALSE(step.is_dangerous);
    }
}

TEST(recovery_force_push_is_dangerous) {
    auto s = rec.suggest(make_error(ErrorType::PushRejected));
    bool found_dangerous = false;
    for (const auto& step : s.steps) {
        if (step.is_dangerous) found_dangerous = true;
    }
    ASSERT_TRUE(found_dangerous);
}
