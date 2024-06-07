#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[]) {
    int pipe1[2];
    int pipe2[2];

    char buf[] = {'a'};

    if (pipe(pipe1) < 0) exit(-1);
    if (pipe(pipe2) < 0) exit(-1);

    // parent send in pipes1[1], child receives in pipes1[0]
    // child send in pipes2[1], parent receives in pipes2[0]
    if (fork() == 0) {  // child
        int pid = getpid();
        close(pipe1[1]);
        close(pipe2[0]);

        read(pipe1[0], buf, 1);
        printf("%d: received ping\n", pid);
        write(pipe2[1], buf, 1);
    
    } else {    // parent
        int pid = getpid();
        close(pipe1[0]);
        close(pipe2[1]);

        read(pipe1[1], buf, 1);
        printf("%d: received pong\n", pid);
        write(pipe2[0], buf, 1);
        
    }

    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    exit(0);
}   