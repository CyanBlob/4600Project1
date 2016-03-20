class Process
{
    public:
        int pid;
        int mem;
        int cpu;
        int enterTime;
        int waitTime;

        bool isWaiting;
        bool running;
        bool arrived;
        bool wasRunning;
};
