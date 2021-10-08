//
// Created by jakob on 05.10.2021.
//

#include <sys/stat.h>
#include <fstream>
#include <dirent.h>
#include <csignal>
#include "Logger.h"
#include "Daemon.h"
#include "SystemSignalsHandler.h"
#include <ctime>
#include <errno.h>
#include <string.h>

#define INFO(msg) info(getLogger(),"["+std::to_string(getpid())+"] "+msg)
#define WARNING(msg) warning(getLogger(),"["+std::to_string(getpid())+"] "+msg)
#define SEVERE(msg) severe(getLogger(),"["+std::to_string(getpid())+"] "+msg)


const Logger Daemon::logger = Logger(PROCESS_NAME);
const std::string path_to_pid = std::string(PID_DIRECTORY)+std::string ("/")+std::string (PROCESS_NAME)+std::string (".pid");
const std::string path_to_proc = std::string(PROC_DIRECTORY)+std::string ("/");

std::string Daemon::subfolder_name_new = "NEW";
std::string Daemon::subfolder_name_old = "OLD";

const std::map<std::string, int> Daemon::TokenInfo::token_id_map = {
        {"FROM", Daemon::TokenInfo::FROM},
        {"TO", Daemon::TokenInfo::TO},
        {"DT", Daemon::TokenInfo::DT},
        {"OLD", Daemon::TokenInfo::OLD},
        {"", Daemon::TokenInfo::INDISTINGUISHABLE}
};
const std::string Daemon::TokenInfo::delimiter = "=";


bool Daemon::exists(std::string const &full_path)
{
    struct stat s{};
    if( stat(full_path.c_str(),&s) == 0 )
    {
        return true;
    }
    else
    {
        WARNING("file not found");
        return false;
    }
}

bool Daemon::isFile(std::string const &full_path)
{
    struct stat s{};
    if( stat(full_path.c_str(),&s) == 0 )
    {
        if (s.st_mode & S_IFDIR)
        {
            //it's a directory
            return false;
        }
        if (s.st_mode & S_IFREG)
        {
            //it's a file
            return true;
        }
        //something else
        return false;
    }
    else
    {
        exit(666);
        SEVERE("trying to check filename which is not present at the folder - and the moment ago it was! Magic occurred!");
    }
}

std::string const& Daemon::getNewOrOldFolderName(std::string const &full_path) const // returns output folder prefix. Never call before checking if the file exists!
{
    struct stat s{};
    if( stat(full_path.c_str(),&s) == 0 )
    {
        if (std::difftime(time(nullptr), s.st_mtim.tv_sec) > old_borderline)
        {
            //it's a directory
            return subfolder_name_old;
        }
        return subfolder_name_new;
    }
    else
    {
        SEVERE("trying to check filename which is not present at the folder - and the moment ago it was! Magic occurred!");
        exit(666);
    }
}


void Daemon::moveFiles() const
{
    DIR* src = opendir(path_from.c_str());
    if (src == nullptr)
    {
        SEVERE ("failed to open source dir");
    }
    INFO("opened src="+path_from+" dir, copying files to dst="+path_to);
    struct dirent* curr=readdir(src);
    std::string  path_to_new = path_to + "/" + subfolder_name_old;
    std::string  path_to_old = path_to + "/" + subfolder_name_new;
    mkdir(path_to_new.c_str(), ACCESSPERMS);
    mkdir(path_to_old.c_str(), ACCESSPERMS);
    while (curr != nullptr)
    {
        std::string full_path = path_from + "/" + curr->d_name;
        if (exists(full_path) && isFile(full_path))
        {
            std::ifstream src_stream(full_path, std::ios::binary);

            std::string  full_path_to = path_to + "/" + getNewOrOldFolderName(full_path) + "/" + curr->d_name;
            std::ofstream dest_stream(full_path_to, std::ios::binary);
            if (!(src_stream.is_open()&&dest_stream.is_open()))
            {
                SEVERE("failed to either open src or dst file stream");
            }
            else
            {
                dest_stream << src_stream.rdbuf();
            }
            src_stream.close();
            dest_stream.close();
        }
        curr = readdir(src);
    }
    closedir(src);
}

int Daemon::run()
{
    size_t state = SystemSignalsHandler::getState();
    INFO("getting system signals");
    while ((state & MY_HANDLER_SIGNAL_TERMINATE) == 0)
    {
        INFO("another cycle");
        if (state & MY_HANDLER_SIGNAL_UP)
        {
            INFO("reloading config file");
            reloadConfigFile();
        }
        SystemSignalsHandler::resetState();
        moveFiles();
        INFO("files were moved. Going to sleep");
        if(sleep(repeat_after) != 0)
        {
            INFO("process was waken up by signal");
        }
        INFO("getting system signals");
        state = SystemSignalsHandler::getState();
    }
    INFO("SIGTERM received");
    return  EXIT_SUCCESS;
}

pid_t Daemon::isAnotherInstanceRunning()
{
    std::ifstream pid_file;

    mkdir(PID_DIRECTORY, ACCESSPERMS);
    pid_file.open(path_to_pid);

    if (!pid_file.is_open())
    {
        INFO("none pid file found");
        return 0;
    }
    pid_t pid = 0;
    pid_file >> pid;
    if (pid != 0)
    {
        INFO("found pid = " +std::to_string(pid)+" file of another process");
    }
    else
    {
        WARNING("pid file "+ path_to_pid +" found, but there is no pid");
    }
    pid_file.close();
    struct stat st = {};
    if(stat((path_to_proc+ std::to_string(pid)).c_str(),&st) == 0 || S_ISDIR(st.st_mode))
    {
        INFO("this process is registered at the proc system");
        return pid;
    }
    WARNING("PID file exists, but no process running atm");
    return 0;
}

void Daemon::forkOff(pid_t &pid)
{
    pid = isAnotherInstanceRunning();

    if (pid != 0)
    {
        INFO("sending SIGTERM signal to precess with pid = "+std::to_string(pid));
        if (kill(pid, SIGTERM) == -1)
        {
            SEVERE(std::string("failed to kill with error = ")+strerror(errno));
        }
    }

    pid = fork();
    /* An error occurred */
    if (pid < 0)
    {
        SEVERE("failed to create child process 1");
        exit(EXIT_FAILURE);
    }
    /* Success: Let the parent terminate */
    if (pid > 0)
    {
        INFO("child process 1 created. Terminating the parent");
        exit(EXIT_SUCCESS);
    }

    /* On success: The child process 1 becomes session leader */
    if (setsid() < 0)
    {
        SEVERE("failed to create a new session for child 1");
        exit(EXIT_FAILURE);
    }

    /* Fork off for the second time*/
    pid = fork();

    if (pid < 0)
    {
        SEVERE("failed to create child process 2");
        exit(EXIT_FAILURE);
    }
    /* Success: Let the parent terminate */
    if (pid > 0)
    {
        INFO("child process 2 created. Terminating the child 1");
        exit(EXIT_SUCCESS);
    }
    pid = getpid();
}

void Daemon::unmaskAndChangeDir()
{
    /* Set new file permissions */
    umask(0);

    std::string curr_dir(get_current_dir_name());
    if (curr_dir.empty())
    {
        SEVERE("failed to get current directory");
    }
    cfg_file = curr_dir+"/"+cfg_file;
    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    long x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close ((int)x);
    }
}

void Daemon::createPidFile() const
{
    std::ofstream pid_file;

    mkdir(PID_DIRECTORY, ACCESSPERMS);
    pid_file.open(path_to_pid);

    if (!pid_file.is_open())
    {
        SEVERE("failed to create or rewrite pid file");
        return;
    }
    pid_file << my_pid;
    pid_file.close();
    INFO("successfully created new pid file with pid = "+std::to_string(my_pid));
}

void Daemon::deletePidFile() const
{

    if (std::remove(path_to_pid.c_str()) == -1)
    {
        WARNING("failed to delete the pid file");
    }

}

void Daemon::init()
{
    INFO("trying to init the daemon");
    forkOff(my_pid);
    reloadConfigFile();
    if (!SystemSignalsHandler::regHandler())
    {
        SEVERE("failed to register signal handler function");
    }
    else
    {
        INFO("registered handler for SIGTERM and SIGHUP");
    }
    unmaskAndChangeDir();
    createPidFile();
}

const Logger &Daemon::getLogger()
{
    return logger;
}


void Daemon::reloadConfigFile()
{
    if (tokens == nullptr)
    {
       tokens = new LoaderTokens (Daemon::TokenInfo::delimiter,
                        [this](std::string const &a, int b) { this->setParam(a, b); },
                        [this](std::string const &a) -> int { return Daemon::getParamId(a); });
    }

    ConfigLoader cf(cfg_file);
    if (!cf.valid())
    {
        SEVERE("failed to open config file");
        return;
    }

    cf.load(tokens);
    if (!cf.valid())
    {
        SEVERE("failed to read config file correctly");
    }
    else
    {
        INFO("successfully read config file");
    }
}

void Daemon::loadConfigFile(const std::string &filename)
{
    if (!filename.empty())
    {
        cfg_file = filename;
    }
    reloadConfigFile();
}


void Daemon::setParam(const std::string &val, int param_id)
{
    switch(param_id)
    {
        case TokenInfo::FROM:
            path_from = val;
            break;
        case TokenInfo::TO:
            path_to = val;
            break;
        case TokenInfo::DT:
            repeat_after = std::stoul(val);
            break;
        case TokenInfo::OLD:
            old_borderline = std::stoul(val);
            break;
        case TokenInfo::INDISTINGUISHABLE:
            SEVERE("parser failed to get the token id");
            break;
        default:
            SEVERE("unexpected parameter id to be set");
            break;
    }
}

int Daemon::getParamId(const std::string &par)
{
    if (Daemon::TokenInfo::token_id_map.find(par) != Daemon::TokenInfo::token_id_map.end())
    {
        return Daemon::TokenInfo::token_id_map.at(par);
    }
    return Daemon::TokenInfo::INDISTINGUISHABLE;
}

Daemon::Daemon(const std::string &cfg_filename)
{
    INFO("created new instance of Daemon");
    cfg_file = cfg_filename;
}

Daemon::~Daemon()
{
    INFO("killing the current instance of the daemon");
    delete tokens;
    tokens = nullptr;
    deletePidFile();
}