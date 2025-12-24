#include<iostream>
#include<string>
#include<vector>
#include<sstream>
#include<unistd.h> //for fork(),execvp(),chdir(),getcwd()
#include<wait.h> // for waitpid()
#include<fcntl.h> // for open(),O_WRONLY,O_CREAT,O_TRUNC
#include<cstdlib> //for exit(),getenv()
#include<pwd.h>
using namespace std;
// function to execute user command
vector <string> parseLine(const string& line){
vector<string> tokens;
stringstream ss(line);
string word;
while (ss>>word){
tokens.push_back(word);
}
return tokens;
}
// function to execute user command
void executeCommand(vector<string>& tokens, vector<string>& history){
if (tokens.empty()) return;
string command=tokens[0];
//----Built-in commands---
// Exit command
if(command == "exit" || command =="shutdown" || command =="poweroff"){
cout<<"Exiting Mini Bash. Have a nice day!" <<endl;
exit(0);
}
//change directory 
if(command == "cd"){
string path;
if(tokens.size()<2){
char* home = getenv("HOME");
if(home != nullptr){
path = home;
}
else{
struct passwd* pw = getpwuid(getuid());
path = pw->pw_dir;
}}
else if(tokens[1] == "~"){
char* home = getenv("HOME");
if (home != nullptr)
path=home;
}else{
path = tokens[1];
}
if(chdir(path.c_str()) !=0){
perror("cd failed");
}
return;
}
// Show history
if (command == "history"){
cout<<"\n-----Command history ------\n";
for (size_t i = 0; i < history.size(); i++) {
cout << i + 1 << " " << history[i] << endl;
}
cout<<"---------------------------\n";
return;
}
string outputFile = "";
int redirectPos = -1;
for (size_t i = 0; i < tokens.size(); i++) {
if (tokens[i] == ">") {
redirectPos = i;
if (i + 1 < tokens.size()) {
outputFile = tokens[i + 1];
}
break;
}
}
//Fork a new process
pid_t pid = fork();
if(pid==0){
// --child process--
// handle output redirection
if (redirectPos !=-1 && !outputFile.empty()){
int fd = open(outputFile.c_str(),O_WRONLY | O_CREAT |O_TRUNC,0644);
if (fd <0){
perror("Cannot open file");
exit(1);
}
dup2(fd, STDOUT_FILENO);
close(fd);
tokens.erase(tokens.begin() + redirectPos, tokens.begin() +redirectPos+2);
}
vector<char*> args;
for (auto& t : tokens){
args.push_back(const_cast<char*>(t.c_str()));
}
args.push_back(nullptr);
if (execvp(args[0], args.data()) < 0) {
cerr << "Command not found:" << tokens[0] << endl;
exit(1);
}
} else if (pid > 0) {
int status;
waitpid(pid, &status, 0);
} else {
perror("Fork failed");
}
}
int main() {
string line;
vector<string> history;
cout <<"---- Simple Mini Bash Shell ----\n";
cout <<"Supported: cd, history, exit, > redirection, and system commands (ls, 
cat, etc.)\n";
while (true) {
char cwd[1024];
getcwd(cwd, sizeof(cwd));
cout <<"\033[1;32mminibash:" <<cwd <<"$ \033[0m";
if (!getline(cin, line)) break;
if(line.empty()) continue;
history.push_back(line);
vector<string> tokens = parseLine(line);
executeCommand(tokens, history);
}
return 0;
}
