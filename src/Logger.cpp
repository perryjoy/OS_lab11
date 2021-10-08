//
// Created by jakob on 06.10.2021.
//

#include "Logger.h"
#include <syslog.h>

Logger::~Logger()
{
    if (p_name != nullptr)
    {
        closelog();
        delete p_name;
    }
}

void Logger::_severe(const std::string &msg, const std::string &file, int line)
{
    syslog(LOG_ERR, "%s at %s, line %i", msg.c_str(), file.c_str(), line);
}

void Logger::_warning(const std::string &msg, const std::string &file, int line)
{
    syslog(LOG_WARNING, "%s at %s, line %i", msg.c_str(), file.c_str(), line);
}

void Logger::_info(const std::string &msg, const std::string &file, int line)
{
    syslog(LOG_INFO, "%s at %s, line %i", msg.c_str(), file.c_str(), line);
}

Logger::Logger(const std::string &process_name)
{
    size_t len = process_name.length();
    p_name = new(std::nothrow) char[len+1];
    if (p_name == nullptr)
    {
        syslog(LOG_WARNING, "Failed to allocate memory for the process name buffer, using log without specification");
        return;
    }
    process_name.copy(p_name, len);
    p_name[len] = 0;
    openlog(p_name, LOG_PID, LOG_DAEMON);
}
