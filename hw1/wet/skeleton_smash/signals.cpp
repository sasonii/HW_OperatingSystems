#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    std::cout << "smash: got ctrl-Z" << std::endl;
    JobsList::JobEntry* job = SmallShell::getInstance().getCurrentForegroundJob();
    if(job == nullptr){
        return;
    }
    job->setJobStatus(Status::Stopped);
    pid_t job_pid = job->getJobProcessId();
    if(kill(job_pid,SIGSTOP) < 0){
        perror("smash error: kill failed");
    }
    (SmallShell::getInstance().getJobsList())->addJob(job);
    std::cout << "smash: process " << job_pid << " was stopped" << std::endl;
}
void ctrlCHandler(int sig_num) {
    std::cout << "smash: got ctrl-C" << std::endl;
    JobsList::JobEntry* job = SmallShell::getInstance().getCurrentForegroundJob();
    if(job==nullptr){
        return;
    }
    job->setJobStatus(Status::Finished);

    if(kill(job->getJobProcessId(),SIGKILL) < 0){
        perror("smash error: kill failed");
    }
    std::cout << "smash: process "<< job->getJobProcessId() << " was killed" << std::endl;
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

