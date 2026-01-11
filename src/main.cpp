#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>
#include <array>
#include <cstdio>
#include <sys/wait.h>

/* ---------------- Utility ---------------- */

std::string cleanHistoryLine(const std::string& line) {
    auto pos = line.find(';');
    if (pos != std::string::npos)
        return line.substr(pos + 1);
    return line;
}

bool isGitCommand(const std::string& line) {
    return line.rfind("git ", 0) == 0;
}

/* ---------------- History ---------------- */

std::string getHistoryPath(const std::string& shell) {
    const char* home = std::getenv("HOME");
    if (!home) return "";

    if (shell.find("zsh") != std::string::npos)
        return std::string(home) + "/.zsh_history";

    if (shell.find("bash") != std::string::npos)
        return std::string(home) + "/.bash_history";

    return "";
}

std::vector<std::string> readHistory(const std::string& path) {
    std::ifstream file(path);
    std::vector<std::string> lines;

    if (!file.is_open()) return lines;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        lines.push_back(cleanHistoryLine(line));
    }
    return lines;
}

std::string findLastGitCommand(const std::vector<std::string>& history) {
    for (size_t i = history.size(); i-- > 0;) {
        if (isGitCommand(history[i])) {
            return history[i];
        }
    }
    return "";
}

void flushShellHistory(const char* shell) {
    if (!shell) return;

    std::string s(shell);
    if (s.find("zsh") != std::string::npos) {
        system("zsh -c 'fc -W'");
    } else if (s.find("bash") != std::string::npos) {
        system("bash -c 'history -w'");
    }
}

/* ---------------- Command Execution ---------------- */

std::string runCommand(const std::string& command, int& exitCode) {
    std::array<char, 256> buffer{};
    std::string result;
    std::string cmd = command + " 2>&1";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        exitCode = -1;
        return "Failed to execute command";
    }

    while (fgets(buffer.data(), buffer.size(), pipe)) {
        result += buffer.data();
    }

    int status = pclose(pipe);
    if (WIFEXITED(status))
        exitCode = WEXITSTATUS(status);
    else
        exitCode = -1;

    return result;
}
std::string explainPushError(const std::string& error) {

    if (error.find("src refspec") != std::string::npos &&
        error.find("does not match any") != std::string::npos) {

        return
        "You are trying to push a branch that does not exist locally.\n\n"
        "Why this happens:\n"
        "• You haven't created the branch yet\n"
        "• You made a typo in the branch name\n"
        "• You haven't made your first commit\n\n"
        "How to fix it:\n"
        "→ Check your branch: git branch\n"
        "→ Create one if needed: git checkout -b main\n"
        "→ Make a commit, then push again\n";
    }

    if (error.find("failed to push some refs") != std::string::npos) {
        return
        "Your local branch is behind the remote branch.\n\n"
        "Why this happens:\n"
        "• Someone else pushed changes\n"
        "• The remote branch has new commits\n\n"
        "How to fix it:\n"
        "→ git pull --rebase\n"
        "→ Resolve conflicts if prompted\n"
        "→ git push\n";
    }

    if (error.find("permission denied") != std::string::npos ||
        error.find("publickey") != std::string::npos) {

        return
        "Git could not authenticate you with the remote repository.\n\n"
        "Why this happens:\n"
        "• SSH key is missing or not added to GitHub\n"
        "• Wrong GitHub account\n\n"
        "How to fix it:\n"
        "→ Check SSH: ssh -T git@github.com\n"
        "→ Add SSH key to GitHub\n";
    }

    if (error.find("repository not found") != std::string::npos) {
        return
        "The remote repository does not exist or you don't have access.\n\n"
        "Why this happens:\n"
        "• Wrong repository URL\n"
        "• Repository was deleted or made private\n\n"
        "How to fix it:\n"
        "→ Check remote: git remote -v\n"
        "→ Verify repository URL on GitHub\n";
    }

    return "";
}

/* ---------------- main ---------------- */

int main() {
    const char* shell = std::getenv("SHELL");
    flushShellHistory(shell);

    if (!shell) {
        std::cerr << "❌ Could not detect shell\n";
        return EXIT_FAILURE;
    }

    std::string historyPath = getHistoryPath(shell);
    if (historyPath.empty()) {
        std::cerr << "❌ Unsupported shell\n";
        return EXIT_FAILURE;
    }

    std::cout << "History file: " << historyPath << "\n";

    auto history = readHistory(historyPath);
    if (history.empty()) {
        std::cerr << "❌ Could not read history file\n";
        return EXIT_FAILURE;
    }

    std::cout << "History loaded, total commands = "
              << history.size() << "\n";

    std::string lastGitCmd = findLastGitCommand(history);
    if (lastGitCmd.empty()) {
        std::cerr << "❌ No recent git command found\n";
        return EXIT_FAILURE;
    }

    std::cout << "Last git command: " << lastGitCmd << "\n";

    int exitCode = 0;
    std::string output = runCommand(lastGitCmd, exitCode);

    if (exitCode == 0) {
        std::cout << "✅ Git command succeeded. Nothing to explain.\n";
        return EXIT_SUCCESS;
    }

    std::cout << "\n❌ Git command failed\n\n";
    if (lastGitCmd.find("git push") == 0) {
    std::string explanation = explainPushError(output);

    if (!explanation.empty()) {
        std::cout << explanation;
        return EXIT_SUCCESS;
    }
}

std::cout << "Raw Git error:\n";
std::cout << output << "\n";


    return EXIT_SUCCESS;
}
