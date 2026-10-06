#include "shell.h"
#include "ulib.h"
#include "syscall.h"
#include "vfs.h"

#define MAX_CMD_LEN 128
#define MAX_ARGS    8

static void cmd_help(void) {
    u_puts("\n==============================================");
    u_puts("           MiniOS Interactive Shell Help       ");
    u_puts("==============================================");
    u_puts("  help             : Show command list");
    u_puts("  clear            : Clear console screen");
    u_puts("  echo [args...]   : Print message");
    u_puts("  ps               : List running process tasks");
    u_puts("  ls               : List files in RamFS/DevFS");
    u_puts("  cat <file>       : Print file contents");
    u_puts("  touch <file>     : Create an empty file");
    u_puts("  write <file> <msg>: Write text into file");
    u_puts("  exec <file>      : Execute ELF binary");
    u_puts("  exit             : Exit shell");
    u_puts("==============================================\n");
}

static void cmd_clear(void){
    u_printf("\033[2J\033[H");
}

static void cmd_echo(int argc, char **argv){
    for(int i = 1; i < argc; i++){
        u_printf("%s%s", argv[i], (i == argc - 1) ? "" : " ");
    }
    u_puts("");
}

static void cmd_cat(const char *path){
    int fd = (int)sys_open(path, O_RDONLY);
    if(fd < 0){
        u_printf("[SHELL ERROR] Failed to open file: %s\n", path);
        return;
    }

    char buf[20];
    int64_t bytes;
    while((bytes = sys_read(fd, buf, sizeof(buf) - 1)) > 0){
        buf[bytes] = '\0';
        u_printf("%s", buf);
    }
    u_puts("");
    sys_close(fd);
}

static void cmd_touch (const char *path){
    int fd = (int)sys_open(path, O_CREAT | O_WRONLY);
    if(fd >= 0){
        u_printf("[SHELL] File created: %s\n", path);
        sys_close(fd);
    }else{
        u_printf("[SHELL ERROR] Failed to create file: %s\n", path);
    }
}

static void cmd_write(const char *path, const char *text){
    int fd = (int)sys_open(path, O_CREAT | O_RDWR);
    if(fd >=0){
        sys_write(fd, text, u_strlen(text));
        sys_close(fd);
        u_printf("[SHELL] Successfully wrote to %s\n", path);
    }
    else{
        u_printf("[SHELL ERROR] Failed to write file: %s\n", path);
    }
}

static void cmd_exec(const char *path){
    int64_t res = sys_execve(path);
    if(res < 0){
        u_printf("[SHELL ERROR] Failed to execute ELF binary: %s\n", path);
    }
}

void __attribute__((section(".user_text"))) shell_main(void){
    char line[MAX_CMD_LEN];
    char *argv[MAX_ARGS];

    u_puts("\n=======================================================");
    u_puts("   Welcome to MiniOS AArch64 Interactive Shell (EL0)   ");
    u_puts("=======================================================");
    u_puts("Type 'help' for a list of available commands.\n");

    while (1) {
        u_printf("MiniOS:~$ ");
        int len = u_readline(line, MAX_CMD_LEN);
        if (len == 0) continue;

        int argc = 0;
        char *ptr = line;
        while (*ptr && argc < MAX_ARGS) {
            while (*ptr == ' ') *ptr++ = '\0';
            if (*ptr == '\0') break;
            argv[argc++] = ptr;
            while (*ptr && *ptr != ' ') ptr++;
        }

        if (argc == 0) continue;

        if (u_strcmp(argv[0], "help") == 0) {
            cmd_help();
        } else if (u_strcmp(argv[0], "clear") == 0) {
            cmd_clear();
        } else if (u_strcmp(argv[0], "echo") == 0) {
            cmd_echo(argc, argv);
        } else if (u_strcmp(argv[0], "ps") == 0) {
            sys_ps();
        } else if (u_strcmp(argv[0], "ls") == 0) {
            const char *path = (argc > 1) ? argv[1] : "/ram";
            sys_readdir(path);
        } else if (u_strcmp(argv[0], "cat") == 0) {
            if (argc > 1) cmd_cat(argv[1]);
            else u_puts("Usage: cat <filename>");
        } else if (u_strcmp(argv[0], "touch") == 0) {
            if (argc > 1) cmd_touch(argv[1]);
            else u_puts("Usage: touch <filename>");
        } else if (u_strcmp(argv[0], "write") == 0) {
            if (argc > 2) cmd_write(argv[1], argv[2]);
            else u_puts("Usage: write <filename> <text>");
        } else if (u_strcmp(argv[0], "exec") == 0 || u_strcmp(argv[0], "run") == 0) {
            if (argc > 1) cmd_exec(argv[1]);
            else u_puts("Usage: exec <elf_filepath>");
        } else if (u_strcmp(argv[0], "exit") == 0) {
            u_puts("[SHELL] Terminating shell process.");
            sys_exit(0);
        } else {
            u_printf("Unknown command: '%s'. Type 'help' for assistance.\n", argv[0]);
        }
    }
}