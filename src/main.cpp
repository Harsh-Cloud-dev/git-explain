#include<iostream>
#include<cstdlib>
#include<string>
#include<fstream>
#include<vector>
std::string getHistoryPath(const std::string& shell){
    const char* home = std::getenv("HOME");
    if(!home) return "";
    if(shell.find("zsh") != std::string::npos)
        return std::string(home) + "/.zsh_history";
    if (shell.find("bash") != std::string::npos)
        return std::string(home) + "/.bash_history";
    return "";
}
std::vector<std::string>
readHistory(const std::string& path){
    std::ifstream file(path);
    std::vector<std::string> lines;
    if(!file.is_open()){
        return lines;
    }
    std::string line;
    while(std::getline(file,line)){
        if(!line.empty())
            lines.push_back(line);
    }
    return lines;
}
int main(){
    const char* shell = std::getenv("SHELL");
    std::string historyPath = getHistoryPath(shell);
    if (historyPath.empty()){
        std::cerr << "Unsupported shell\n";
        return 0;
    }
    std::cout<<"History file: "<< historyPath << "\n";
    auto history = readHistory(historyPath);
    if(history.empty()){
        std::cerr << "Could not read history file\n";
        return 1;
    }
    std::cout<<"History loaded, total commands... = "<<history.size()<<std::endl;
    return 0;
}