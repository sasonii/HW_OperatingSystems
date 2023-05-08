#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
}

void ctrlCHandler(int sig_num) {
    std::cout << "smash: got ctrl-C" << std::endl;
    SmallShell& smash = SmallShell::getInstance();

    if (smash.current_foreground_job != nullptr) {
        int job_id = smash.current_foreground_job->getJobId();
        pid_t pid = smash.current_foreground_job->getJobProcessId();

        if (kill(pid, SIGKILL) == -1) {
            // TODO: do we need to add exceptions?
            return;
        }

        smash.current_foreground_job = nullptr;
        smash.jobsList.removeJobById(job_id);
        std::cout << "smash: process " << pid << " was killed" << std::endl;
    }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

