
#include "Daemon.h"



int main(int argc, char ** argv)
{
    if(argc != 2)
    {
        return EXIT_FAILURE;
    }

    Daemon d(argv[1]);
    d.init();
    return d.run();
}