Compile using the command: clear && g++ MichaelThomas+AndrewThomasProject1.cpp && ./a.out > "MichaelThomas+AndrewThomasOutput.txt"

1. This program generates A LOT of output (14,000 lines). I would HIGHLY recommend that you pipe the output to a text file, as I have done in the compile command above.

2. Navigate the file by searching for "STARTING xxx" where xxx is either RR, FIFO, SJF, RR - QUAD, FIFO - QUAD, SJF - QUAD

3. For Round Robin on 4 cores, we assumed that when a process is ran on two quantums back to back, that the scheduler is not able to keep it on the same core that it was on previously. This means that there is a context switch even if the same process is ran twice in a row. This is only relevant when there are 8 or less active processes, so it doesn't make a significant difference in this context.

4. The seed for random process generation is randomized as well, meaning that you will get a new set of processes every time the program is ran.

