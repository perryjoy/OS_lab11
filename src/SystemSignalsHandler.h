//
// Created by jakob on 07.10.2021.
//

#ifndef LAB1_1_SYSTEMSIGNALSHANDLER_H
#define LAB1_1_SYSTEMSIGNALSHANDLER_H

#include <cstddef>

#define MY_HANDLER_SIGNAL_UP 1
#define MY_HANDLER_SIGNAL_TERMINATE 2

class SystemSignalsHandler
{
    static size_t state;
public:
    static void onSigUp(int unused);
    static void onSigTerm(int unused);
    static size_t getState();
    static void resetState();
    static bool regHandler();
};


#endif //LAB1_1_SYSTEMSIGNALSHANDLER_H
