#include "kernel/types.h"
#include "user/user.h"

void prime(int fd) {

    int n;
    read(fd, &n, sizeof(int));
    int flag = 0;
    printf("prime %d\n", n);
    int pipes[2];
    int num;

    while(read(fd, &num, sizeof(int)) != 0) { // keep reading
        if(flag == 0) {
            
            flag = 1;
            pipe(pipes);
            if (fork() == 0) {  // child
                close(pipes[1]);
                prime(pipes[0]);
                return;
            } else {    // parent
                close(pipes[0]);
            }
        }
        // not multiply for our prime, pass to next filter
        if(num % n != 0){
            write(pipes[1], &num, sizeof(int));
        }
        
    }
    close(fd);
    close(pipes[1]);
    wait(0);
}

int main(int argc, char* argv[]) {
    int pipes[2]; // [0] to receive, [1] to send
    pipe(pipes);

    if (fork() == 0) {  // child
        close(pipes[1]);
        prime(pipes[0]);
        close(pipes[0]);
    } else {    // parent

        close(pipes[0]);
        for(int i = 2; i<=35; i++) {
            write(pipes[1], &i, sizeof(int));
        }        
        close(pipes[1]);
        wait(0);
    }
    return 0;
}
