// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "user/user.h"
// #include "kernel/param.h"

// char* readline(char* argv[MAXARG], int curr_argc) {
//     char buff[1024];

//     int n = 0;
//     while(read(0, buff, 1)) {
//         if (n >= 1023) {
//             printf("arg too long");
//             exit(0);
//         }
//     }
// }

// int main(int argc, char* argv[]) {
//     if (argc <= 1) {
//         printf("Usage: xargs command (arg ...)\n");
//     }

//     char *command = malloc(strlen(argv[1]) + 1);
//     char *new_argv[MAXARG];

//     if (fork() == 0) {  // child

//     } else {    // parent

//     }
// }