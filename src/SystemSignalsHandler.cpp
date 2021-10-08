//
// Created by jakob on 07.10.2021.
//


#include <csignal>
#include <syslog.h>
#include "SystemSignalsHandler.h"

size_t SystemSignalsHandler::state = 0;

void SystemSignalsHandler::onSigUp(int unused)
{
    // only for the debugging purpose
    syslog(LOG_INFO, "recieved signal SIGHUP on handler");
    state |= MY_HANDLER_SIGNAL_UP;
}

void SystemSignalsHandler::onSigTerm(int unused)
{
    // only for the debugging purpose
    syslog(LOG_INFO, "recieved signal SIGTERM on handler");
    state |= MY_HANDLER_SIGNAL_TERMINATE;
}

size_t SystemSignalsHandler::getState()
{
    // only for the debugging purpose
    syslog(LOG_INFO, "handler was accesed for the current state val = %lu", state);
    return state;
}

void SystemSignalsHandler::resetState()
{
    // only for the debugging purpose
    syslog(LOG_INFO, "handler state was reset");
    state = 0;
}

bool SystemSignalsHandler::regHandler()
{
    bool res = true;
    if (signal(SIGHUP, onSigUp) == SIG_ERR)
    {
        res = false;
    }
    if (signal(SIGTERM, onSigTerm) == SIG_ERR)
    {
        res = false;
    }
    return res;
}
