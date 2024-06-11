# **System Monitoring Tool -- Concurrency & Signals**

# **How I Solve the Question**
I first watched the video demo and compared the output in the provided examples, then I read through
the provided resource such as files and C-libraries to figure out the usage of each. Also, I searched up online to find
out more library and functions I can use. Therefore I started working on the program. 

Also, I used pipe for communcation between child process and parent process. In the child process, I decide to call the function to get the desired information, and write to the pipe.
Then I read the pipe in the parent process. 

In addition, we ignore the signal ctrl_z and for ctrl_c we dicided to stop the process, so I write two handler to solve this problem. 


# **Overview of functions**

## generate_header
```
void generate_header(int samples, int tdelay, bool sequential, int curr_line)
```
this function generate the header of the printed information according to the sequential, including the number of samples and the time of tdelay
if sequential, the print the number of iteration based on curr_line

Parameters
- `samples (int)`: The total number of samples specified for the monitoring process.
- `tdelay (int)`: The time delay (in seconds) between consecutive samples.
- `sequential (bool)`: A boolean flag indicating whether samples should be displayed sequentially.
- `curr_line (int)`: The current line or iteration number for which the header is generated.
# 

## generate_memory_usage
```
void generate_memory_usage()
```
this function generate the information of the memory usage using <sys/resourse.h>

#
## calculate_memory_info
```
double calculate_memory_info(char memory_info[][1024], int curr_line)
```
this function calculate the memory information, store the information in the array memory_info at the index curr_line
and return virtue used memory for future use

Parameters
- `memory_info (char[][1024])`: An array of character arrays to store memory-related information.
- `curr_line (int)`: The current line or iteration number for which memory information is being calculated and stored.
#

## generate_memory_graphic
```
void generate_memory_graphic(int curr_line, double *last_memory, double now_memory, char memory_info[][1024])
```
this function generate the memory graph, based on the memory before delay and after the delay
and store the information in the memory_info based on the index curr_line

Parameters

- `curr_line (int)`: The current line or iteration number for which memory graphic information is being generated.
- `last_memory (double*)`: A pointer to the variable storing the memory usage from the last iteration.
- `now_memory (double)`: The memory usage for the current iteration.
- `memory_info (char[][1024])`: An array of character arrays to store memory-related graphic and data information.
#

## read_cpu_stat_return_sum
```
long read_cpu_stat_return_sum()
```
this function read the /proc/stat file and return the sum of fields
#

## read_cpu_stat_return_idle
```
long read_cpu_stat_return_idle()
```
this function read the/proc/stat/ file, return the idle time of the cpu
#

## generate_cpu
```
double generate_cpu(long cpu_now_idle, long cpu_now_sum, long cpu_last_idle, long cpu_last_sum)
```
this function calculate the usage of the cpu using the cpu idle time and sum time before the delay and after the delay

Parameters
- `cpu_now_idle (long)`: The current CPU idle time.
- `cpu_now_sum (long)`: The current sum of CPU-related statistics.
- `cpu_last_idle (long)`: The previous CPU idle time.
- `cpu_last_sum (long)`: The previous sum of CPU-related statistics.
#

## generate_cpu_graphic
```
void generate_cpu_graphic(int samples, double now_cpu, char cpu_info[][1024], int curr_line, bool sequential)
```
this function generate the graphic of cpu information, store the graph in the cpu_info array in the index curr_line, based on the cpu current usage
and print the overall graph based on sequential

Parameters
- `samples (int)`: The total number of samples.
- `now_cpu (double)`: The current CPU usage percentage.
- `cpu_info([][1024])`: A 2D array to store CPU information and graphics.
- `curr_line (int)`: The index of the current line in the array.
- `sequential (bool)`: A boolean flag indicating sequential printing.

## generate_memory_info
#
```
void generate_memory_info(int samples, char memory_info[][1024], int curr_line, bool sequential)
```
this function generate the memory_info before samples based on if user eneter --sequential or not

Parameters
- `samples (int)`: The total number of samples.
- `memory_info (char[][1024]`: A 2D array storing memory information.
- `curr_line (int)`: The index of the current line in the array.
- `sequential (bool)`: A boolean flag indicating sequential printing.
#

## generate_user
```
void generate_user(int write_fd)
```
this function generates all the users' information using utmp.h and write to write_fd

Parameters
- `write_fd (int)`: The location write the user infomation
#

## generate_cores
```
void generate_cores()
```
this function generates the number of cores using sysconf
#

## generate_system_infomation
```
void generate_system_information()
```
this function generates infomation of the system
#

## check_valid_integer
```
bool check_valid_integer(char *input)
```
this function checks if the user input is valid integer

Parameters
- `input (char*)` : A pointer to the input string to be validated.
#

## get_memory_child_pid
```
int get_memory_child_pid(pid_t pid){
```
store the pid value of the memory child process

Parameters
- `pid (pid_t)` : the pid of the memory child process hat need to be killed
#

## get_user_child_pid
```
int get_user_child_pid(pid_t pid){
```
store the pid value of the user child process

Parameters
- `pid (pid_t)` : the pid of the user child process hat need to be killed
#

## get_cpu_child_pid
```
int get_cpu_child_pid(pid_t pid){
```
store the pid value of the cpu child process

Parameters
- `pid (pid_t)` : the pid of the cpu child process hat need to be killed
#

## ctrl_c_handler_for_child
```
void ctrl_c_handler_for_child(int sig){
```
handler the situation that if the user enter ctrl_c in child process

Parameters
- `pid (pid_t)` : the pid of the cpu child process hat need to be killed
#

## ctrl_c_handler
```
void ctrl_c_handler(int sig){
```
handler the situation that if the user enter ctrl_c in parent process

Parameters
- `sig (int)` 
#

## ctrl_v_handler
```
handler the situation that if the user enter ctrl_v
```

Parameters
- `sig (int)` 
#

## main
```
main(int argc, char **argv)
```
this is the main function, display memory information, user information, cpu usage, machine information
it has two parameters, argc is the number of arguments and argc is the array of arrguments. 
It parse the arguement and print the desired output

Parameters
- `argc (int)` : Number of command-line arguments
- `argv (char**)` : Array of command-line arguments. 
#
# **How to Use**

Use the makefile to compile the program. 
1. Open terminal or command prompt
2. Open the directory where the file is saved
3. Type `make`. 
4. run the progtam using `./a3`
5. type `make clean` to remove object file
   
### --system
to indicate that only the system usage should be generated
```
./a3 --system
```

### --user
to indicate that only the users usage should be generated
```
./a3 --user
```

### --graphic
 to include graphical output in the cases where a graphical outcome is possible as indicated below.
```
./a3 --graphic
```
or
```
./a3 -g
```
#### Graphical representations
for Memory utilization:
````
::::::@  total relative negative change
######*  total relative positive change

(OPTIONAL)
|o    zero+
|@    zero-
````

for CPU utilization:
````
||||   positive percentage increase
````

### --sequential
to indicate that the information will be output sequentially without needing to "refresh" the screen 
(useful if you would like to redirect the output into a file)
```
./a3 --sequential
```

### --samples=N
if used the value N will indicate how many times the statistics are going to be collected and results will be average and reported based on the N number of repetitions.
If not value is indicated the default value will be 10.
```
./a3 --samples=N
```

### --tdelay=T
to indicate how frequently to sample in seconds.
If not value is indicated the default value will be 1 sec.
```
./a3 --tdelays=T
```
### other way for setting samples and tdelay
make sure the order is correct, the first number is samples and the second is tdelay
```
./a3 8 2
```
# **Design Decison**

## sampling for CPUs match tdelay
```
collecting the cpu usage before the delay and after the delay
comparing two values and calculate the cpu usage. For iteration 1, we just delay in the child to calculate the tdelay time cpu use. 
```
## sequential
```
printing the current memory and cpu usage one by one at the index of the iteration
the other lines are blank
```

# test
