#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "Process.h"

using namespace std;

void fillMemValues(int k, Process *processes[], int seed)
{
        int x;

        //The total amount my sum of k processes needs to equal when finished
        int total = k * 20;

        //srand (time(NULL) + k);
        srand(seed);

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

        cout<<"TOTAL: "<<total<<endl;
}

void fillCpuValues(int k, Process *processes[], int seed)
{
        int x;

        //The total amount my sum of k processes needs to equal when finished
        int total = k * 6000;

        //srand (time(NULL) + k);
        srand(seed);

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
                processes[x]->cpuStart = cpuRand;
        }
}

void fillPIDs(int k, Process *processes[], int seed)
{
        int x;
        srand(seed);
        for (x = 0; x < k; x++)
        {
                if(x == 0)
                        processes[x]->pid = (rand() % k) * 99;
                else
                        processes[x]->pid = processes[x-1]->pid + (rand() % k) + 1;
        }
}

//Increment x and add waittime to every process that is waiting
int incrementX(int x, int amount, int k, Process *processes[])
{
        int y;

        for (y = 0; y < k; y++)
        {

                if(processes[y]->cpu > 0 && processes[y]->enterTime <= x && processes[y]->running == false)
                {
                        processes[y]->waitTime += amount;
                }

        }

        x += amount;
        return x;
}


//Generate the set of processes with the same seed
void generateProcesses(int k, Process *processes[], int seed)
{
        int x;

        for(x = 0; x < k; x++)
        {
                processes[x] = new Process();
                processes[x]->enterTime = x * 50;
                processes[x]->waitTime = 0;
                processes[x]->arrived = false;
                processes[x]->running = false;
        }

        fillPIDs(k, processes, seed);
        fillMemValues(k, processes, seed);
        fillCpuValues(k, processes, seed);
}

int main()
{

        int pizza = 0;
        int k = 64;
        int i = 0;

        int quantum = 50;
        int contextSwitch = 10;

        //Pick a new seed to be used for every scheduling method
        srand (time(NULL));
        int seed = rand();

        cout<<"SEED: "<<seed<<endl;


        Process *processes[k];

        //Run all scheduling method, recreating the processes every time

        generateProcesses(k, processes, seed);

        for (i = 0; i < k; i++)
        {
                cout<<processes[i]->mem<<endl;
        }

        return 0;
}
