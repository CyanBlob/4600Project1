class Process
{
    public:
        int pid;
        int mem;
        int cpu;
        int cpuStart;
        int enterTime;
        int waitTime;

        char * buffer;

        bool isWaiting;
        bool running;
        bool arrived;
};
