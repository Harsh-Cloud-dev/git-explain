#include "test_framework.hpp"
#include "detection/detector.hpp"

using namespace git_explain;

static Detector det;

TEST(detect_unknown_command) {
    auto e = det.detect_from_stderr("git: 'comit' is not a git command. See 'git --help'.");
    ASSERT_EQ(e.type, ErrorType::UnknownCommand);
    ASSERT_EQ(e.context, "comit");
}

TEST(detect_not_a_repo) {
    auto e = det.detect_from_stderr("fatal: not a git repository (or any of the parent directories): .git");
    ASSERT_EQ(e.type, ErrorType::NotAGitRepo);
}

TEST(detect_merge_conflict) {
    std::string stderr_text =
        "Auto-merging src/foo.cpp\n"
        "CONFLICT (content): Merge conflict in src/foo.cpp\n"
        "Automatic merge failed; fix conflicts and then commit the result.\n";
    auto e = det.detect_from_stderr(stderr_text);
    ASSERT_EQ(e.type, ErrorType::MergeConflict);
    ASSERT_TRUE(e.affected_files.size() == 1);
    ASSERT_EQ(e.affected_files[0], "src/foo.cpp");
}

TEST(detect_push_rejected) {
    auto e = det.detect_from_stderr(
        "To github.com:user/repo.git\n"
        " ! [rejected]        main -> main (non-fast-forward)\n"
        "error: failed to push some refs to 'github.com:user/repo.git'\n");
    ASSERT_EQ(e.type, ErrorType::PushRejected);
}

TEST(detect_branch_already_exists) {
    auto e = det.detect_from_stderr("fatal: a branch named 'feature/auth' already exists.");
    ASSERT_EQ(e.type, ErrorType::BranchAlreadyExists);
}

TEST(detect_detached_head) {
    auto e = det.detect_from_stderr("Note: switching to 'abc1234'.\nYou are in 'detached HEAD' state.\nHEAD detached at abc1234\n");
    ASSERT_EQ(e.type, ErrorType::DetachedHead);
}

TEST(detect_no_tracking_branch) {
    auto e = det.detect_from_stderr(
        "There is no tracking information for the current branch.\n"
        "Please specify which branch you want to merge with.\n");
    ASSERT_EQ(e.type, ErrorType::NoTrackingBranch);
}

TEST(detect_user_not_configured) {
    auto e = det.detect_from_stderr(
        "*** Please tell me who you are.\n\n"
        "Run\n\n  git config --global user.email \"you@example.com\"\n");
    ASSERT_EQ(e.type, ErrorType::UserNotConfigured);
}

TEST(detect_permission_denied) {
    auto e = det.detect_from_stderr("git@github.com: Permission denied (publickey).\nfatal: Could not read from remote repository.");
    ASSERT_EQ(e.type, ErrorType::PermissionDenied);
}

TEST(detect_nothing_to_commit) {
    auto e = det.detect_from_stderr("", 1);
    
    ExecuteResult r;
    r.stdout_output = "On branch main\nnothing to commit, working tree clean\n";
    r.exit_code = 1;
    auto e2 = det.detect(r);
    ASSERT_EQ(e2.type, ErrorType::NothingToCommit);
}

TEST(detect_unknown_fallback) {
    auto e = det.detect_from_stderr("something totally unrecognised zxqy");
    ASSERT_EQ(e.type, ErrorType::Unknown);
}

TEST(detect_stash_empty) {
    auto e = det.detect_from_stderr("No stash entries found.");
    ASSERT_EQ(e.type, ErrorType::StashEmpty);
}

TEST(detect_merge_in_progress) {
    auto e = det.detect_from_stderr(
        "error: merging is not possible because you have unmerged files.\n"
        "hint: Fix them up in the work tree, and then use 'git add/rm <file>'\n");
    ASSERT_EQ(e.type, ErrorType::MergeInProgress);
}
