#include "executor.hpp"
#include <sstream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <initializer_list>

#ifdef _WIN32
  
  
  
  #define popen  _popen
  #define pclose _pclose
  #include <array>
#else
  #include <unistd.h>
  #include <sys/wait.h>
  #include <sys/ioctl.h>
  #include <termios.h>
  #include <poll.h>
  #include <cerrno>
  #include <csignal>
  #include <atomic>
  #if defined(__APPLE__) || defined(__FreeBSD__)
    #include <util.h>      
  #else
    #include <pty.h>       
  #endif
#endif

namespace git_explain {

#ifndef _WIN32
namespace {
    std::atomic<pid_t> g_child_pid{-1};

    void forward_signal_to_child(int sig) {
        pid_t pid = g_child_pid.load();
        if (pid > 0) {
            kill(pid, sig);
        }
    }

    void install_signal_forwarding() {
        struct sigaction sa{};
        sa.sa_handler = forward_signal_to_child;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT,  &sa, nullptr);
        sigaction(SIGTERM, &sa, nullptr);
        sigaction(SIGTSTP, &sa, nullptr);
    }

    void restore_default_signal_handling() {
        signal(SIGINT,  SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
    }

    
    void write_all(int fd, const char* buf, size_t len) {
        size_t off = 0;
        while (off < len) {
            ssize_t w = write(fd, buf + off, len - off);
            if (w > 0) {
                off += static_cast<size_t>(w);
            } else if (w < 0 && errno == EINTR) {
                continue;
            } else {
                break; 
            }
        }
    }
}
#endif

Executor::Executor(int timeout_seconds)
    : timeout_seconds_(timeout_seconds) {}

#ifndef _WIN32

ExecuteResult Executor::run(const std::string& command) const {
    ExecuteResult result;

    
    
    
    struct termios orig_termios{};
    bool stdin_is_tty = (tcgetattr(STDIN_FILENO, &orig_termios) == 0);

    struct winsize ws{};
    bool have_winsize = (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0);

    int master_fd = -1;
    pid_t pid = forkpty(&master_fd,
                         nullptr,
                         stdin_is_tty ? &orig_termios : nullptr,
                         have_winsize ? &ws : nullptr);

    if (pid < 0) {
        result.exit_code = -1;
        result.stderr_output = "Failed to allocate a pseudo-terminal for: " + command;
        return result;
    }

    if (pid == 0) {
        
        execlp("/bin/sh", "sh", "-c", command.c_str(), static_cast<char*>(nullptr));
        _exit(127); 
    }

    
    g_child_pid.store(pid);
    install_signal_forwarding();

    
    
    
    
    if (stdin_is_tty) {
        struct termios raw = orig_termios;
        cfmakeraw(&raw);
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    }

    std::string captured;
    char buf[4096];

    struct pollfd fds[2];
    fds[0].fd = master_fd;
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = stdin_is_tty ? POLLIN : 0;
    int nfds = stdin_is_tty ? 2 : 1;

    bool running = true;
    while (running) {
        int ready = poll(fds, static_cast<nfds_t>(nfds), -1);
        if (ready < 0) {
            if (errno == EINTR) continue;
            break;
        }

        
        if (fds[0].revents & (POLLIN | POLLHUP | POLLERR)) {
            ssize_t n = read(master_fd, buf, sizeof(buf));
            if (n > 0) {
                std::fwrite(buf, 1, static_cast<size_t>(n), stdout);
                std::fflush(stdout);
                captured.append(buf, static_cast<size_t>(n));
            } else {
                running = false; 
            }
        }

        
        if (stdin_is_tty && (fds[1].revents & POLLIN)) {
            ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));
            if (n > 0) {
                write_all(master_fd, buf, static_cast<size_t>(n));
            } else {
                
                
                fds[1].events = 0;
            }
        }
    }

    close(master_fd);

    if (stdin_is_tty) {
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    }

    int status = 0;
    waitpid(pid, &status, 0);

    restore_default_signal_handling();
    g_child_pid.store(-1);

    if (WIFEXITED(status)) {
        result.exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        result.exit_code = 128 + WTERMSIG(status);
    } else {
        result.exit_code = -1;
    }

    result.stderr_output = captured;

    return result;
}

#else 

ExecuteResult Executor::run(const std::string& command) const {
    ExecuteResult result;

    {
        std::string cmd_stdout = command + " 2>/dev/null";
        FILE* pipe = popen(cmd_stdout.c_str(), "r");
        if (!pipe) {
            result.exit_code = -1;
            result.stderr_output = "Failed to open pipe for command: " + command;
            return result;
        }
        std::array<char, 4096> buf{};
        while (fgets(buf.data(), buf.size(), pipe)) {
            result.stdout_output += buf.data();
        }
        pclose(pipe);
    }
    {
        std::string cmd_stderr = command + " 2>&1 >/dev/null";
        FILE* pipe = popen(cmd_stderr.c_str(), "r");
        if (!pipe) {
            result.exit_code = -1;
            return result;
        }
        std::array<char, 4096> buf{};
        while (fgets(buf.data(), buf.size(), pipe)) {
            result.stderr_output += buf.data();
        }
        result.exit_code = pclose(pipe);
    }

    return result;
}

#endif

ExecuteResult Executor::git(const std::vector<std::string>& args) const {
    return run(build_command(args));
}

namespace {
    bool has_any(const std::vector<std::string>& args,
                 std::initializer_list<const char*> flags) {
        for (const auto& a : args) {
            for (const char* f : flags) {
                if (a == f) return true;
                
                std::string prefix = std::string(f) + "=";
                if (a.rfind(prefix, 0) == 0) return true;
            }
        }
        return false;
    }
}

bool Executor::needs_passthrough(const std::vector<std::string>& args) {
    if (args.empty()) return false;
    const std::string& sub = args[0];

    if (sub == "commit") {
        bool has_message      = has_any(args, {"-m", "--message", "-F", "--file",
                                                 "-C", "--reuse-message"});
        bool no_edit          = has_any(args, {"--no-edit", "--fixup", "--squash",
                                                 "--allow-empty-message"});
        
        
        if (has_message || no_edit) return false;
        return true;
    }

    if (sub == "rebase") {
        return has_any(args, {"-i", "--interactive"});
    }

    if (sub == "add") {
        return has_any(args, {"-p", "--patch", "-i", "--interactive"});
    }

    if (sub == "tag") {
        bool annotated = has_any(args, {"-a", "--annotate", "-s", "--sign"});
        bool has_message = has_any(args, {"-m", "--message", "-F", "--file"});
        return annotated && !has_message;
    }

    return false;
}

ExecuteResult Executor::run_passthrough(const std::vector<std::string>& args) const {
    ExecuteResult result;

#ifdef _WIN32
    
    
    std::string cmd = "git";
    for (const auto& a : args) cmd += " " + a; 
    result.exit_code = std::system(cmd.c_str());
    return result;
#else
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>("git"));
    for (const auto& a : args) argv.push_back(const_cast<char*>(a.c_str()));
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid < 0) {
        result.exit_code = -1;
        return result;
    }
    if (pid == 0) {
        
        
        
        execvp("git", argv.data());
        _exit(127);
    }

    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        result.exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        result.exit_code = 128 + WTERMSIG(status);
    } else {
        result.exit_code = -1;
    }
    return result;
#endif
}

std::string Executor::build_command(const std::vector<std::string>& args) {
    std::ostringstream oss;
    oss << "git";
    for (const auto& a : args) {
        oss << " '";
        for (char c : a) {
            if (c == '\'') oss << "'\\''";
            else           oss << c;
        }
        oss << "'";
    }
    return oss.str();
}

} 
