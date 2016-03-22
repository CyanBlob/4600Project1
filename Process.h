class Process
{
    public:
        int pid;
        int mem;
        int cpu;
        int cpuStart;
        int enterTime;
        int waitTime;

        bool isWaiting;
        bool running;
        bool arrived;
        bool wasRunning;
};
