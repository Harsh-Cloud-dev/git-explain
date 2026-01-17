#include<iostream>
std::string historypath(std::string_view shell){
    const char* home = std::getenv("HOME");
    std::string user = std::string(std::getenv("USER"));
    std::cout<<"User found : "<<user<<std::endl;
    if(home){
        return std::string(home) + "/.zsh_history";
    }else{
        std::cerr<<"Not found"<<std::endl;
    }
    return "";
}
int main(){
    const char* shell = std::getenv("SHELL");
    if(shell){
        std::cout<<"Found it"<<std::endl;
    }else{
        std::cerr<<"Not found"<<std::endl;
        return EXIT_FAILURE;
    }
    std::string path = historypath(shell);
    return EXIT_SUCCESS;
}