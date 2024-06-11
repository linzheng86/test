#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <utmp.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
# define MAX_LENGTH 2048

/** this function generates and prints the header information based on user input and iteration.
 * @param samples The number of samples specified by the user.
 * @param tdelay The time delay between samples specified by the user (in seconds).
 * @param sequential Flag indicating if sequential mode is enabled (true) or not (false).
 * @param curr_line The current iteration line number.
 */
void generate_header(int samples, int tdelay, bool sequential, int curr_line){
    if(sequential){ // if user enters sequential, then true
        printf(">>> iteration %d\n", curr_line); // print the number of the iteration
    }
    else{ //if user doesn't enter sequential, false
        printf("\033[H\033[2J"); // clear screan
        printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay); // print the number of samples and tdelay
    }
}

/** this function generate the information of the memory usage using <sys/resourse.h>
 */
void generate_memory_usage(){
    struct rusage memory_usage; // declear the struct using <sys/resource.h>
    getrusage(RUSAGE_SELF, &memory_usage); // get memory usgae
    printf("Memory usage: %ld kilobytes\n",memory_usage.ru_maxrss); // generate the memory usage using the struct
}

/**
 * this function calculates memory information and stores it in the specified array.
 * 
 * @param memory_info   Array to store the calculated memory information.
 * @param curr_line     Index of the current line in the array.
 * @return              The calculated virtual memory used.
 */
double calculate_memory_info(char memory_info[][1024], int curr_line){
    struct sysinfo memory_stat; // declear the struct using <sys/sysinfo.h>
    sysinfo(&memory_stat); // get the memory information
    // convert KB to GB by divide by 10^9
    double totalram = (float)memory_stat.totalram / 1000 / 1000 / 1000; 
    double freeram = (float)memory_stat.freeram / 1000 / 1000 / 1000;
    double totalswap = (float)memory_stat.totalswap / 1000 / 1000 / 1000;
    double freeswap = (float)memory_stat.freeswap /  1000 / 1000 / 1000;

    //calculate the desired information
    double phys_used = totalram - freeram;
    double virt_used = phys_used + totalswap - freeswap;
    double virt_total = totalram + totalswap;

    //store the information in the array according to the current time wanted
    sprintf(memory_info[curr_line], "%.2f GB / %.2f GB -- %.2f GB / %.2f GB", phys_used, totalram, virt_used, virt_total);  
    return virt_used;  
}

/**
 * this function generates a memory graphic based on the difference between current and last memory usage.
 * The graphic consists of bars and characters representing the memory change.
 * 
 * @param curr_line     Index of the current line in the array.
 * @param last_memory   Pointer to the last recorded memory usage.
 * @param now_memory    Current memory usage.
 * @param memory_info   Array to store the memory graphic information.
 */
 void generate_memory_graphic(int curr_line, double *last_memory, double now_memory, char memory_info[][1024]){
    float diff = 0; // set the default diff between current and last time to 0;
    char graphic_info[1024] = "\0";    // declear the array for storing graphic information 
    char data_info[1024] = "\0";    // declear the arry for storing the data information
    float rounded_diff;    // store the rounded information to two decimal place
    int bar_num = 0;
    char rounded_string[1024];
    strcpy(graphic_info, "    |");  

    if(curr_line == 0){
        diff = 0.00;   // for the first, diff is 0 by default
        rounded_diff = 0.00;    // rounded diff is also 0.00
    }
    else{
        diff = now_memory - *last_memory;   // the diff is calculated by the difference betwee now memory usage and the last time memoru usage
        sprintf(rounded_string, "%.2f", diff);  
        rounded_diff = atof(rounded_string);     // round the diff to two decimal place
    }
    

    bar_num = (int)(diff * 100);  // get the number of bar that without rounding
    if(rounded_diff == 0.00 && diff >= 0.00){     // zero+ : if the difference is greater than 0 but the rounded is 0.00
        strcat(graphic_info, "o "); 
    }
    else if(rounded_diff == 0.00 && diff < 0.00 ){  // zero- : if the difference is smaller than 0 but the rounded is 0.00
        strcat(graphic_info, "@ ");
    }
    else if(rounded_diff > 0){      // relative postive change
        for(float i = 0; i < bar_num; i++){
            strcat(graphic_info, "#");
        }
        strcat(graphic_info, "* ");
    }
    else if(rounded_diff < 0){     // relative negative change
        for(float i = 0; i < -bar_num; i++){
            strcat(graphic_info, ":");
        }
        strcat(graphic_info, "@ ");
    }
    *last_memory = now_memory;      // store the now memory usage as the last memory usage for next time usage
    sprintf(data_info, " %.2f (%.2f)", diff, now_memory);  // store the format
    strcat(graphic_info, data_info);
    strcat(memory_info[curr_line], graphic_info);   // store the information into the array
}

/**
 * this function reads the CPU statistics from the "/proc/stat" file and returns the sum of CPU usage fields.
 *
 * @return The sum of CPU usage fields.
 */
 long read_cpu_stat_return_sum(){
    FILE *file = fopen("/proc/stat", "r"); // open the file
    if(file == NULL){    //checking if successfully open the file
        fprintf(stderr, "Error opening file\n");  // if failed,  print the error
        exit(1);    // exit the program
    }
    char filecontent[1024];     // declear the array for storing file content
    fgets(filecontent, 1024, file); // copy the content in the file to the char array
    fclose(file);  // close the file
    
    char *token = strtok(filecontent, " "); // the first is cpu
    token = strtok(NULL, " "); // move to user
    long cpu_user = atoi(token); // store user
    token = strtok(NULL, " "); // move to nice
    long cpu_nice = atoi(token); // store nice
    token = strtok(NULL, " "); // move to system
    long cpu_system = atoi(token); // store system
    token = strtok(NULL, " "); // move to idle
    long cpu_idle = atoi(token); // store idle
    token = strtok(NULL, " "); // move to iowait
    long cpu_iowait = atoi(token); // store iowait
    token = strtok(NULL, " "); // move to irq
    long cpu_irq = atoi(token); // store irq
    token = strtok(NULL, " "); // move to softirq
    long cpu_softirq = atoi(token); // store softirq

    //get the sum of fields
    long sum = cpu_user + cpu_nice + cpu_system + cpu_idle + cpu_iowait + cpu_irq + cpu_softirq;
    return sum;
}

/**
 * this function reads the CPU statistics from the "/proc/stat" file and returns the CPU idle value.
 *
 * @return The CPU idle value.
 */
 long read_cpu_stat_return_idle(){
    FILE *file = fopen("/proc/stat", "r");    // open the file
    if(file == NULL){    //checking if successfully open the file
        fprintf(stderr, "Error opening file\n");  // if failed,  print the error
        exit(1);    // exit the program
    }
    char filecontent[1024];     // declear the array
    fgets(filecontent, 1024, file);     // copy the file content into the array
    fclose(file);   //close the array

    char *token = strtok(filecontent, " "); // the first is cpu
    token = strtok(NULL, " "); // move to user
    token = strtok(NULL, " "); // move to nice
    token = strtok(NULL, " "); // move to system
    token = strtok(NULL, " "); // move to idle
    long cpu_idle = atoi(token); // store idle

    return cpu_idle;
}

/**
 * this function calculates CPU usage based on the provided CPU statistics.
 *
 * @param cpu_now_idle Current CPU idle time.
 * @param cpu_now_sum Current sum of CPU fields.
 * @param cpu_last_idle Last CPU idle time.
 * @param cpu_last_sum Last sum of CPU fields.
 * @return The calculated CPU usage percentage.
 */
 float generate_cpu(long cpu_now_idle, long cpu_now_sum, long cpu_last_idle, long cpu_last_sum){
    long U_2 = cpu_now_sum - cpu_now_idle;
    long U_1 = cpu_last_sum - cpu_last_idle;

    float U = U_2 - U_1;
    float T = cpu_now_sum - cpu_last_sum;
    float cpu_usage = (U/T) * 100;
    
    // sprintf(cpu_use, " total cpu use = %.2f%%\n", cpu_usage);  
    
    return cpu_usage;
}

/**
 * this function generates CPU graphic information and prints it based on the provided CPU statistics.
 *
 * @param samples Number of samples.
 * @param now_cpu Current CPU usage.
 * @param cpu_info Array to store CPU information.
 * @param curr_line Current line of the iteration.
 * @param sequential Flag indicating sequential mode.
 */
 void generate_cpu_graphic(int samples, double now_cpu, char cpu_info[][1024], int curr_line, bool sequential){
    char graphic_info[1024] = "\0";
    
    // concatenate | for base graphic informtaion
    strcat(cpu_info[curr_line], "       |||");

    // concatenate | according to the nunber of now_cpu
    for(int i = 0; i < (int)now_cpu; i++){
        strcat(cpu_info[curr_line], "|");
    }

    sprintf(graphic_info, " %.2f", now_cpu);
    strcat(cpu_info[curr_line], graphic_info);

    if(sequential)  // if the user enter --sequential 
    {
        for(int i = 0; i < samples; i++){      
            if(i == curr_line){     // for the line that correspond to the iteration
                printf("%s\n", cpu_info[i]);     // print the information
            }
            else{
                printf("\n");     // for the lines that doesn't correspond to the iteration, print empty line
            }
        }
    }
    else{      // if the user doesn't enter --sequential
        for(int i = 0; i <= curr_line; i++){      // for the lines that up to the current line
            printf("%s\n", cpu_info[i]);      // print the infomation
        }
        for(int i = curr_line + 1; i < samples; i++){     // for the rest, after the current line
            printf("\n");    // print the empty line
        }
    }
}

/**
 * this function generates and prints memory information based on the provided memory statistics.
 *
 * @param samples Number of samples.
 * @param memory_info Array to store memory information.
 * @param curr_line Current line of the iteration.
 * @param sequential Flag indicating sequential mode.
 */
 void generate_memory_info(int samples, char memory_info[][1024], int curr_line, bool sequential){
    printf("---------------------------------------\n");
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");
    if(sequential)  // if the user enter --sequential 
    {
        for(int i = 0; i < samples; i++){      
            if(i == curr_line){     // for the line that correspond to the iteration
                printf("%s\n", memory_info[i]);     // print the information
            }
            else{
                printf("\n");     // for the lines that doesn't correspond to the iteration, print empty line
            }
        }
    }
    else{      // if the user doesn't enter --sequential
        for(int i = 0; i <= curr_line; i++){      // for the lines that up to the current line
            printf("%s\n", memory_info[i]);      // print the infomation
        }
        for(int i = curr_line + 1; i < samples; i++){     // for the rest, after the current line
            printf("\n");    // print the empty line
        }
    }
} 


/**
 * this function generates and prints information about active user sessions using utmp.h.
 */
void generate_user(int write_fd){
    printf("---------------------------------------\n");
    printf("### Sessions/users ### \n");
    
    struct utmp *ut;    //declear the struct
    setutent();     //go to the start of the user information
    while((ut = getutent()) != NULL){   // if the cuser urrent information is not NULL, also, read the next user information
        if(ut -> ut_type == USER_PROCESS){  // check if the user is currently login
            char user_info[1024];
            int nbytes;
            sprintf(user_info, " %s\t%s (%s)\n", ut -> ut_user, ut -> ut_line, ut -> ut_host);
            if((nbytes = write(write_fd, user_info, sizeof(user_info))) < 0){
                fprintf(stderr, "Fail to write to the pipe5");
                exit(1);
            }
        }
    }
}

/**
 * this function generates and prints the number of cores using sysconf.
 */
void generate_cores(){
    printf("---------------------------------------\n");
    printf("Number of cores: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));    // using sysconf to return the number of cores
}

/**
 * this function generates and prints system information including system name, machine name, version, release, architecture,
 * and the system running time since the last reboot.
 */
void generate_system_information(){
    printf("---------------------------------------\n");
    printf("### System Information ### \n");
    struct utsname system_inf;      // declare the struct if utsname
    uname(&system_inf);     // get the infomation of system 
    printf(" System Name = %s\n", system_inf.sysname);
    printf(" Machine Name = %s\n", system_inf.nodename);
    printf(" Version = %s\n", system_inf.version);
    printf(" Release = %s\n", system_inf.release);
    printf(" Architecture = %s\n", system_inf.machine);

    FILE *file = fopen("/proc/uptime", "r"); // open the /proc/uptime file to get the system running time
    if(file == NULL){    //checking if successfully open the file
        fprintf(stderr, "Error opening file\n");  // if failed,  print the error
        exit(1);    // exit the program
    }
    char filecontent[1024];     // declear the array
    fgets(filecontent, 1024, file);     // copy the content in the file into the array
    fclose(file);       // close the file

    char *token = strtok(filecontent, " ");     // get the first number in the file
    float time = atof(token);  // convert the time to float

    // convert the time to days hours minutes and seconds
    int days = time / 86400;
    time -= days * 86400;
    int hours = time / 3600;
    time -= hours * 3600.0;
    int minutes = time / 60;
    time -= minutes * 60.0;
    int seconds = time;
    int totalhours = 24 * days + hours;
    printf(" System running since last reboot: %d days %d:%d:%d (%d:%d:%d)\n", days, hours, minutes, seconds, totalhours, minutes, seconds);
}

/**
 * this function checks if a given string represents a valid integer.
 *
 * @param input The input string to be checked.
 * @return True if the string is a valid integer, false otherwise.
 */
bool check_valid_integer(char *input) {
    int i = 0;
    while (input[i] != '\0') {      // when the input is nonempty
        if (!isdigit(input[i])) {   //if the input is not integer
            return false;   // return false
        }
        i++;
    }
    return true;    // else, return true
}

