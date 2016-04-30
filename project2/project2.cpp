#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <malloc.h>
#include "Process.h"

using namespace std;

int k = 64;

void sysMalloc(Process *processes[])
{
  int x;
  int n;
  //char * buffer;

  for (x = 0; x < k; x++)
  {
          //cout<<processes[x]->mem<<endl;

          processes[x]->buffer = (char*) malloc (processes[x]->mem+1);

          if (processes[x]->buffer==NULL)
            exit (1);

          for (n=0; n<processes[x]->mem; n++)
            processes[x]->buffer[n]=rand()%26+'a';
          processes[x]->buffer[processes[x]->mem]='\0';

          //free (processes[x]->buffer);
  }
}

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

//Increment x and decrement cpu from running processes
//Also frees processes once every processes is finished
int runProcesses(int x, int amount, int k, Process *processes[])
{
        int y;
        bool oneRan = false;

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

                         oneRan = true;
                         processes[y]->cpu--;
                 }

            }
            if(!oneRan)
            {

                countBuffers(k, processes);
                for (y = 0; y < k; y++)
                {
                        free(processes[y]->buffer);
                }
                cout<<"Final x value: "<<x<<endl;
                return x;
            }
            x += amount;
        }
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

        //while(true);
        return 0;
}
