//Project 2 - Andrew Thomas and Michael Thomas

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
int memSize = 20000;

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

//Run processes using system malloc
int runProcesses(int x, int amount, int k, Process *processes[])
{
    int y, z;
    int memUsed = 0;
    bool oneRan = false;
    clock_t time_a;
    clock_t time_b;

    time_a = clock();
    while(true)
    {
        //Reset the run counter
        oneRan = false;

        for (y = 0; y < k; y++)
        {
            //Check if the process has entered and is not complete
            if(processes[y]->cpu > 0 && processes[y]->enterTime <= x)
            {
                //If the process has just entered, malloc. startMemBlock starts at -1 and is actually assigned when the process starts
                if(processes[y]->startMemBlock == -1)
                {
                  if((memUsed + processes[y]->mem) < memSize)
                  {
                    //Malloc the required memory, add to the memory total, and assign startMemBlock
                    processes[y]->buffer = (bool*) malloc (processes[y]->mem);
                    memUsed += processes[y]->mem;
                    processes[y]->startMemBlock = x;
                  }
                }

                //If even one process ran, we need to not quit yet
                if(processes[y]->cpu > 0 && processes[y]->startMemBlock != -1)
                {
                  oneRan = true;
                  processes[y]->cpu--;
                }

                //When process CPU reaches 0, free the memory it used and subtract from the memory total
                if(processes[y]->cpu == 0)
                {
                  free(processes[y]->buffer);
                  memUsed -= processes[y]->mem;
                }
            }
        }

        //If no processes ran, free all processes and quit
        if(!oneRan)
        {
          int done = 1;

          //Loop through all processes to make sure they're all at 0 CPU
          for(z = 0; z < k; z++)
          {
            if(processes[z]->cpu != 0)
            {
              done = 0;
            }
          }

          //If all the processes are complete, end the timer and finish
          if(done == 1)
          {
            time_b = clock();
            cout<<"Final x value for system malloc: "<<x<<endl;
            cout<<"Total time for custom malloc: "<<time_b - time_a<< " cycles, or " << (time_b - time_a) / (double) CLOCKS_PER_SEC<< " seconds"<<endl;
            return x;
          }

        }
        x += amount; //Increment x (psuedo clock cycles) by 1
    }
    return x;
}

void my_malloc(bool *memArray, int processNum, Process *processes[])
{
    int i;
    int j;
    int contiguousSpace = 0;

    //Loop based on process memory requirement
    for(i = 0; i < memSize; i++)
    {
        //If the memory location is free
        if(memArray[i] == false)
        {
            contiguousSpace++;
            //Check if the amount of contiguous space found matches our required memory
            if(contiguousSpace == processes[processNum]->mem)
            {
                //Move back to the start of the contiguous block
                i -= processes[processNum]->mem;
                //Assign startMemBlock
                processes[processNum]->startMemBlock = ++i;

                j = i;

                //I set i = j to shut my compiler up
                //Assign the memory to true to mark it as unavailable
                for(i = j; i < j + processes[processNum]->mem; i++)
                {
                    memArray[i] = true;
                }
                return;
            }
        }
        //If the block wasn't big enough, reset the block tracker and keep looking
        else
        {
            contiguousSpace = 0;
        }
    }
}

void my_free(bool *memArray, int processNum, Process *processes[])
{
    int i;

    for(i = processes[processNum]->startMemBlock; i < processes[processNum]->startMemBlock + processes[processNum]->mem; i++)
    {
        memArray[i] = false;
    }
}

int runProcesses2(int x, int amount, int k, Process *processes[])
{
    int y;
    bool oneRan = false;
    int memUsed = 0;
    clock_t time_a;
    clock_t time_b;

    //Note: We are intentionally not timing the initial malloc call, as it seems
    //that we're only supposed to be timing my_malloc and my_free
    bool *memArray = (bool*)(malloc(memSize * sizeof(bool)));
    for(y = 0; y < memSize; y++)
    {
        memArray[y] = false;
    }

    time_a = clock();
    while(true)
    {
        oneRan = false;
        for (y = 0; y < k; y++)
        {
            //Check is process has entered and still has cpu time remaining
            if(processes[y]->cpu > 0 && processes[y]->enterTime <= x)
            {
                //If the process hasn't had memory allocated, try to allocate memory
                //We don't need to check if the process has entered, since that's already been checked
                if(processes[y]->startMemBlock == -1 && memUsed + processes[y]->mem <= memSize)
                {
                    my_malloc(memArray, y, processes);

                    //my_malloc isn't guaranteed to find space the first time, so we have to check if the process was actually given memory
                    if(processes[y]->startMemBlock != -1)
                    {
                        //Add to the used memory tracker
                        memUsed += processes[y]->mem;
                    }
                }

                //If even one process ran, we need to not quit yet
                if(processes[y]->startMemBlock != -1)
                {
                    oneRan = true;
                    //Decrement cpu time for running processes
                    processes[y]->cpu--;
                }

                //When processes finish, free the memory and subtract from used memory
                if(processes[y]->cpu == 0)
                {
                    my_free(memArray, y, processes);
                    memUsed -= processes[y]->mem;
                }
            }
        }

        //If no processes ran, free all processes and quit
        if(!oneRan)
        {
          int done = 1;
          int z;

          //Loop through the processes to make sure they all have 0 cpu. Otherwise, we need to keep going
          for(z = 0; z < k; z++)
          {
            if(processes[z]->cpu != 0)
            {
              done = 0;
            }
          }
          if(done == 1)
          {
            time_b = clock();
            cout<<"Final x value for custom malloc: "<<x<<endl;
            cout<<"Total time for custom malloc: "<<time_b - time_a<< " cycles, or " << (time_b - time_a) / (double) CLOCKS_PER_SEC<< " seconds"<<endl;

            return x;
          }
        }
        x += amount; //Increment x (psuedo cycle tracker)
    }
    return x;
}

int main()
{
        //x keeps track of number of pseudo clock cycles
        int x = 0;
        //Choice of memory amount to run
        int choice = 0;
        //Pick a new seed to be used for every scheduling method
        srand (time(NULL));
        int seed = rand();

        cout<<"Seed: "<<seed<<endl;

        Process *processes[k];

        //Main loop. Asks for input and runs using memory based on the selected option
        while(1)
        {
          cout<<"Type '1' for 20MB memory, '2' for 50% required, or '3' for 10% required. Type '-1' to quit."<<endl;
          cin>>choice;
          if(choice == 1)
          {
            memSize = 20000;
          }
          //Total required memory is always 1280KB because the 20KB average is guaranteed. 20KB * 64 = 1280KB
          else if (choice == 2)
          {
            memSize = 640;
          }
          else if (choice == 3)
          {
            memSize = 128;
          }
          else if (choice == -1)
          {
            return 0;
          }
          else
          {
            cout<<"Invalid selection"<<endl;
            return 0;
          }

          //Run system malloc
          x = 0;
          generateProcesses(k, processes, seed);
          x = runProcesses(x, 1, k, processes);

          //Reset x
          x = 0;
          //Run custom malloc
          generateProcesses(k, processes, seed);
          x = runProcesses2(x, 1, k, processes);
        }
        return 0;
}
