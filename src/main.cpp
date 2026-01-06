#include<iostream>
#include<cstdlib>
#include<string>
#include<fstream>
#include<vector>
#include<array>
#include<cstdio>
bool isGitCommand(const std::string_view line){
    return line.rfind("git ",0) == 0;
}
std::string findLastGitCommand(const std::vector<std::string>& history){
    for (size_t i = history.size(); i-- > 0; ) {
        if (isGitCommand(history[i])) {
            return history[i];
        }
    }
    return "";
}
std::string getHistoryPath(const std::string& shell){
    const char* home = std::getenv("HOME");
    if(!home) return "";
    if(shell.find("zsh") != std::string::npos)
        return std::string(home) + "/.zsh_history";
    if (shell.find("bash") != std::string::npos)
        return std::string(home) + "/.bash_history";
    return "";
}
std::vector<std::string> readHistory(const std::string& path){
    std::ifstream file(path);
    std::vector<std::string> lines;
    if (!file.is_open()) return lines;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        auto pos = line.find(';');
        if (pos != std::string::npos) {
            lines.push_back(line.substr(pos + 1));
        } else {
            lines.push_back(line);
        }
    }
    return lines;
}
void flushShellHistory(const char* shell) {
    if (!shell) return;
    std::string shellStr(shell);
    if (shellStr.find("zsh") != std::string::npos) {
        system("zsh -c 'fc -W'");
    } 
    else if (shellStr.find("bash") != std::string::npos) {
        system("bash -c 'history -w'");
    }
}

int main(){
    const char* shell = std::getenv("SHELL");
    flushShellHistory(shell);
    std::string historyPath = getHistoryPath(shell);
    if (historyPath.empty()){
        std::cerr << "Unsupported shell\n";
        return EXIT_FAILURE;
    }
    std::cout<<"History file: "<< historyPath << "\n";
    auto history = readHistory(historyPath);
    if(history.empty()){
        std::cerr << "Could not read history file\n";
        return EXIT_FAILURE;
    }
    std::cout<<"History loaded, total commands... = "<<history.size()<<std::endl;
    std::string lastGitCmd = findLastGitCommand(history);
    if (lastGitCmd.empty()){
        std::cerr << "No recent git command found.";
        return EXIT_FAILURE;
    }
    std::cout<<"Last git command found : "<<lastGitCmd<<std::endl;
    return EXIT_SUCCESS;
}