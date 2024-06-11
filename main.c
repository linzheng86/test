# include "stats_functions.c"

int get_memory_child_pid(pid_t pid){
    static int check_called_memory = -1;
    if(pid == -1){
        return check_called_memory;
    }
    else{
        check_called_memory = pid;
        return 0;
    }
}

int get_user_child_pid(pid_t pid){
    static int check_called_user = -1;
    if(pid == -1){
        return check_called_user;
    }
    else{
        check_called_user = pid;
        return 0;
    }
}

int get_cpu_child_pid(pid_t pid){
    static int check_called_cpu = -1;
    if(pid == -1){
        return check_called_cpu;
    }
    else{
        check_called_cpu = pid;
        return 0;
    }
}

void ctrl_c_handler(int sig){
    char input;
    printf("\nDo you want to quit the program? Choose by pressing [y/n]: ");
    scanf(" %c", &input);
    if (input == 'y' || input == 'Y') {
        int memory_pid = get_memory_child_pid(-1);
        int user_pid = get_user_child_pid(-1);
        int cpu_pid = get_cpu_child_pid(-1);
        if(memory_pid != -1){
            kill(memory_pid, SIGKILL);  //kill child process
        }
        if(user_pid != -1){
            kill(user_pid, SIGKILL);    // kill child process
        }
        if(cpu_pid != -1){
            kill(cpu_pid, SIGKILL);     // kill child process
        }
        exit(0);    // exit the parent
    }
    // else, resume the program
}

void ctrl_c_handler_for_child(int sig){
    return;
}

void ctrl_z_handler(int sig) {
    return;
}

/**
 * @brief Main function to collect and display system resource information.
 *
 * The program collects information about memory and CPU usage based on
 * user-specified command-line arguments. It provides options to customize
 * the number of samples, time delay between samples, and whether to display
 * system and user information.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return Exit code. Returns 0 on successful execution, 1 on error.
 */
int main(int argc, char **argv) {

    signal(SIGTSTP, ctrl_z_handler);    // signal handler for ctrl_z
    signal(SIGINT, ctrl_c_handler);     // signal handler for ctrl_c

    int samples = 10; // default number of samples set to 10
    int tdelay = 1; // default number of seconds set to 1
    bool system = false;    // default status is false;
    bool user = false;      // default status is false;
    bool sequential = false;    // default status is false;
    bool graphic = false;   // default status is false;
    double last_memory = 0.00;    // default number is 0.00
    bool sample_flag = false;
    bool tdelay_flag = false;
    int number_of_integer = 0;
    char memory_info[samples][1024];    // declare the array for memory information 
    char cpu_info[samples][1024];      // declare the array for cpu information
    char write_memory_info[samples][1024];
    char write_cpu_use[1024];
    char last_number[1024];
    

    if (argc > 1){
        int i = 1;  // Start from index 1 to skip the program name (argv[0])
        while (i < argc){   // iterate whening i is smaller than the number of argc
            char *token = strtok(argv[i], "=");     // if the user enters --samples=N or --tdelay=N, split these input
            if(strcmp(token, "--samples") == 0){    //if the user enters --tsamples=N, and split at =, check if input is correct
                token = strtok(NULL, "");
                if(token != NULL){      // if N is nonempty
                    samples = atoi(token);      // convert N to integer, and store in samples
                    sample_flag = true;
                } 
            } 
            else if(strcmp(token, "--tdelay") == 0){    // if the user enters --tdelay=N. and split at =, ckeck if input is correct
                token = strtok(NULL, "");
                if(token != NULL){      // if N in nonempty
                    tdelay = atoi(token);      // convert N to integer, and store in tdelay
                    tdelay_flag = true;
                } 
            } 
            else if(strcmp(argv[i], "--system") == 0){  // if the user enters --system
                system = true;      // turn system to true
            } 
            else if(strcmp(argv[i], "--user") == 0){    // if the user enters --user
                user = true;        // turn user to true
            } 
            else if(strcmp(argv[i], "--sequential") == 0){      // if the user enters --sequential
                sequential = true;      // turn sequential to true
            } 
            //if the user doesn't put any words but integer, check if input has two consecutive numbers and they are valid integers
            else if(check_valid_integer(argv[i])) {
                number_of_integer++;
                if(number_of_integer == 1 && sample_flag == false){
                    samples = atoi(argv[i]);
                }
                else if(number_of_integer == 2 && tdelay_flag == false){
                    tdelay = atoi(argv[i]);
                }
                if (number_of_integer > 2){
                    printf("INVALID INPUT\n");      // print invalid input
                    return 1;   // exit the program
                }
            }
            else if(strcmp(argv[i], "--graphic") == 0 || strcmp(token, "-g") == 0){ // check if the user eneters --graphic or -g
                graphic = true;     // turn graphic to true
            }
            else{   //if the input is none of the above
                printf("INVALID INPUT\n");      // print invalid input
                return 1;   // exit the program
            }
            i++;    // go to the next
        }
    }

    if((user && system)|| !user ){
        for (int i = 0; i < samples; i++){
            long cpu_last_sum = read_cpu_stat_return_sum();  // get the cpu sum before delay
            long cpu_last_idle = read_cpu_stat_return_idle();    // get the cpu idle before delay
            if(i > 0){
                sleep(tdelay);  // cause the delay
            }
            generate_header(samples, tdelay, sequential, i); //print the header for situation
        
            generate_memory_usage(); 
            int memory_pipe[2];
            int user_pipe[2];
            int cpu_pipe[2];
            int last_memory_pipe[2];
            int nbytes;

            if (pipe(memory_pipe) == -1 || pipe(user_pipe) == -1 || pipe(cpu_pipe) == -1 || pipe(last_memory_pipe) == -1)
            {
                perror("Pipe");
                exit(1);
            }
            pid_t child_pids[3];
            for (int j = 0; j < 3; j++){
                if(j == 0) {    // child process 1 for memory utilization
                    child_pids[j] = fork();
                    if(child_pids[j] == -1) {    // unsuccessfully call fork
                        perror("Fork");
                        exit(1);
                    }
                    else if(child_pids[j] == 0) {    // in child process, write to the pipe
                        signal(SIGINT, ctrl_c_handler_for_child);     // signal handler for ctrl_c
                        close(memory_pipe[0]);  
                        close(last_memory_pipe[0]);
                        double now_memory = calculate_memory_info(memory_info, i);
                        if(graphic){
                            generate_memory_graphic(i, &last_memory, now_memory, memory_info);
                        }
                        if((nbytes = write(memory_pipe[1], memory_info[i], sizeof(memory_info[i]))) < 0){
                            fprintf(stderr, "Fail to write to the pipe1");
                            exit(0);
                        }
                        close(memory_pipe[1]);

                        char transfer[1024]; 
                        sprintf(transfer, "%f\n", last_memory);
                        if((nbytes = write(last_memory_pipe[1], transfer, sizeof(transfer))) < 0){
                            fprintf(stderr, "Fail to write to the pipe2");
                            exit(1);
                        }
                        close(last_memory_pipe[1]);
                        exit(1);
                    }
                    else {      // in parent process, read from the pipe
                        get_memory_child_pid(child_pids[j]);
                        wait(NULL);
                        close(memory_pipe[1]);
                        close(last_memory_pipe[1]);
                        if ((nbytes = read(memory_pipe[0], write_memory_info[i], sizeof(write_memory_info[i]))) < 0){
                            fprintf(stderr, "Fail to read to the pipe3");
                            exit(1);
                        }

                        if ((nbytes = read(last_memory_pipe[0], last_number, sizeof(last_number))) < 0){
                            fprintf(stderr, "Fail to read to the pipe4");
                            exit(1);
                        }
                        close(last_memory_pipe[0]);
                        last_memory = atof(last_number);

                        close(memory_pipe[0]);
                        generate_memory_info(samples, write_memory_info, i, sequential);
                    }
                }
                if((j == 1) && ((user && system) || !system)) {    // child process 2 for connected user
                    child_pids[j] = fork();
                    if(child_pids[j] == -1) {
                        perror("Fork");
                        exit(1);
                    }
                    else if(child_pids[j] == 0){
                        signal(SIGINT, ctrl_c_handler_for_child);     // signal handler for ctrl_c
                        close(user_pipe[0]);
                        generate_user(user_pipe[1]);
                        close(user_pipe[1]);
                        exit(1);
                    }
                    else{
                        get_user_child_pid(child_pids[j]);
                        wait(NULL);
                        close(user_pipe[1]);

                        char buffer[1024];
                        while ((nbytes = read(user_pipe[0], buffer, 1024)) > 0) {
                            printf("%s", buffer);
                        }
                        if((nbytes = read(user_pipe[0], buffer, 1024)) < 0){
                            fprintf(stderr, "Fail to read to the pipe");
                            exit(1);
                        }
                        close(user_pipe[0]);
                    }
                }
                if(j == 2) {    // child process 3 for CPU utilization
                    child_pids[j] = fork();
                    if(child_pids[j] == -1) {
                        perror("Fork");
                        exit(1);
                    }
                    else if(child_pids[j] == 0){
                        signal(SIGINT, ctrl_c_handler_for_child);     // signal handler for ctrl_c
                        close(cpu_pipe[0]);
                        if(i == 0){
                            sleep(tdelay);
                        }
                        long cpu_now_sum = read_cpu_stat_return_sum();   // get the cpu sum after delay
                        long cpu_now_idle = read_cpu_stat_return_idle();
                        float cpu_usage = generate_cpu(cpu_now_idle, cpu_now_sum, cpu_last_idle, cpu_last_sum);   // calculate cpu usage
                        char convert[50]; 
                        sprintf(convert, "%f\n", cpu_usage);
                        if((nbytes = write(cpu_pipe[1], convert, sizeof(convert))) < 0){
                            fprintf(stderr, "Fail to write to the pipe");
                            exit(1);
                        }
                        close(cpu_pipe[1]);
                        exit(1);
                    }
                    else{
                        get_cpu_child_pid(child_pids[j]);
                        wait(NULL);
                        close(cpu_pipe[1]);
                        if ((nbytes = read(cpu_pipe[0], write_cpu_use, sizeof(write_cpu_use))) < 0){
                            fprintf(stderr, "Fail to read to the pipe");
                            exit(1);
                        }
                        close(cpu_pipe[0]);
                        generate_cores();
                        float cpu_use = atof(write_cpu_use);
                        printf(" total cpu use = %.2f%%\n", cpu_use);  
                        if(graphic){    // if user enters --graphic or -g
                            generate_cpu_graphic(samples, cpu_use, cpu_info, i, sequential);    // print graphic for cpu
                        }
                    }
                }
            }
        }
    }
    else{
        generate_header(samples, tdelay, sequential, 0);    // print header
        generate_memory_usage();
        int user_pipe[2];
        int nbytes;
        if (pipe(user_pipe) == -1)
        {
            perror("Pipe");
            exit(1);
        }

        pid_t child_pids = fork();
        if(child_pids == -1) {
            perror("Fork");
            exit(1);
        }
        else if(child_pids == 0){
            close(user_pipe[0]);
            generate_user(user_pipe[1]);
            close(user_pipe[1]);
            exit(1);
        }
        else{
            get_user_child_pid(child_pids);
            wait(NULL);
            close(user_pipe[1]);
            char buffer[1024];
            while ((nbytes = read(user_pipe[0], buffer, 1024)) > 0) {
                printf("%s", buffer);
            }
            if((nbytes = read(user_pipe[0], buffer, 1024)) < 0){
                fprintf(stderr, "Fail to read to the pipe");
                exit(1);
            }
            close(user_pipe[0]);
        }

    }
    generate_system_information();
    return 0;
}