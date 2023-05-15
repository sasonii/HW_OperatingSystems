#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <list>
#include <time.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

bool isProcessRunning(pid_t pid);

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

JobsList::JobsList(){
    jobsList = list<JobEntry*>();
}
JobsList::~JobsList(){}
void addJob(Command* cmd, bool isStopped = false){

}
void printJobsList();
void killAllJobs();
void removeFinishedJobs();
JobsList::JobEntry * getJobById(int jobId);
void removeJobById(int jobId);
JobsList::JobEntry * getLastJob(int* lastJobId);
JobsList::JobEntry *getLastStoppedJob(int *jobId);

// BUILT-IN COMMANDS //
//
//// NOTE : delete those 3
//void EmptyCommand::execute() {
//
//}
//void CTRLZCommand::execute() {
//    std::cout << "smash: got ctrl-Z" << std::endl;
//    JobsList::JobEntry* job = SmallShell::getInstance().getCurrentForegroundJob();
//    if(job == nullptr){
//        return;
//    }
//    job->setJobStatus(Status::Stopped);
//    pid_t job_pid = job->getJobProcessId();
//    if(kill(job_pid,SIGSTOP) < 0){
//        perror("smash error: kill failed");
//    }
//    if(job->getJobId() == -1){
//        //from external without &
//        JobsList::JobEntry* real_job =  SmallShell::getInstance().getJobsList()->addJob(job->getJobProcessId(), Status::Stopped, job->getCmdLine());
//        (SmallShell::getInstance().getJobsList())->addJob(real_job);
//    }
//    else{
//        (SmallShell::getInstance().getJobsList())->addJob(job);
//    }
//    std::cout<<"smash: process "<< job_pid << " was stopped" << std::endl;
//}
//void CTRLCCommand::execute() {
//    std::cout<<"smash: got ctrl-C\n";
//    JobsList::JobEntry* job = SmallShell::getInstance().getCurrentForegroundJob();
//    if(job==nullptr){
//        return;
//    }
//    job->setJobStatus(Status::Finished);
//
//    if(kill(job->getJobProcessId(),SIGKILL) < 0){
//        perror("smash error: kill failed");
//    }
//    std::cout<<"smash: process "<< job->getJobProcessId() << " was killed" << std::endl;
//}

void ChangePrompt::execute() {
    if(argc == 1){
        SmallShell::getInstance().changePrompt( "smash");
    }
    else {
        SmallShell::getInstance().changePrompt(argv[1]);
    }
}

void ShowPidCommand::execute() {
    std::cout << "smash pid is " << SmallShell::getInstance().getPid() << "\n";
}

void GetCurrDirCommand::execute() {
    char *path = new char[PATH_MAX_LENGHT];
    if (!getcwd(path, PATH_MAX_LENGHT))
    {
        perror("smash error: getcwd failed");
        return;
    }
    std::cout << path << "\n";
}

void ChangeDirCommand::execute() {
    if(argc > 2){
        std::cerr << "smash error: cd: too many arguments" << '\n';
        return;
    }
    char *new_path = new char[PATH_MAX_LENGHT];
    char *last_path = SmallShell::getInstance().getLastPath();
    if(!strcmp(argv[1], "-")){
        if(!SmallShell::getInstance().containsLastPath()){
            std::cerr << "smash error: cd: OLDPWD not set" << '\n';
            return;
        }
        else{
            if (!getcwd(new_path, PATH_MAX_LENGHT))
            {
                perror("smash error: getcwd failed");
                return;
            }
            else {
                if(chdir(last_path) != 0){
                    perror("smash error: chdir failed");
                    return;
                }
            }

        }
    }
    else{
        if (!getcwd(new_path, PATH_MAX_LENGHT))
        {
            perror("smash error: getcwd failed");
            return;
        }
        else{
            if (chdir(argv[1]) != 0)
            {
                perror("smash error: chdir failed");
                return;
            }
        }
    }
    strncpy(last_path, new_path, PATH_MAX_LENGHT - 1);
    SmallShell::getInstance().TurnTrueLastPath();
    delete new_path;
}

void JobsCommand::execute() {
    SmallShell::getInstance().getJobsList()->removeFinishedJobs();
    SmallShell::getInstance().getJobsList()->printJobsList();
}

void ForegroundCommand::execute() {
    SmallShell::getInstance().getJobsList()->removeFinishedJobs();
    if(argc > 2){
        std::cerr << "smash error: fg: invalid arguments" << std::endl;
        return;
    }
    int job_id;
    JobsList* jobs = SmallShell::getInstance().getJobsList();
    if(argc == 1 ){
        if(!jobs->getNumberOfJobs()){
            std::cerr << "smash error: fg: jobs list is empty" << std::endl;
            return;
        }
        job_id = jobs->getMaxJobID();
    }
    if(argc == 2){
        job_id = atoi(argv[1]);
        if(!job_id){
            std::cerr << "smash error: fg: invalid arguments" << std::endl;
            return;
        }
    }
    JobsList::JobEntry* job = jobs->getJobById(job_id);
    if(job == nullptr){
            std::cerr << "smash error: fg: job-id " << job_id << " does not exist" << std::endl;
            return;
    }
    pid_t job_pid = job->getJobProcessId();
    std::cout << job->getCmdLine() << " : " << job_pid << std::endl;
    job->setJobStatus(Status::Foreground);
    SmallShell::getInstance().setCurrentForegroundJob(job);
    kill(job_pid, SIGCONT);
    jobs->removeJobById(job_id);
    waitpid(job_pid, NULL, WUNTRACED);
    SmallShell::getInstance().setCurrentForegroundJob(nullptr);
}

void BackgroundCommand::execute() {
    SmallShell::getInstance().getJobsList()->removeFinishedJobs();
    if(argc > 2){
        std::cerr << "smash error: bg: invalid arguments" << std::endl;
        return;
    }
    int job_id;
    JobsList* jobs = SmallShell::getInstance().getJobsList();
    if(argc == 1){
        job_id = jobs->getMaxStoppedJobID();
        if(job_id == 0){
            std::cerr << "smash error: bg: there is no stopped jobs to resume" << std::endl;
            return;
        }
    }
    if(argc == 2){
        job_id = atoi(argv[1]);
        if(!job_id){
            std::cerr << "smash error: fg: invalid arguments" << std::endl;
            return;
        }
    }
    JobsList::JobEntry* job = jobs->getJobById(job_id);
    if(job == nullptr){
        std::cerr << "smash error: bg: job-id " << job_id << " does not exist" << std::endl;
        return;
    }
    if(job->getJobStatus() == Status::Background){
        std::cerr << "smash error: bg: job-id " << job_id << " is already running in the background" << std::endl;
        return;
    }
    pid_t job_pid = job->getJobProcessId();
    std::cout << job->getCmdLine() << " : " << job_pid << std::endl;
    job->setJobStatus(Status::Background);
    kill(job_pid, SIGCONT);
}

void QuitCommand::execute() {
    SmallShell::getInstance().getJobsList()->removeFinishedJobs();
    if (argc == 1)
    {
        kill(SmallShell::getInstance().getPid(),SIGKILL);
        exit(0);
    }
    else if (!strcmp(argv[1], "kill"))
    {
        JobsList *jobsList = (SmallShell::getInstance().getJobsList());
        // NOTE :: implement getNumberOfUnfinishedJobs
        std::cout << "smash: sending SIGKILL signal to " << jobsList->getNumberOfJobs() << " jobs:" << std::endl;
        jobsList->killAllJobs();
//        kill(SmallShell::getInstance().getPid(),SIGKILL);
        exit(0);
    }
}

void KillCommand::execute()
{
    SmallShell::getInstance().getJobsList()->removeFinishedJobs();
    if (argv[1] == nullptr || argv[2] == nullptr || argv[1][0] != '-' || argv[3] != nullptr)
    {
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
        return;
    }
    std::string signal_number_str = std::string(argv[1]);
    std::string job_id_str = std::string(argv[2]);
    int signal_number, job_id;
    job_id = atoi(job_id_str.c_str());
    if(!job_id){
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
        return;
    }
    signal_number = atoi(signal_number_str.substr(1, signal_number_str.size()).c_str());
    if(!signal_number){
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
        return;
    }

    JobsList::JobEntry *job = SmallShell::getInstance().getJobsList()->getJobById(job_id);
    if (job == nullptr)
    {
        std::cerr << "smash error: kill: job-id " << job_id << " does not exist" << std::endl;
        return;
    }
    if (kill(job->getJobProcessId(), signal_number) == -1){ // no need to return because there is no more code, but just in case
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
        //perror("smash error: kill failed"); // NOTE :: replace those 2 lines
        return;
    }
    else{
        cout << "signal number " << signal_number << " was sent to pid " << job->getJobProcessId() << std::endl;
        if(signal_number == 9){
            job->setJobStatus(Status::Finished);
        }
        if(signal_number == 19){
            job->setJobStatus(Status::Stopped);
        }
    }
}

void ExternalCommand::execute() {
    SmallShell::getInstance().getJobsList()->removeFinishedJobs();
    string cmd_s = _trim(string(cmd_line));
    bool is_complex = cmd_s.find_first_of("*?") == std::string::npos ? false : true;
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    pid_t pid = fork();
    if(pid == 0){
        setpgrp();
        if(is_complex){
            execl("/bin/bash", "bash", "-c", cmd_line, NULL);
            // if returned -> error occured
            perror("smash error: exec failed");
            exit(0);
        } else {
            execvp(firstWord.c_str(), (argv));
            perror("smash error: exec failed");
            exit(0);
        }
    } else if(pid < 0) {
        perror("smash error: fork failed");
        return;
    } else if(is_background_command){
        char* cmd_line_copy = new char[COMMAND_ARGS_MAX_LENGTH + 1];
        strcpy(cmd_line_copy, cmd_line);
        SmallShell::getInstance().getJobsList()->addJob(pid, Status::Background, cmd_line_copy);
    } else {
        char* cmd_line_copy = new char[COMMAND_ARGS_MAX_LENGTH + 1];
        strcpy(cmd_line_copy, cmd_line);
        // NOTE :: check why cmd_line changes over runtimes
        //JobsList::JobEntry* job =  SmallShell::getInstance().getJobsList()->addJob(pid, Status::Foreground, cmd_line_copy);
        JobsList::JobEntry* job = new JobsList::JobEntry(-1, pid, Status::Foreground, cmd_line_copy);
        SmallShell::getInstance().setCurrentForegroundJob(job);
        if (waitpid(pid, NULL, WUNTRACED) < 0)
        {
            perror("smash error: wait failed");
        }
        SmallShell::getInstance().setCurrentForegroundJob(nullptr);
    }
}

void PipeCommand::execute() {
    SmallShell::getInstance().getJobsList()->removeFinishedJobs();
    string cmd_s = _trim(string(cmd_line));
    int first_occurance = cmd_s.find_first_of("|");
    string first_command = cmd_s.substr(0, first_occurance);
    bool is_error_pipe = cmd_s.at(first_occurance + 1) == '&';
    string second_command = _trim(cmd_s.substr(first_occurance + 1 + (int)is_error_pipe, cmd_s.size()));
    int fd[2];
    if (pipe(fd) < 0)
    {
        perror("smash error: pipe failed");
    }
    pid_t pid_1 = fork();
    if (pid_1 == 0) {
        setpgrp();
        // first child
        if(is_error_pipe){
            dup2(fd[1],2);
        } else {
            dup2(fd[1],1);
        }
        close(fd[0]);
        close(fd[1]);
        SmallShell::getInstance().executeCommand(first_command.c_str());
        exit(0);

    } else if(pid_1 < 0) {
        perror("smash error: fork failed");
        return;
    }

    pid_t pid_2 = fork();
    if (pid_2 == 0) {
        setpgrp();
        // second child
        dup2(fd[0],0);
        close(fd[0]);
        close(fd[1]);
        SmallShell::getInstance().executeCommand(second_command.c_str());
        exit(0);
    } else if(pid_2 < 0) {
        perror("smash error: fork failed");
        return;
    }
    close(fd[0]);
    close(fd[1]);
    char* cmd_line_copy = new char[COMMAND_ARGS_MAX_LENGTH + 1];
    strcpy(cmd_line_copy, cmd_line);
    // NOTE :: check why cmd_line changes over runtimes
    JobsList::JobEntry* job =  SmallShell::getInstance().getJobsList()->addJob(pid_2, Status::Foreground, cmd_line_copy);
    SmallShell::getInstance().setCurrentForegroundJob(job);
    if (waitpid(pid_2, NULL, WUNTRACED) < 0)
    {
        perror("smash error: wait failed");
    }
    SmallShell::getInstance().setCurrentForegroundJob(nullptr);
}

void RedirectionCommand::execute() {
    SmallShell::getInstance().getJobsList()->removeFinishedJobs();
    string cmd_s = _trim(string(cmd_line));
    int first_occurance = cmd_s.find_first_of(">");
    string command_to_exe = cmd_s.substr(0, first_occurance);
    bool is_append = cmd_s.at(first_occurance + 1) == '>';
    string file_to_write = _trim(cmd_s.substr(first_occurance + 1 + (int)is_append, cmd_s.size()));

    pid_t pid = fork();

    if (pid == 0) {
        setpgrp();
        // child
        close(1); //close stdout
        if(is_append){
            if(open(file_to_write.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0655) == -1){
                perror("smash error: open failed");
                exit(0);
            }
        }
        else{
            if(open(file_to_write.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0655) == -1) {
                perror("smash error: open failed");
                exit(0);
            }
        }
        SmallShell::getInstance().executeCommand(command_to_exe.c_str()); //executing the command that was supplied the pipe
        exit(0);
    }else if (pid > 0){
        //father
        if(is_background_command){
            char* cmd_line_copy = new char[COMMAND_ARGS_MAX_LENGTH + 1];
            strcpy(cmd_line_copy, cmd_line);
            SmallShell::getInstance().getJobsList()->addJob(pid, Status::Background, cmd_line_copy);
        } else {
            char* cmd_line_copy = new char[COMMAND_ARGS_MAX_LENGTH + 1];
            strcpy(cmd_line_copy, cmd_line);
            // NOTE :: check why cmd_line changes over runtimes
            //JobsList::JobEntry* job =  SmallShell::getInstance().getJobsList()->addJob(pid, Status::Foreground, cmd_line_copy);
            JobsList::JobEntry* job = new JobsList::JobEntry(-1, pid, Status::Foreground, cmd_line_copy);
            SmallShell::getInstance().setCurrentForegroundJob(job);
            if (waitpid(pid, NULL, WUNTRACED) < 0)
            {
                perror("smash error: wait failed");
            }
            SmallShell::getInstance().setCurrentForegroundJob(nullptr);
        }
    } else {
        perror("smash error: fork failed");
        return;
    }
}

void SetcoreCommand::execute() {
    SmallShell::getInstance().getJobsList()->removeFinishedJobs();
    if(argc != 3){
        std::cerr << "smash error: setcore: invalid arguments" << std::endl;
        return;
    }
    int job_id, core_num;
    JobsList* jobs = SmallShell::getInstance().getJobsList();

    try {
        job_id = stoi(argv[1]);
        core_num = stoi(argv[2]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "smash error: setcore: invalid arguments" << std::endl;
        return;
    }
    if((core_num >= get_nprocs()) | (core_num < 0)){
        std::cerr << "smash error: setcore: invalid core number" << std::endl;
        return;
    }

    JobsList::JobEntry* job = jobs->getJobById(job_id);
    if(job == nullptr){
        std::cerr << "smash error: setcore: job-id " << job_id << " does not exist" << std::endl;
        return;
    }

    cpu_set_t cpuset;

    // Clear the CPU set and set CPU 0
    CPU_ZERO(&cpuset);
    CPU_SET(core_num, &cpuset);
    // Set the process affinity to CPU 0
    if (sched_setaffinity(job->getJobProcessId(), sizeof(cpu_set_t), &cpuset) == -1) {
        perror("smash error: sched_setaffinity failed");
        return;
    }
}

void GetFileTypeCommand::execute() {
    if(argc != 2){
        std::cerr << "smash error: getfiletype: invalid arguments" << std::endl;
        return;
    }
    string cmd_s = _trim(string(cmd_line));
    string second_word = _trim(cmd_s.substr(cmd_s.find_first_of(" ") + 1, cmd_s.size()));

    struct stat s;
    if (lstat(second_word.c_str(), &s) == -1) {
        perror("smash error: lstat failed");
        return;
    }

    if (S_ISREG(s.st_mode)) {
        std::cout << second_word << "'s type is \"regular file\" and takes up " << s.st_size << " bytes" << std::endl;
    } else if (S_ISDIR(s.st_mode)) {
        std::cout << second_word << "'s type is \"directory\" and takes up " << s.st_size << " bytes" << std::endl;
    } else if (S_ISCHR(s.st_mode)) {
        std::cout << second_word << "'s type is \"character device\" and takes up " << s.st_size << " bytes" << std::endl;
    } else if (S_ISBLK(s.st_mode)) {
        std::cout << second_word << "'s type is \"block device\" and takes up " << s.st_size << " bytes" << std::endl;
    } else if (S_ISFIFO(s.st_mode)) {
        std::cout << second_word << "'s type is \"FIFO\" and takes up " << s.st_size << " bytes" << std::endl;
    } else if (S_ISLNK(s.st_mode)) {
        std::cout << second_word << "'s type is \"symbolic link\" and takes up " << s.st_size << " bytes" << std::endl;
    } else if (S_ISSOCK(s.st_mode)) {
        std::cout << second_word << "'s type is \"דocket\" and takes up " << s.st_size << " bytes" << std::endl;
    }
//    else {
//        std::cout << "Unknown file type\n";
//    }
}

void ChmodCommand::execute() {
    if(argc != 3){
        std::cerr << "smash error: chmod: invalid arguments" << std::endl;
        return;
    }

    string cmd_s = _trim(string(cmd_line));
    string second_word = _trim(cmd_s.substr(cmd_s.find_first_of(" ") + 1, cmd_s.size()));

    int new_mode = std::stoi(argv[1], nullptr, 8);
    const char* path_to_file = argv[2];

    try {
        new_mode = std::stoi(argv[1], nullptr, 8);
    } catch (const std::invalid_argument& e) {
        std::cerr << "smash error: chmod: invalid arguments" << std::endl;
        return;
    }
    if (new_mode < 0 || new_mode > 0777) {
        std::cerr << "smash error: chmod: invalid arguments" << std::endl;
        return;
    }
    if (chmod(path_to_file, new_mode) == -1) {
        perror("smash error: chmod failed");
        return;
    }
}

// COMMAND //
Command::Command(const char *cmd_line){
    this->cmd_line = cmd_line;
    is_background_command = _isBackgroundComamnd(cmd_line);
    char *command = new char[COMMAND_ARGS_MAX_LENGTH + 1];
    strcpy(command, cmd_line);
    _removeBackgroundSign(command);
    argv = new char*[COMMAND_MAX_ARGS];
    argc = _parseCommandLine(command, argv);
}
Command::~Command(){
    delete[] argv;
}

// SMALL SHELL //
SmallShell::SmallShell() {
    jobsList =  new JobsList();
    prompt = "smash";
    pid = getpid();
    hasLastPath = false;
    last_path = new char[PATH_MAX_LENGHT];
    current_foreground_job = nullptr;
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    if(firstWord.back() == '&'){
        firstWord = firstWord.substr(0, firstWord.length() - 1);
    }
    if (cmd_s.find_first_of("|") != static_cast<std::string::size_type>(-1))
    {
        return new PipeCommand(cmd_line);
    }
    if (cmd_s.find_first_of(">") != static_cast<std::string::size_type>(-1))
    {
        return new RedirectionCommand(cmd_line);
    }
    if (firstWord.compare("setcore") == 0) {
        return new SetcoreCommand(cmd_line);
    }
    if (firstWord.compare("getfiletype") == 0) {
        return new GetFileTypeCommand(cmd_line);
    }if (firstWord.compare("chmod") == 0) {
        return new ChmodCommand(cmd_line);
    }
//    if(cmd_s == "^Z"){
//        return new CTRLZCommand(cmd_line);
//    }
//    if(cmd_s == "^C"){
//        return new CTRLCCommand(cmd_line);
//    }
//    if(cmd_s[0] == '^'){
//        return new EmptyCommand(cmd_line);
//    }

    if (firstWord.compare("chprompt") == 0) {
        return new ChangePrompt(cmd_line);
    }
    if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    }
    if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    }
    if (firstWord.compare("cd") == 0)    {
        return new ChangeDirCommand(cmd_line);
    }
    if (firstWord.compare("jobs") == 0) {
        return new JobsCommand(cmd_line, SmallShell::getInstance().getJobsList());
    }
    if (firstWord.compare("fg") == 0) {
        return new ForegroundCommand(cmd_line, SmallShell::getInstance().getJobsList());
    }
    if (firstWord.compare("bg") == 0) {
        return new BackgroundCommand(cmd_line, SmallShell::getInstance().getJobsList());
    }
    if (firstWord.compare("quit") == 0) {
        return new QuitCommand(cmd_line, SmallShell::getInstance().getJobsList());
    }
    if (firstWord.compare("kill") == 0) {
        return new KillCommand(cmd_line, SmallShell::getInstance().getJobsList());
    }
    return new ExternalCommand(cmd_line);
    //return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
     Command* cmd = CreateCommand(cmd_line);
     cmd->execute();
}

string SmallShell::getPrompt(){
    return prompt;
}

void SmallShell::changePrompt(std::string new_prompt){
    prompt = new_prompt;
}

pid_t SmallShell::getPid(){
    return pid;
}

bool SmallShell::containsLastPath(){
    return hasLastPath;
}

void SmallShell::TurnTrueLastPath(){
    hasLastPath = true;
}

char* SmallShell::getLastPath(){
    return last_path;
}

JobsList* SmallShell::getJobsList(){
    return jobsList;
}

JobsList::JobEntry* SmallShell::getCurrentForegroundJob(){
    return current_foreground_job;
}

void SmallShell::setCurrentForegroundJob(JobsList::JobEntry *current_job){
    this->current_foreground_job = current_job;
}

// JOBS LIST :: JOB ENTRY //
JobsList::JobEntry::JobEntry(const int jobId, const int jobProcessId, Status jobStatus, const char *cmd_line)
: jobId(jobId), jobProcessId(jobProcessId), jobStatus(jobStatus), cmd_line(cmd_line) {
    entryTime = time(NULL);
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    for (JobsList::JobEntry* job : jobsList) {
        if (job->getJobId() == jobId) {
            return job;
        }
    }
    return nullptr;
}

// JOBS LIST //
void JobsList::printJobsList() {
    // Sort the jobs list by job ID
    jobsList.sort();

    // Print each job entry in the sorted list
    for (const JobsList::JobEntry* job : jobsList) {
        std::cout << "[" << job->getJobId() << "] " << job->getCmdLine() << " : " << job->getJobProcessId() << " " << difftime(time(NULL), job->getEntryTime()) << " secs";
        if (job->getJobStatus() == Status::Stopped) {
            std::cout << " (stopped)" << std::endl;
        } else {
            std::cout << std::endl;
        }
    }
}

int JobsList::getNumberOfJobs(){
    return jobsList.size();
}

int JobsList::getMaxJobID() {
    return maxJobID;
}

void JobsList::removeJobById(int jobId) {
    for (auto iter = jobsList.begin(); iter != jobsList.end(); iter++) {
        if ((*iter)->getJobId() == jobId) {
            jobsList.erase(iter);
            if((*iter)->getJobId() == maxJobID){
                maxJobID--;
            }
            return;
        }
    }
}

void JobsList::removeJobByPid(pid_t pid) {
    for (auto iter = jobsList.begin(); iter != jobsList.end(); iter++) {
        if ((*iter)->getJobProcessId() == pid) {
            jobsList.erase(iter);
            if((*iter)->getJobId() == maxJobID){
                maxJobID--;
            }
            return;
        }
    }
}

int JobsList::getMaxStoppedJobID(){
    int max_job_id = 0;
    for (auto iter = jobsList.begin(); iter != jobsList.end(); iter++) {
        if ((*iter)->getJobStatus() == Status::Stopped && (*iter)->getJobId() > max_job_id) {
            max_job_id = (*iter)->getJobId();
        }
    }
    return max_job_id;
}

void JobsList::killAllJobs() {
    for (const JobsList::JobEntry* job : jobsList) {
        std::cout << job->getJobProcessId() << ": " << job->getCmdLine() << std::endl;
        kill(job->getJobProcessId(), SIGKILL);
    }
}

void JobsList::removeFinishedJobs() {
    // chatgpt code to overcome sleep problem with WIFIEXIT
    pid_t finproc;
    finproc = waitpid(-1, NULL, WNOHANG);
    while (finproc > 0)
    { //checks if there is a finished child
        removeJobByPid(finproc);
        finproc = waitpid(-1, NULL, WNOHANG);
    }

    int max_id = 0;

//    for (auto it = jobsList.begin(); it != jobsList.end();) {
//        JobEntry* job = *it;
//        if (job->getJobStatus() == Status::Finished) {
//            it = jobsList.erase(it);
//            delete job; // Clean up the dynamically allocated JobEntry object
//        } else {
//            ++it;
//        }
//    }


    for (auto it = begin(jobsList); it != end(jobsList); ++it)
    {
        if (((*it)->getJobStatus() == Status::Finished)){ //  || !isProcessRunning((*it)->getJobProcessId()
            //cout << "removed. " << !isProcessRunning((*it)->getJobProcessId()) << ". " << ((*it)->getJobStatus() == Status::Finished) << endl;
            it = jobsList.erase(it);
            it--;
            continue;
        }
        if (max_id < (*it)->getJobId())
        {
            max_id = (*it)->getJobId();
        }
    }
    maxJobID = max_id;
}

bool isProcessRunning(pid_t pid) {
    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);

    if (result == 0) {
        // Process is still running
        return true;
    }

    if (WIFEXITED(status)) {
        // Process exited normally
        //std::cout << "Process exited with status: " << WEXITSTATUS(status) << std::endl;
        return false;
    } else if (WIFSIGNALED(status)) {
        // Process exited due to a signal
        //std::cout << "Process exited due to signal: " << WTERMSIG(status) << std::endl;
        if(WTERMSIG(status)){
            return false;
        }
    }

    if (result == -1) {
        //cout << -1 << endl;
        //std::cerr << "Error occurred while waiting for process" << std::endl;
        return false;
    }
    return false;
}

JobsList::JobEntry* JobsList::addJob(int jobProcessId, Status jobStatus, const char *cmd_line){
    JobsList::JobEntry* job = new JobsList::JobEntry(++maxJobID, jobProcessId, jobStatus, cmd_line);
    jobsList.push_back(job);
    return job;
}
JobsList::JobEntry* JobsList::addJob(JobsList::JobEntry* job){
    if(JobsList::getJobById(job->getJobId()) == nullptr){
        jobsList.push_back(job);
        return job;
    }
    else
    {
        //cout << "JOB ALREADY EXIST::DEBUG" << endl;
        return nullptr;
    }
}

