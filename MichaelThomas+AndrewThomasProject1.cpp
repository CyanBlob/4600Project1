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


void SJF(int k, Process *processes[], int contextSwitch)
{
        int w = 0, y = 0, z = 0, x = 0; //Loop counters
        int shortest = 0;
        int switches = 0;
        int totalWait = 0;

        cout<<"STARTING SJF"<<endl;

        while(true)
        {
                for(w = 0; w < k; w++)
                {
                        if(processes[shortest]->cpu == 0)
                                shortest = (shortest + 1) % k;
                }
                //Find the shortest process that has arrived
                for(z = 0; z < k; z++)
                {
                        if(processes[z]->cpu <= processes[shortest]->cpu && processes[z]->cpu > 0 && processes[z]->enterTime <= x)
                        {
                                shortest = z;
                        }
                }
                //Loop
                while(true)
                {
                        //Process is running and adding wait time to other processes that have arrived
                        processes[shortest]->running = true;

                        //Print process begin time
                        cout<<"PROCESS " << shortest;
                        if(shortest < 10)
                                cout << " ";
                        cout << " BEGINNING AT TIME " << x<<" TIME REMAINING: " << processes[shortest]->cpu<< " / " << processes[shortest]->cpuStart<<endl;

                        for(w = 0; w < processes[shortest]->cpu; w++)
                        {
                                x = incrementX(x, 1, k, processes);
                        }
                        processes[shortest]->running = false;

                        //Print process end time
                        cout<<"PROCESS " << shortest << "  ENDING AT TIME " << x<<endl;

                        processes[shortest]->cpu = 0;
                        //y++;
                        break;
                }
                //End on last process
                if(y == k - 1)
                {
                        for(y = 0; y < k; y++)
                        {
                                totalWait += processes[y]->waitTime;
                                cout<<"    PROCESS "<<y<<" CPU: " << processes[y]->cpuStart;
                                if(y < 10)
                                        cout << " ";
                                cout<<" ----- WAITTIME: "<<processes[y]->waitTime<<endl;
                        }
                        cout<<"TOTAL RUNTIME: "<<x<<" AVERAGE WAITTIME: "<< totalWait / k<<" SWITCH TIME: "<<switches * 10<<endl<<endl;
                        return;
                }
                else
                {
                        switches++;
                        x = incrementX(x, contextSwitch, k, processes);
                }
                y++;
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
        int totalWait = 0;

        cout<<"STARTING RR"<<endl;

        //Find next ready process
        while(true)
        {
                //If the current process is not ready to run
                if(processes[currentProcess]->enterTime > x || processes[currentProcess]->cpu == 0)
                {
                        if(0 == currentProcess)
                        {
                                //If we got through all processes and nothing was able to run, stall
                                if(oneRan == false)
                                {

                                        x = incrementX(x, 1, k, processes);
                                }
                                oneRan = false;
                        }

                        //Look at the next process
                        currentProcess = (currentProcess + 1) % k;
                }

                else
                {
                        //If the current process has time remaining, subtract the quantum
                        if(processes[currentProcess]->cpu != 0)
                        {
                                processes[currentProcess]->cpu -= quantum;
                        }

                        //Don't allow negative time remaining
                        if (processes[currentProcess]->cpu <= 0)
                        {
                                processes[currentProcess]->cpu = 0;

                        }

                        //Check if there are no CPU processes
                        for (y = 0; y < k; y++)
                        {
                                if(processes[y]->cpu != 0)
                                {
                                        break;
                                }
                                //If all processes have 0 cpu time left, end
                                if(y == k - 1)
                                {
                                        for(y = 0; y < k; y++)
                                        {
                                                totalWait += processes[y]->waitTime;
                                                cout<<"    PROCESS "<<y<<" CPU: " << processes[y]->cpuStart;
                                                if(y < 10)
                                                        cout << " ";
                                                cout<<" ----- WAITTIME: "<<processes[y]->waitTime<<endl;
                                        }
                                        cout<<"TOTAL RUNTIME: "<<x<<" AVERAGE WAITTIME: "<< totalWait / k<<" SWITCH TIME: "<<switches * 10<<endl<<endl;
                                        return;
                                }
                        }

                        //If we switched processes, add a context switch
                        if(lastProcess != currentProcess)
                        {
                                switches++;
                                x = incrementX(x, contextSwitch, k, processes);
                                //cout<<"SWITCHING "<<x<<endl;
                        }

                        //Keep track of the last process to run, and mark that one ran this iteration
                        lastProcess = currentProcess;
                        oneRan = true;

                        //Add wait time to all processes that aren't running
                        processes[y]->running = true;

                        //Print process start time
                        cout<<"PROCESS " << currentProcess;
                        if(y < 10)
                                cout << " ";
                        cout << " BEGINNING AT TIME " << x<<" TIME REMAINING: " << processes[currentProcess]->cpu<< " / " << processes[currentProcess]->cpuStart<<endl;

                        for (w = 0; w < quantum; w++)
                        {
                                x = incrementX(x, 1, k, processes);
                        }
                        processes[y]->running = false;

                        //Print process end time
                        if(processes[currentProcess]->cpu <= 0)
                        {
                                cout<<"        PROCESS " << currentProcess;
                                if(y < 10)
                                        cout << " ";
                                cout << " COMPLETING AT TIME " << x<<endl;
                        }

                        currentProcess = (currentProcess + 1) % k;
                }
        }
}

void roundRobinQuad(int k, Process *processes[], int quantum, int contextSwitch)
{
        int w;
        int y;
        int x = 0;
        int end = 0;
        int switches = 0;
        int processCount = 0;
        int lastAdded = 0;
        int totalWait = 0;

        cout<<"STARTING RR - QUAD"<<endl;

        while (true)
        {
                //Pick all processes to run
                for(y = 0; y < k; y++)
                {
                        //Change focus to the group of 4 after the last process we chose last time
                        w = (y + end) % k;

                        if(processCount < 4 && processes[w]->cpu > 0 && processes[w]->enterTime <= x)
                        {
                                processes[w]->running = true;

                                //Print process start time
                                cout<<"PROCESS " << w;
                                if(y < 10)
                                        cout << " ";
                                cout << " BEGINNING AT TIME " << x<<" TIME REMAINING: " << processes[w]->cpu<< " / " << processes[w]->cpuStart<<endl;

                                processCount++;
                                lastAdded = w;
                        }

                        //Check if we're done
                        if(y == k - 1 && processCount == 0)
                        {
                                for(y = 0; y < k; y++)
                                {
                                        totalWait += processes[y]->waitTime;
                                        cout<<"    PROCESS "<<y<<" CPU: " << processes[y]->cpuStart;
                                        if(y < 10)
                                                cout << " ";
                                        cout<<" ----- WAITTIME: "<<processes[y]->waitTime<<endl;
                                }
                                cout<<"TOTAL RUNTIME: "<<x<<" AVERAGE WAITTIME: "<< totalWait / k<<" SWITCH TIME: "<<switches * 10<<endl<<endl;
                                return;
                        }
                }

                //Add one switch to the total per active processor
                switches+= processCount;
                x = incrementX(x, contextSwitch, k, processes);

                //Keep track of where we left off
                end = (lastAdded + 1) % k;

                //Run all chosen processes
                for(y = 0; y < k; y++)
                {
                        if(processes[y]->running)
                        {
                                processes[y]->cpu -= 50;
                        }
                }

                //Add wait time
                for(w = 0; w < 50; w++)
                {
                        x = incrementX(x, 1, k, processes);
                }

                //Reset the process counter, and keep track of all processes that ran
                processCount = 0;
                for(y = 0; y < k; y++)
                {
                        if(processes[y]->running == true)
                        {
                                processes[y]->running = false;

                                if(processes[y]->cpu <= 0)
                                {

                                        //Print process end time
                                        cout<<"        PROCESS " << y;
                                        if(y < 10)
                                                cout << " ";
                                        cout << " COMPLETING AT TIME " << x<<endl;
                                }
                        }
                }
                cout<<endl;
        }


}

void fifo(int k, Process *processes[], int contextSwitch)
{
        int w;
        int y;
        int x = 0;
        int switches = 0;
        int totalWait = 0;

        cout<<"STARTING FIFO"<<endl;

        //Loop through all processes
        for (y = 0; y < k; y++)
        {

                while(true)
                {
                        //If the current process is not ready to run, stall
                        if(processes[y]->enterTime > x)
                        {
                                x = incrementX(x, 1, k, processes);
                        }

                        //Run the process to completion, adding wait time to waiting processes
                        else
                        {
                                processes[y]->running = true;

                                //Print process begin time
                                cout<<"PROCESS " << y;
                                if(y < 10)
                                        cout << " ";
                                cout << " BEGINNING AT TIME " << x<<" TIME REMAINING: " << processes[y]->cpu<< " / " << processes[y]->cpuStart<<endl;

                                for(w = 0; w < processes[y]->cpu; w++)
                                {
                                        x = incrementX(x, 1, k, processes);
                                }
                                processes[y]->running = false;

                                //Print process end time
                                cout<<"PROCESS " << y;
                                if(y < 10)
                                        cout << " ";
                                cout << " ENDING AT TIME " << x<<endl;


                                processes[y]->cpu = 0;
                                break;
                        }
                }

                //Check if we're done
                if(y == k - 1)
                {
                        for(y = 0; y < k; y++)
                        {
                                totalWait += processes[y]->waitTime;
                                cout<<"    PROCESS "<<y<<" CPU: " << processes[y]->cpuStart;
                                if(y < 10)
                                        cout << " ";
                                cout<<" ----- WAITTIME: "<<processes[y]->waitTime<<endl;
                        }
                        cout<<"TOTAL RUNTIME: "<<x<<" AVERAGE WAITTIME: "<< totalWait / k<<" SWITCH TIME: "<<switches * 10<<endl<<endl;
                        return;
                }

                //Add a context switch
                else
                {
                        switches++;
                        x = incrementX(x, contextSwitch, k, processes);
                }
        }
}

void SJFQuad(int k, Process *processes[], int contextSwitch)
{
        int y;
        int x = 0;
        int w = 0;
        int shortest = 0;
        int largest = 0;
        int switches = 0;
        int processCount = 0;
        int contextCounter = 0;
        int totalWait = 0;

        cout<<"STARTING SJF - QUAD"<<endl;

        //Loop through all processes
        while(true)
        {
                for (y = 0; y < k; y++)
                {
                        if(processCount < 4 && processes[y]->cpu > 0 && processes[y]->enterTime <= x && processes[y]->running == false)
                        {
                                contextCounter++;
                                if(contextCounter > 4)
                                {
                                        x = incrementX(x, contextSwitch, k, processes);
                                        //Add one switch per active processor
                                        switches+= processCount;
                                }
                                processCount++;

                                //Find the largest process so that we have a good starting point for finding the smallest remaining process
                                for(w = 0; w < k; w++)
                                {
                                        if(processes[w]->cpu >= processes[largest]->cpu)
                                        {
                                                largest = w;
                                        }
                                }
                                shortest = largest;

                                //Find the smallest remaining process
                                for(w = 0; w < k; w++)
                                {
                                        if(processes[w]->cpu <= processes[shortest]->cpu && processes[w]->cpu > 0 && processes[w]->enterTime <= x && processes[w]->running == false)
                                        {
                                                shortest = w;
                                        }
                                }

                                processes[shortest]->running = true;

                                //Print process begin time
                                cout<<"PROCESS " << shortest;
                                if(shortest < 10)
                                        cout << " ";
                                cout << " BEGINNING AT TIME " << x<<" TIME REMAINING: " << processes[shortest]->cpu<< " / " << processes[shortest]->cpuStart<<endl;
                        }
                }

                //Progress the current processes, removing them when they're done
                for (y = 0; y < k; y++)
                {
                        if(processes[y]->running)
                        {
                                processes[y]->cpu--;
                                if(processes[y]->cpu == 0)
                                {
                                        processCount--;
                                        processes[y]->running = false;

                                        //Print process end time
                                        cout<<"PROCESS " << y;
                                        if(y < 10)
                                                cout << " ";
                                        cout << " ENDING AT TIME " << x<<endl;

                                        break;
                                }

                        }
                }

                //Increment x and add wait time to all waiting processes
                x = incrementX(x, 1, k, processes);


                //Check if we're done
                for (y = 0; y < k; y++)
                {
                        if(processes[y]->cpu != 0)
                                break;
                        else if(y == k - 1)
                        {
                                for(y = 0; y < k; y++)
                                {
                                        totalWait += processes[y]->waitTime;
                                        cout<<"    PROCESS "<<y<<" CPU: " << processes[y]->cpuStart;
                                        if(y < 10)
                                                cout << " ";
                                        cout<<" ----- WAITTIME: "<<processes[y]->waitTime<<endl;
                                }
                                cout<<"TOTAL RUNTIME: "<<x<<" AVERAGE WAITTIME: "<< totalWait / k<<" SWITCH TIME: "<<switches * 10<<endl<<endl;
                                return;
                        }
                }
        }

}
void fifoQuad(int k, Process *processes[], int contextSwitch)
{
        int y;
        int x = 0;
        int switches = 0;
        int processCount = 0;
        int contextCounter = 0;
        int totalWait = 0;

        cout<<"STARTING FIFO - QUAD"<<endl;


        //Loop through all processes
        while(true)
        {
                for (y = 0; y < k; y++)
                {
                        //Pick our processes. Find a new process every time there is one available and there is a CPU core available
                        if(processCount < 4 && processes[y]->cpu > 0 && processes[y]->enterTime <= x && processes[y]->running == false)
                        {
                                //Add context switches every time we add a new process, excluding the first 4
                                contextCounter++;
                                if(contextCounter > 4)
                                {
                                        x = incrementX(x, contextSwitch, k, processes);
                                        //Add one switch per active processor
                                        switches+= processCount;
                                }
                                processCount++;
                                processes[y]->running = true;

                                //Print process begin time
                                cout<<"PROCESS " << y;
                                if(y < 10)
                                        cout << " ";
                                cout << " BEGINNING AT TIME " << x<<" TIME REMAINING: " << processes[y]->cpu<< " / " << processes[y]->cpuStart<<endl;

                        }

                        //Progress the running processes, removing them when they're done
                        if(processes[y]->running)
                        {
                                processes[y]->cpu--;
                                if(processes[y]->cpu == 0)
                                {
                                        processCount--;
                                        processes[y]->running = false;

                                        //Print process end time
                                        cout<<"PROCESS " << y;
                                        if(y < 10)
                                                cout << " ";
                                        cout << " ENDING AT TIME " << x<<endl;

                                        break;
                                }

                        }
                }

                //Increment x and add wait time to all waiting processes
                x = incrementX(x, 1, k, processes);

                //Check if we're done
                for (y = 0; y < k; y++)
                {
                        if(processes[y]->cpu != 0)
                                break;
                        else if(y == k - 1)
                        {
                                for(y = 0; y < k; y++)
                                {
                                        totalWait += processes[y]->waitTime;
                                        cout<<"    PROCESS "<<y<<" CPU: " << processes[y]->cpuStart;
                                        if(y < 10)
                                                cout << " ";
                                        cout<<" ----- WAITTIME: "<<processes[y]->waitTime<<endl;
                                }
                                cout<<"TOTAL RUNTIME: "<<x<<" AVERAGE WAITTIME: "<< totalWait / k<<" SWITCH TIME: "<<switches * 10<<endl<<endl;
                                return;
                        }
                }
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

        int k = 50;

        int quantum = 50;
        int contextSwitch = 10;

        //Pick a new seed to be used for every scheduling method
        srand (time(NULL));
        int seed = rand();

        cout<<"SEED: "<<seed<<endl;


        Process *processes[k];

        //Run all scheduling method, recreating the processes every time

        generateProcesses(k, processes, seed);
        roundRobin(k, processes, quantum, contextSwitch);

        generateProcesses(k, processes, seed);
        fifo(k, processes, contextSwitch);

        generateProcesses(k, processes, seed);
        SJF(k, processes, contextSwitch);

        generateProcesses(k, processes, seed);
        fifoQuad(k, processes, contextSwitch);

        generateProcesses(k, processes, seed);
        SJFQuad(k, processes, contextSwitch);

        generateProcesses(k, processes, seed);
        roundRobinQuad(k, processes, quantum, contextSwitch);
        return 0;
}
