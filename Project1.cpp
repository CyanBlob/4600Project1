#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "Process.h"

using namespace std;

void fillMemValues(int k, Process *processes[])
{
        int x;

        //The total amount my sum of k processes needs to equal when finished
        int total = k * 20;

        //srand (time(NULL) + k);

        //Generate k memory amounts, averaging to 20
        for (x = 0; x < k; x++)
        {
                //Pick a number between 1 and 100
                int memRand = rand() % 99 + 1;

                /*
                   (total - memRand < k - x) is true when using the current memRand value
                   would put us in a situation where a later process is going to need
                   to take a value of 0 for memory, which needs to be avoided
                   To solve this, decrement memRand
                 */
                while (total - memRand < k - x)
                {
                        memRand--;
                }

                //Same as above, but checks if the we would need a value greater than the max
                while (total - memRand > ((k - x) - 1) * 100)
                {
                        //cout<<"TEST"<<endl;
                        memRand++;
                }

                //If we're on the last process, force the average to be 20
                if(k - x == 1)
                {
                        memRand = total;
                }
                total -= memRand;

                //Save the value
                processes[x]->mem = memRand;
        }
}

void fillCpuValues(int k, Process *processes[])
{
        int x;

        //The total amount my sum of k processes needs to equal when finished
        int total = k * 6000;

        //srand (time(NULL) + k);

        //Generate k cpu cycle amounts, averaging to 6000
        for (x = 0; x < k; x++)
        {
                //Pick a number between 1 and 100
                int cpuRand = rand() % 10000 + 1000;

                /*
                   (total - memRand < k - x) is true when using the current memRand value
                   would put us in a situation where a later process is going to need
                   to take a value of 0 for memory, which needs to be avoided
                   To solve this, decrement memRand
                 */
                while (total - cpuRand < k - x)
                {
                        cpuRand--;
                }

                //Same as above, but checks if the we would need a value greater than the max
                while (total - cpuRand > ((k - x) - 1) * 11000)
                {
                        //cout<<"TEST"<<endl;
                        cpuRand++;
                }

                //If we're on the last process, force the average to be 6000
                if(k - x == 1)
                {
                        cpuRand = total;
                }
                total -= cpuRand;

                //Save the value
                processes[x]->cpu = cpuRand;
        }
}

void fillPIDs(int k, Process *processes[])
{
        int x;
        //srand (time(NULL));
        for (x = 0; x < k; x++)
        {
                if(x == 0)
                        processes[x]->pid = (rand() % k) * 99;
                else
                        processes[x]->pid = processes[x-1]->pid + (rand() % k) + 1;
        }
}

int incrementX(int x, int amount, int k, Process *processes[], int currentProcess)
{
        int y;
        for (y = 0; y < k; y++)
        {
                if(processes[y]->cpu != 0 && processes[y]->enterTime <= x && y != currentProcess)
                {
                        processes[y]->waitTime += amount;
                }

        }

        x += amount;
        return x;
}

void roundRobin(int k, Process *processes[], int quantum, int contextSwitch)
{
        int x = 0;
        int y = 0;
        int currentProcess = 0;
        int lastProcess = 0;
        int switches = 0;
        bool oneRan = false;

        for(y = 0; y < k; y++)
                cout<<"  PROCESS "<<y<<"  REMAINING: "<<processes[y]->cpu<<endl;

        //Find next ready process
        while(true)
        {
                if(processes[currentProcess]->enterTime > x || processes[currentProcess]->cpu == 0)
                {
                        if(0 == currentProcess)
                        {
                                if(oneRan == false)
                                {
                                        //currentProcess = (currentProcess + 1) % k;
                                        cout<<"No process ready "<<x<<endl;
                                        //x += 50;
                                        x = incrementX(x, 50, k, processes, currentProcess);
                                }
                                oneRan = false;
                        }
                        else
                        {
                                //currentProcess = (currentProcess + 1) % k;
                        }
                        currentProcess = (currentProcess + 1) % k;
                }
                else
                {
                        if(processes[currentProcess]->cpu != 0)
                                processes[currentProcess]->cpu -= 50;

                        if (processes[currentProcess]->cpu <= 0)
                        {
                                processes[currentProcess]->cpu = 0;

                        }

                        cout<<"PROCESS "<<currentProcess<<"  REMAINING: "<<processes[currentProcess]->cpu<<endl;

                        //Check if there are no remaining processes
                        for (y = 0; y < k; y++)
                        {
                                if(processes[y]->cpu != 0)
                                {
                                        break;
                                }
                                if(y == k - 1)
                                {
                                        for(y = 0; y < k; y++)
                                                cout<<"  PROCESS "<<y<<" WAITTIME: "<<processes[y]->waitTime<<endl;
                                        cout<<"TOTAL RUNETIME: "<<x  <<" SWITCHES: "<<switches<<endl;
                                        return;
                                }
                        }

                        if(lastProcess != currentProcess)
                        {
                                switches++;
                                x = incrementX(x, 10, k, processes, currentProcess);
                                cout<<"SWITCHING "<<x<<endl;
                        }

                        lastProcess = currentProcess;
                        oneRan = true;

                        x = incrementX(x, 50, k, processes, currentProcess);
                        currentProcess = (currentProcess + 1) % k;
                }
        }

}
int main()
{

        int k = 2;
        int x;

        int quantum = 50;
        int contextSwitch = 10;

        Process *processes[k];

        for(x = 0; x < k; x++)
        {
                processes[x] = new Process();
                processes[x]->enterTime = x * 5000;
                processes[x]->waitTime = 0;
        }


        fillPIDs(k, processes);
        fillMemValues(k, processes);
        fillCpuValues(k, processes);
        //for(x = 0; x < k; x++)
        //    cout<<"PID: "<<processes[x]->pid<<" CPU: "<<processes[x]->cpu<<" MEM: "<<processes[x]->mem<<endl;

        roundRobin(k, processes, quantum, contextSwitch);
        return 0;
}
