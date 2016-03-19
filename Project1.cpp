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
        srand(1);

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
        srand(1);

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


void SJF(int k, Process *processes[], int contextSwitch)
{
  int w = 0, y = 0, z = 0, x = 0; //Loop counters
  int currentProcess = 0;
  int shortest = 0;

  cout<<"STARTING SJF"<<endl;

  for(z = 0; z < k; z++)
          cout<<"    PROCESS "<<z<<"  CPU: "<<processes[z]->cpu<<endl;
while(true)
{
          //Find the shortest process that has arrived
          for(z = 0; z < (x / 50); z++)
          {
              if(processes[z]->cpu > processes[shortest]->cpu && processes[z]->cpu > 0)
                shortest = z;
          }
                  cout<<"BEFORE WHILE. y = " << y << endl;
                  //Loop
                  while(true)
                  {
                          //Stall if process hasn't arrived
                          /*if(processes[shortest]->enterTime > x)
                          {
                                  x = incrementX(x, 1, k, processes, shortest);
                          }*/
                          //Do stuff only if process has arrived
                          //else
                          {
                                  //Process is running and adding wait time to other processes that have arrived
                                  for(w = 0; w < processes[shortest]->cpu; w++)
                                  {
                                          x = incrementX(x, 1, k, processes, shortest);
                                  }
                                  cout<<"BEFORE BREAK. y = "<<y<<endl;
                                  processes[shortest]->cpu = 0;
                                  y++;
                                  break;
                          }
                  }
                  cout<<"BEFORE PRINT. y = " << y << endl;
                  //End on last process
                  if(y == k - 1)
                  {
                          for(y = 0; y < k; y++)
                                  cout<<"    PROCESS "<<y<<" WAITTIME: "<<processes[y]->waitTime<<endl;
                          cout<<"TOTAL RUNTIME: "<<x<<endl;
                          return;
                  }
                  else
                  {
                      x = incrementX(x, contextSwitch, k, processes, y);
                  }
      }
}

void roundRobin(int k, Process *processes[], int quantum, int contextSwitch)
{
        int x = 0;
        int y = 0;
        int w;
        int currentProcess = 0;
        int lastProcess = 0;
        int switches = 0;
        bool oneRan = false;

        cout<<"STARTING RR"<<endl;

        for(y = 0; y < k; y++)
                cout<<"    PROCESS "<<y<<"  CPU: "<<processes[y]->cpu<<endl;

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
                                        //cout<<"No process ready "<<x<<endl;
                                        //x += 50;
                                        for (w = 0; w < quantum; w++)
                                            x = incrementX(x, 1, k, processes, currentProcess);
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
                                processes[currentProcess]->cpu -= quantum;

                        if (processes[currentProcess]->cpu <= 0)
                        {
                                processes[currentProcess]->cpu = 0;

                        }

                        //cout<<"PROCESS "<<currentProcess<<"  CPU: "<<processes[currentProcess]->cpu<<endl;

                        //Check if there are no CPU processes
                        for (y = 0; y < k; y++)
                        {
                                if(processes[y]->cpu != 0)
                                {
                                        break;
                                }
                                if(y == k - 1)
                                {
                                        for(y = 0; y < k; y++)
                                                cout<<"    PROCESS "<<y<<" WAITTIME: "<<processes[y]->waitTime<<endl;
                                        cout<<"TOTAL RUNTIME: "<<x  <<" SWITCHES: "<<switches<<endl;
                                        return;
                                }
                        }

                        if(lastProcess != currentProcess)
                        {
                                switches++;
                                x = incrementX(x, contextSwitch, k, processes, currentProcess);
                                //cout<<"SWITCHING "<<x<<endl;
                        }

                        lastProcess = currentProcess;
                        oneRan = true;

                        for (w = 0; w < quantum; w++)
                            x = incrementX(x, 1, k, processes, currentProcess);
                        currentProcess = (currentProcess + 1) % k;
                }
        }

}

void fifo(int k, Process *processes[], int contextSwitch)
{
        int w;
        int y;
        int x = 0;

        cout<<"STARTING FIFO"<<endl;

        for(y = 0; y < k; y++)
                cout<<"    PROCESS "<<y<<"  CPU: "<<processes[y]->cpu<<endl;

        //Loop through all processes
        for (y = 0; y < k; y++)
        {

                while(true)
                {
                        if(processes[y]->enterTime > x)
                        {
                                x = incrementX(x, 1, k, processes, y);
                        }
                        else
                        {
                                for(w = 0; w < processes[y]->cpu; w++)
                                {
                                        x = incrementX(x, 1, k, processes, y);
                                }
                                processes[y]->cpu = 0;
                                break;
                        }
                }
                if(y == k - 1)
                {
                        for(y = 0; y < k; y++)
                                cout<<"    PROCESS "<<y<<" WAITTIME: "<<processes[y]->waitTime<<endl;
                        cout<<"TOTAL RUNTIME: "<<x<<endl;
                        return;
                }
                else
                {
                    x = incrementX(x, contextSwitch, k, processes, y);
                }
        }
}

int main()
{

        int k = 5;
        int x;

        int quantum = 50;
        int contextSwitch = 10;

        Process *processes[k];

        for(x = 0; x < k; x++)
        {
                processes[x] = new Process();
                processes[x]->enterTime = x * 50;
                processes[x]->waitTime = 0;
                processes[x]->arrived = false;
        }

        fillPIDs(k, processes);
        fillMemValues(k, processes);
        fillCpuValues(k, processes);

        roundRobin(k, processes, quantum, contextSwitch);

        for(x = 0; x < k; x++)
        {
                processes[x] = new Process();
                processes[x]->enterTime = x * 50;
                processes[x]->waitTime = 0;
                processes[x]->arrived = false;
        }

        fillPIDs(k, processes);
        fillMemValues(k, processes);
        fillCpuValues(k, processes);

        fifo(k, processes, contextSwitch);

        for(x = 0; x < k; x++)
        {
                processes[x] = new Process();
                processes[x]->enterTime = x * 50;
                processes[x]->waitTime = 0;
                processes[x]->arrived = false;
        }

        fillPIDs(k, processes);
        fillMemValues(k, processes);
        fillCpuValues(k, processes);
        SJF(k, processes, contextSwitch);
        return 0;
}
