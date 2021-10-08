//
// Created by jakob on 06.10.2021.
//

#ifndef LAB1_1_LOGGER_H
#define LAB1_1_LOGGER_H

#include <string>

#define info(l,msg) l._info(msg,__FILE__,__LINE__)
#define warning(l,msg) l._warning(msg,__FILE__,__LINE__)
#define severe(l,msg) l._severe(msg,__FILE__,__LINE__)

class Logger {

    char* p_name = nullptr;

public:
    Logger() = delete;
    explicit Logger(const std::string &process_name);
    static void _info(const std::string &msg, const std::string &file, int line) ;
    static void _warning(const std::string &msg, const std::string &file, int line) ;
    static void _severe(const std::string &msg, const std::string &file, int line) ;
    ~Logger();
};


#endif //LAB1_1_LOGGER_H

