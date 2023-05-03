#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <list>
#include <iostream>

using namespace std;


#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define PATH_MAX_LENGHT (101)
enum Status {
    Foreground, Background, Stopped, Finished
};

class Command {
protected:
    Status cmdStatus;
    const char *cmd_line;
    char **argv;
    int argc;
    bool is_background_command;

public:
    Command(const char *cmd_line);

    virtual ~Command();

    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}

    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char *cmd_line) : Command(cmd_line) {}

    virtual ~ExternalCommand() {}

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);

    virtual ~PipeCommand() {}

    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);

    virtual ~RedirectionCommand() {}

    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChangePrompt : public BuiltInCommand
{
public:
    ChangePrompt(const char *cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~ChangePrompt() {}
    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members
  public:
    ChangeDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~ChangeDirCommand() {}

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~GetCurrDirCommand() {}

    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~ShowPidCommand() {}

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
    JobsList *jobs;

public:
    QuitCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}

    virtual ~QuitCommand() {}

    void execute() override;
};


class JobsList {

public:
    class JobEntry {

    private:
        int jobId;
        int jobProcessId;
        Status jobStatus;
        const char *cmd_line;
        time_t entryTime;

    public:
        JobEntry(int jobId, int jobProcessId, Status jobStatus, const char *cmd_line);
        ~JobEntry() = default;
        bool operator<(const JobEntry& other) const {
            return jobId < other.jobId;
        }
        int getJobId() const {
            return jobId;
        }

        int getJobProcessId() const {
            return jobProcessId;
        }

        Status getJobStatus() const {
            return jobStatus;
        }

        const char* getCmdLine() const {
            return cmd_line;
        }

        time_t getEntryTime() const {
            return entryTime;
        }

        void setJobStatus(Status jobStatus) {
            this->jobStatus = jobStatus;
        }
    };

    // TODO: Add your data members
private:
    list <JobEntry*> jobsList;
    int maxJobID;

public:
    JobsList();

    ~JobsList();

    void addJob(Command *cmd, bool isStopped = false);
    JobEntry* addJob(int jobProcessId, Status jobStatus, const char *cmd_line);
    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);

    int getNumberOfJobs();

    int getMaxJobID();

    int getMaxStoppedJobID();
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
private:
    JobsList *jobs;
public:
    JobsCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}

    virtual ~JobsCommand() {}

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    JobsList *jobs;
public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}

    virtual ~ForegroundCommand() {}

    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    JobsList *jobs;
public:
    BackgroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}

    virtual ~BackgroundCommand() {}

    void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
/* Bonus */
// TODO: Add your data members
public:
    explicit TimeoutCommand(const char *cmd_line);

    virtual ~TimeoutCommand() {}

    void execute() override;
};

class ChmodCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ChmodCommand(const char *cmd_line);

    virtual ~ChmodCommand() {}

    void execute() override;
};

class GetFileTypeCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    GetFileTypeCommand(const char *cmd_line);

    virtual ~GetFileTypeCommand() {}

    void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    SetcoreCommand(const char *cmd_line);

    virtual ~SetcoreCommand() {}

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    JobsList *jobs;
public:
    KillCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}

    virtual ~KillCommand() {}

    void execute() override;
};

class SmallShell {
private:
    JobsList* jobsList;
    string prompt;
    pid_t pid;
    char *last_path;
    bool hasLastPath;
    JobsList::JobEntry *current_foreground_job;


    SmallShell();

public:
    Command *CreateCommand(const char *cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell();

    void executeCommand(const char *cmd_line);
    // TODO: add extra methods as needed

    string getPrompt();
    void changePrompt(std::string s);
    pid_t getPid();
    bool containsLastPath();
    void TurnTrueLastPath();
    char* getLastPath();
    JobsList* getJobsList();
    void setCurrentForegroundJob(JobsList::JobEntry *current_job);

};

#endif //SMASH_COMMAND_H_
