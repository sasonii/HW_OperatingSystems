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
    delete new_path;
}

void JobsCommand::execute() {
    SmallShell::getInstance().getJobsList()->printJobsList();
}

void ForegroundCommand::execute() {
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
        try {
            job_id = stoi(argv[1]);
        } catch (const std::invalid_argument& e) {
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
        try {
            job_id = stoi(argv[1]);
        } catch (const std::invalid_argument& e) {
            std::cerr << "smash error: bg: invalid arguments" << std::endl;
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
        kill(SmallShell::getInstance().getPid(),SIGKILL);
        exit(0);
    }
}

void KillCommand::execute()
{
    if (argv[1] == nullptr || argv[2] == nullptr || argv[1][0] != '-' || argv[3] != nullptr)
    {
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
        return;
    }
    std::string signal_number_str = std::string(argv[1]);
    std::string job_id_str = std::string(argv[2]);
    int signal_number, job_id;
    try {
        signal_number = stoi(signal_number_str.substr(1, signal_number_str.size()));
        job_id = stoi(job_id_str.substr(1, job_id_str.size()));
    } catch (const std::invalid_argument& e) {
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
        perror("smash error: kill failed");
        return;
    }
    else
        cout << "signal number " << signal_number << " was sent to pid " << job->getJobProcessId() << std::endl;
}

void ExternalCommand::execute() {
    string cmd_s = _trim(string(cmd_line));
    bool is_complex = cmd_s.find_first_of("*?") == ((unsigned)-1) ? false : true;
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    pid_t pid = fork();
    if(pid == 0){
        setpgrp();
        if(is_complex){
            execl("/bin/bash", "bash", "-c", cmd_line, NULL);
            // if returned -> error occured
            perror("smash error: exec failed");
            return;
        } else {
            execv(firstWord.c_str(), (argv + 1)); // TODO: maybe + 2
            perror("smash error: exec failed");
            return;
        }
    } else if(pid < 0) {
        perror("smash error: fork failed");
        return;
    } else if(is_background_command){
        SmallShell::getInstance().getJobsList()->addJob(pid, Status::Background, cmd_line);
    } else {
        char* cmd_line_copy = new char[COMMAND_ARGS_MAX_LENGTH + 1];
        strcpy(cmd_line_copy, cmd_line);
        // NOTE :: check why cmd_line changes over runtimes
        JobsList::JobEntry* job =  SmallShell::getInstance().getJobsList()->addJob(pid, Status::Foreground, cmd_line_copy);
        SmallShell::getInstance().setCurrentForegroundJob(job);
        if (waitpid(pid, NULL, WUNTRACED) < 0)
        {
            perror("smash error: wait failed");
        }
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
    argc = _parseCommandLine(cmd_line, argv);
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
            std::cout << " (Stopped)" << std::endl;
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

JobsList::JobEntry* JobsList::addJob(int jobProcessId, Status jobStatus, const char *cmd_line){
    JobsList::JobEntry* job = new JobsList::JobEntry(++maxJobID, jobProcessId, jobStatus, cmd_line);
    jobsList.push_back(job);
    return job;
}