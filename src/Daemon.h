//
// Created by jakob on 05.10.2021.
//
#define I_HATE_ACCESS_BULLSHOT 1

#ifndef LAB1_1_DAEMON_H
#define LAB1_1_DAEMON_H


#include <unistd.h>
#include <string>
#include <cstdlib>
#include <map>
#include "Logger.h"
#include "ConfigLoader.h"


#define PID_DIRECTORY "/var/run/daemontest"
#define PROC_DIRECTORY "/proc"

#define PROCESS_NAME "KYG_daemon_tsk_11"


class LoaderTokens;


class Daemon
{
    class TokenInfo
    {
    public:
        static const std::string delimiter;
        enum TokenId
        {
            FROM,
            TO,
            DT,
            OLD,
            INDISTINGUISHABLE
        };
        static const std::map<std::string, int> token_id_map;
        TokenInfo() = delete;
    };

    LoaderTokens* tokens = nullptr;

    static const Logger logger;
    static std::string subfolder_name_old;
    static std::string subfolder_name_new;

#ifdef PID_FILE
    static pid_t isAnotherInstanceRunning();
    static void forkOff(pid_t &pid);
#else
    pid_t isAnotherInstanceRunning();
    void forkOff(pid_t &pid);
#endif



    std::string path_from;
    std::string path_to;
    std::string cfg_file;
    size_t repeat_after = 60;
    size_t old_borderline = 300;

    pid_t my_pid;


    void unmaskAndChangeDir();

    void createPidFile() const;
    void deletePidFile() const;

    static bool exists(const std::string &full_path) ;
    static bool isFile(std::string const &full_path) ;
    const std::string &getNewOrOldFolderName(const std::string &full_path) const;

    void moveFiles() const;

public:

    void setParam(std::string const & val, int param_id);
    static int getParamId(std::string const & par);
    void reloadConfigFile();

    void init();
    int run();

    Daemon() = delete;
    explicit Daemon(std::string const& cfg_filename);
    ~Daemon();

    // UNUSED METHODS BELOW

private:
    void loadConfigFile(std::string const& filename); // unused
    static const Logger & getLogger();

};


#endif //LAB1_1_DAEMON_H
