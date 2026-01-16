#include<iostream>
std::string historypath(std::string_view point){
    const char* home = std::getenv("HOME");
    if (!home) return "";
    if(point.find(".zsh") != std::string::npos){
        for(int i=0;home[i]!='\0';i++){
            std::cout<<home[i];
        }
        return std::string(home) + std::string(point);
    }
    return "";
}
int main(){
    char *point = std::getenv("SHELL");
    if(!point){
        std::cerr<<"Couldn't find the given Shell"<<std::endl;
    }
    else{
        std::cout<<"Found it!"<<std::endl;
    }
    std::cout<<historypath(point)<<std::endl;
    return EXIT_SUCCESS;
}