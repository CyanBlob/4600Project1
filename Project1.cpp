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
        for(z = 0; z < k; z++)
                cout<<"    PROCESS "<<z<<"  CPU: "<<processes[z]->cpu<<endl;

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
                        for(w = 0; w < processes[shortest]->cpu; w++)
                        {
                                x = incrementX(x, 1, k, processes);
                        }
                        processes[shortest]->running = false;
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
                                cout<<"    PROCESS "<<y<<" WAITTIME: "<<processes[y]->waitTime<<endl;
                        }
                        cout<<"TOTAL RUNTIME: "<<x<<" AVERAGE WAITTIME "<< totalWait / k<<" SWITCHES: "<<switches<<endl<<endl;
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

        for(y = 0; y < k; y++)
                cout<<"    PROCESS "<<y<<"  CPU: "<<processes[y]->cpu<<endl;

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

                        //cout<<"PROCESS "<<currentProcess<<"  CPU: "<<processes[currentProcess]->cpu<<endl;

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
                                                cout<<"    PROCESS "<<y<<" WAITTIME: "<<processes[y]->waitTime<<endl;
                                        }
                                        cout<<"TOTAL RUNTIME: "<<x<<" AVERAGE WAITTIME "<< totalWait / k<<" SWITCHES: "<<switches<<endl<<endl;
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
                        for (w = 0; w < quantum; w++)
                        {
                                x = incrementX(x, 1, k, processes);
                        }
                        processes[y]->running = false;
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

        for(y = 0; y < k; y++)
                cout<<"    PROCESS "<<y<<"  CPU: "<<processes[y]->cpu<<endl;

        while (true)
        {
                //Pick all processes to run
                for(y = 0; y < k; y++)
                {
                        w = (y + end) % k;
                        //cout<<"Pick processes "<<processCount<<" x: "<<x<<endl;
                        if(processCount < 4 && processes[w]->cpu > 0 && processes[w]->enterTime <= x)
                        {
                                processes[w]->running = true;
                                //cout<<w+1<<" ";
                                processCount++;
                                lastAdded = w;
                        }
                        if(y == k - 1 && processCount == 0)
                        {
                                for(y = 0; y < k; y++)
                                {
                                        totalWait += processes[y]->waitTime;
                                        cout<<"    PROCESS "<<y<<" WAITTIME: "<<processes[y]->waitTime<<endl;
                                }
                                cout<<"TOTAL RUNTIME: "<<x<<" AVERAGE WAITTIME "<< totalWait / k<<" SWITCHES: "<<switches<<endl<<endl;
                                return;
                        }
                }
                switches++;
                x = incrementX(x, contextSwitch, k, processes);

                for(y = 0; y < k; y++)
                {
                        processes[y]->wasRunning = false;
                }

                end = (lastAdded + 1) % k;
                //cout<<processCount<<endl;

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

                processCount = 0;
                for(y = 0; y < k; y++)
                {
                        if(processes[y]->running == true)
                        {
                                processes[y]->running = false;
                                processes[y]->wasRunning = true;
                        }
                }

                //Check if we're finished
                for (y = 0; y < k; y++)
                {
                        if(processes[y]->cpu != 0)
                                break;
                        else if(y == k - 1)
                        {
                                for(y = 0; y < k; y++)
                                {
                                        totalWait += processes[y]->waitTime;
                                        cout<<"    PROCESS "<<y<<" WAITTIME: "<<processes[y]->waitTime<<endl;
                                }
                                cout<<"TOTAL RUNTIME: "<<x<<" AVERAGE WAITTIME "<< totalWait / k<<" SWITCHES: "<<switches<<endl<<endl;
                                return;
                        }
                }
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

        for(y = 0; y < k; y++)
                cout<<"    PROCESS "<<y<<"  CPU: "<<processes[y]->cpu<<endl;

        //Loop through all processes
        for (y = 0; y < k; y++)
        {

                while(true)
                {
                        if(processes[y]->enterTime > x)
                        {
                                x = incrementX(x, 1, k, processes);
                        }
                        else
                        {
                                processes[y]->running = true;
                                for(w = 0; w < processes[y]->cpu; w++)
                                {
                                        x = incrementX(x, 1, k, processes);
                                }
                                processes[y]->running = false;
                                processes[y]->cpu = 0;
                                break;
                        }
                }
                if(y == k - 1)
                {
                        for(y = 0; y < k; y++)
                        {
                                totalWait += processes[y]->waitTime;
                                cout<<"    PROCESS "<<y<<" WAITTIME: "<<processes[y]->waitTime<<endl;
                        }
                        cout<<"TOTAL RUNTIME: "<<x<<" AVERAGE WAITTIME "<< totalWait / k<<" SWITCHES: "<<switches<<endl<<endl;
                        return;
                }
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

        for(y = 0; y < k; y++)
                cout<<"    PROCESS "<<y<<"  CPU: "<<processes[y]->cpu<<endl;

        //Loop through all processes
        while(true)
        {
                for (y = 0; y < k; y++)
                {
                        //cout<<"PROCESS COUNT = "<<processCount<<" PROCESS " << y << " CPU = " << processes[y]->cpu<<" PROCESS " << y << " RUNNING = " << processes[y]->running<<endl;
                        if(processCount < 4 && processes[y]->cpu > 0 && processes[y]->enterTime <= x && processes[y]->running == false)
                        {
                                //cout<<"THAT IF"<<endl;
                                contextCounter++;
                                if(contextCounter > 4)
                                {
                                        x = incrementX(x, contextSwitch, k, processes);
                                        switches++;
                                }
                                processCount++;
                                //processes[y]->running = true;
                                for(w = 0; w < k; w++)
                                {
                                        if(processes[w]->cpu >= processes[largest]->cpu)
                                        {
                                                largest = w;
                                        }
                                }
                                shortest = largest;
                                //cout<<"FOUND SHORTEST(LARGEST). LARGEST = "<< largest <<endl;

                                for(w = 0; w < k; w++)
                                {
                                        if(processes[w]->cpu <= processes[shortest]->cpu && processes[w]->cpu > 0 && processes[w]->enterTime <= x && processes[w]->running == false)
                                        {
                                                //cout<<"FINDING SHORTEST"<<endl;
                                                shortest = w;
                                                //processes[w]->running = true;
                                        }
                                }
                                //cout<<"FOUND SHORTEST. W = "<< w <<endl;
                                processes[shortest]->running = true;
                                //cout<<"AFTER TRUE"<<endl;
                        }
                }
                for (y = 0; y < k; y++)
                {
                        //cout<<"SECOND LOOP"<<endl;
                        if(processes[y]->running)
                        {
                                //cout<<"FIRST IF"<<endl;
                                processes[y]->cpu--;
                                if(processes[y]->cpu == 0)
                                {
                                        //cout<<"SECOND IF"<<endl;
                                        //cout<<"FINISHED PROCESS"<<endl;
                                        processCount--;
                                        processes[y]->running = false;
                                        break;
                                }

                        }
                }


                x = incrementX(x, 1, k, processes);


                for (y = 0; y < k; y++)
                {
                        if(processes[y]->cpu != 0)
                                break;
                        else if(y == k - 1)
                        {
                                for(y = 0; y < k; y++)
                                {
                                        totalWait += processes[y]->waitTime;
                                        cout<<"    PROCESS "<<y<<" WAITTIME: "<<processes[y]->waitTime<<endl;
                                }
                                cout<<"TOTAL RUNTIME: "<<x<<" AVERAGE WAITTIME "<< totalWait / k<<" SWITCHES: "<<switches<<endl<<endl;
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

        for(y = 0; y < k; y++)
                cout<<"    PROCESS "<<y<<"  CPU: "<<processes[y]->cpu<<endl;

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
                                        switches++;
                                }
                                processCount++;
                                processes[y]->running = true;
                        }

                        if(processes[y]->running)
                        {
                                processes[y]->cpu--;
                                if(processes[y]->cpu == 0)
                                {
                                        processCount--;
                                        processes[y]->running = false;
                                        break;
                                }

                        }
                }

                x = incrementX(x, 1, k, processes);


                for (y = 0; y < k; y++)
                {
                        if(processes[y]->cpu != 0)
                                break;
                        else if(y == k - 1)
                        {
                                for(y = 0; y < k; y++)
                                {
                                        totalWait += processes[y]->waitTime;
                                        cout<<"    PROCESS "<<y<<" WAITTIME: "<<processes[y]->waitTime<<endl;
                                }
                                cout<<"TOTAL RUNTIME: "<<x<<" AVERAGE WAITTIME "<< totalWait / k<<" SWITCHES: "<<switches<<endl<<endl;
                                return;
                        }
                }
        }

}

void generateProcesses(int k, Process *processes[])
{
        int x;

        for(x = 0; x < k; x++)
        {
                processes[x] = new Process();
                processes[x]->enterTime = x * 50;
                processes[x]->waitTime = 0;
                processes[x]->arrived = false;
                processes[x]->running = false;
                processes[x]->wasRunning = false;
        }

        fillPIDs(k, processes);
        fillMemValues(k, processes);
        fillCpuValues(k, processes);
}

int main()
{

        int k = 5;

        int quantum = 50;
        int contextSwitch = 10;

        Process *processes[k];

        generateProcesses(k, processes);
        roundRobin(k, processes, quantum, contextSwitch);

        generateProcesses(k, processes);
        fifo(k, processes, contextSwitch);

        generateProcesses(k, processes);
        SJF(k, processes, contextSwitch);

        generateProcesses(k, processes);
        fifoQuad(k, processes, contextSwitch);

        generateProcesses(k, processes);
        SJFQuad(k, processes, contextSwitch);

        generateProcesses(k, processes);
        roundRobinQuad(k, processes, quantum, contextSwitch);
        return 0;
}
