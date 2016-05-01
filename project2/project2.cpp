#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <malloc.h>
#include <ctime>
#include "Process.h"

using namespace std;

int k = 64;
#define memSize 20000

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
                while ((total - memRand) < (k - x))
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

        //sysMalloc(processes);
        //cout<<"TOTAL: "<<total<<endl;
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

void countBuffers(int k, Process *processes[])
{
    int i;
    int totalBufferSize = 0;

    for(i = 0; i < k; i++)
    {
       totalBufferSize += malloc_usable_size(processes[i]->buffer); 
    }

    cout<<"Total space taken by process buffers: "<<totalBufferSize<<endl;
}

//Generate the set of processes with the same seed
void generateProcesses(int k, Process *processes[], int seed)
{
        int x;

        for(x = 0; x < k; x++)
        {
                processes[x] = new Process();
                processes[x]->enterTime = x * 50;
                processes[x]->startMemBlock = -1;
        }

        fillPIDs(k, processes, seed);
        fillMemValues(k, processes, seed);
        fillCpuValues(k, processes, seed);
}

//Increment x and decrement cpu from running processes
//Also frees processes once every processes is finished
int runProcesses(int x, int amount, int k, Process *processes[])
{
    int y;
    bool oneRan = false;
    clock_t time_a;
    clock_t time_b; 

    time_a = clock();
    while(true)
    {
        oneRan = false;
        for (y = 0; y < k; y++)
        {
            if(processes[y]->cpu > 0 && processes[y]->enterTime <= x)
            {
                //If the process has just entered, malloc
                if(processes[y]->enterTime == x)
                {
                    processes[y]->buffer = (char*) malloc (processes[y]->mem+1);
                }

                //If even one process ran, we need to not quit yet
                oneRan = true;
                processes[y]->cpu--;

                if(processes[y]->cpu == 0)
                {
                        //free(processes[y]->buffer);
                }

            }

        }
    
        //If no processes ran, free all processes and quit
        if(!oneRan)
        {
            countBuffers(k, processes);

            /*for (y = 0; y < k; y++)
            {
                free(processes[y]->buffer);
            }*/
            time_b = clock();
            cout<<"Final x value: "<<x<<endl;
            cout<<"Total time: "<<time_b - time_a<<endl;
            return x;
        }
        x += amount;
    }
    return x;
}

void my_malloc(bool *memArray, int processNum, Process *processes[])
{
    int i;
    int j;
    int contiguousSpace = 0;

    for(i = 0; i < memSize; i++)
    {
        if(memArray[i] == false)
        {
            contiguousSpace++;
            if(contiguousSpace == processes[processNum]->mem)
            {
                i -= processes[processNum]->mem;
                processes[processNum]->startMemBlock = ++i;
                
                j = i;
               
                //I set i = j to shut my compiler up 
                for(i = j; i < j + processes[processNum]->mem; i++)
                {
                    memArray[i] = true;
                }
                return;

            }

        }
        else
        {
            contiguousSpace = 0;
        }

    }

}

void my_free(bool *memArray, int processNum, Process *processes[])
{
    int i;
    int freedCount = 0;

    for(i = processes[processNum]->startMemBlock; i < processes[processNum]->startMemBlock + processes[processNum]->mem; i++)
    {
        if(memArray[i] == true)
        {
            freedCount++;
        }
        //cout<<i<<", ";
        memArray[i] = false;
    }

    //cout<<endl<<"Start: "<<processes[processNum]->startMemBlock<<" End: "<<processes[processNum]->startMemBlock + processes[processNum]->mem - 1<<" Memory: "<<processes[processNum]->mem<<" Freed: "<<freedCount<<endl<<endl;
}

int runProcesses2(int x, int amount, int k, Process *processes[])
{
    int y;
    bool oneRan = false;
    clock_t time_a;
    clock_t time_b; 

    //Note: We are intentionally not timing the initial malloc call, as it seems
    //that we're only supposed to be timing my_malloc and my_free
    bool *memArray = (bool*)(malloc(memSize * sizeof(bool)));
    for(y = 0; y < memSize; y++)
    {
        memArray[y] = false;
    }

    cout<<"memArray size: "<<malloc_usable_size(memArray)<<"KB"<<endl;

    time_a = clock();
    while(true)
    {
        oneRan = false;
        for (y = 0; y < k; y++)
        {
            if(processes[y]->cpu > 0 && processes[y]->enterTime <= x)
            {
                //If the process hasn't had memory allocated, try to allocate memory
                //We don't need to check if the process has entered, since that's already been checked 
                if(processes[y]->startMemBlock == -1)
                {
                    //cout<<"Calling my_malloc on process "<<y<<endl;
                    //processes[y]->buffer = (char*) malloc (processes[y]->mem+1);
                    my_malloc(memArray, y, processes);
                }

                //If even one process ran, we need to not quit yet
                if(processes[y]->startMemBlock != -1)
                {
                    oneRan = true;
                    processes[y]->cpu--;
                }

                if(processes[y]->cpu == 0)
                {
                        //cout<<"Calling my_free on process "<<y<<endl;
                        my_free(memArray, y, processes);
                }

            }

        }
    
        //If no processes ran, free all processes and quit
        if(!oneRan)
        {
            
            int i;
            int buffCounter = 0;
            
            for(i = 0; i < memSize; i++)
            {
                if(memArray[i] == true)
                {
                    //cout<<i<<", ";
                    buffCounter++;
                }
            }

            cout<<"buffCounter: "<<buffCounter<<endl;
            
            //countBuffers(k, processes);

            /*for (y = 0; y < k; y++)
            {
                free(processes[y]->buffer);
            }*/
            time_b = clock();
            cout<<"Final x value: "<<x<<endl;
            cout<<"Total time: "<<time_b - time_a<<endl;
            return x;
        }
        x += amount;
    }
    return x;
}

int main()
{
        int x = 0;

        //Pick a new seed to be used for every scheduling method
        srand (time(NULL));
        int seed = rand();

        cout<<"Seed: "<<seed<<endl;


        Process *processes[k];
        cout<<"Number of processes: "<<k<<endl;
        //Run all scheduling method, recreating the processes every time

        generateProcesses(k, processes, seed);
    
        //countBuffers(k, processes);
        x = runProcesses(x, 1, k, processes);

        x = 0;
        generateProcesses(k, processes, seed);
        x = runProcesses2(x, 1, k, processes);
        //while(true);
        return 0;
}
