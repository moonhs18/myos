#include "shell.h"
#include "ulib.h"
#include "syscall.h"
#include "vfs.h"

#define MAX_CMD_LEN 128
#define MAX_ARGS    8

UCODE static void cmd_help(void) {
static const char h1[] URODATA = "\n==============================================";
    static const char h2[] URODATA = "           MiniOS Interactive Shell Help       ";
    static const char h3[] URODATA = "==============================================";
    static const char h4[] URODATA = "  help             : Show command list";
    static const char h5[] URODATA = "  clear            : Clear console screen";
    static const char h6[] URODATA = "  echo [args...]   : Print message";
    static const char h7[] URODATA = "  ps               : List running process tasks";
    static const char h8[] URODATA = "  ls [path]        : List files in RamFS/DevFS";
    static const char h9[] URODATA = "  cat <file>       : Print file contents";
    static const char h10[] URODATA = "  touch <file>     : Create an empty file";
    static const char h11[] URODATA = "  write <file> <text>: Write text into file";
    static const char h12[] URODATA = "  exec <file>      : Execute ELF binary";
    static const char h13[] URODATA = "  exit             : Exit shell";
    static const char h14[] URODATA = "==============================================\n";

    u_puts(h1); u_puts(h2); u_puts(h3);
    u_puts(h4); u_puts(h5); u_puts(h6);
    u_puts(h7); u_puts(h8); u_puts(h9);
    u_puts(h10); u_puts(h11); u_puts(h12);
    u_puts(h13); u_puts(h14);
}

UCODE static void cmd_clear(void){
    static const char clr[] URODATA = "\033[2J\033[H";
    u_printf(clr);
}

UCODE static void cmd_echo(int argc, char **argv){
    static const char sp[] URODATA = " ";
    static const char nl[] URODATA = "";
    for (int i = 1; i < argc; i++) {
        u_printf(argv[i]);
        if (i < argc - 1) u_printf(sp);
    }
    u_puts(nl);
}

UCODE static void cmd_cat(const char *path){
    static const char err_msg[] URODATA = "[SHELL ERROR] Failed to open file: %s\n";
    static const char nl[] URODATA = "\n";
    int fd = (int)sys_open(path, O_RDONLY);
    if (fd < 0) {
        u_printf(err_msg, path);
        return;
    }
    char buf[128];
    int64_t bytes;
    while ((bytes = sys_read(fd, buf, sizeof(buf))) > 0) {
        sys_write(1,buf,(size_t)bytes);
    }
    sys_write(1,nl,1);
    sys_close(fd);
}

UCODE static void cmd_touch (const char *path){
    static const char ok_msg[] URODATA = "[SHELL] File created: %s\n";
    static const char err_msg[] URODATA = "[SHELL ERROR] Failed to create file: %s\n";
    int fd = (int)sys_open(path, O_CREAT | O_WRONLY);
    if (fd >= 0) {
        u_printf(ok_msg, path);
        sys_close(fd);
    } else {
        u_printf(err_msg, path);
    }
}

UCODE static void cmd_write(const char *path, const char *text){
    static const char ok_msg[] URODATA = "[SHELL] Successfully wrote to %s\n";
    static const char err_msg[] URODATA = "[SHELL ERROR] Failed to write file: %s\n";
    int fd = (int)sys_open(path, O_CREAT | O_RDWR);
    if (fd >= 0) {
        sys_write(fd, text, u_strlen(text));
        sys_close(fd);
        u_printf(ok_msg, path);
    } else {
        u_printf(err_msg, path);
    }
}

UCODE static void cmd_exec(const char *path){
    static const char err_msg[] URODATA = "[SHELL ERROR] Failed to execute ELF binary: %s\n";
    int64_t res = sys_execve(path);
    if (res < 0) {
        u_printf(err_msg, path);
    }
}
UCODE void shell_main(void) {
    static const char welcome1[] URODATA = "\n=======================================================";
    static const char welcome2[] URODATA = "   Welcome to MiniOS AArch64 Interactive Shell (EL0)   ";
    static const char welcome3[] URODATA = "=======================================================";
    static const char welcome4[] URODATA = "Type 'help' for a list of available commands.\n";

    static const char prompt[] URODATA = "MiniOS:~$ ";
    static const char default_ram[] URODATA = "/ram";

    static const char c_help[] URODATA = "help";
    static const char c_clear[] URODATA = "clear";
    static const char c_echo[] URODATA = "echo";
    static const char c_ps[] URODATA = "ps";
    static const char c_ls[] URODATA = "ls";
    static const char c_cat[] URODATA = "cat";
    static const char c_touch[] URODATA = "touch";
    static const char c_write[] URODATA = "write";
    static const char c_exec[] URODATA = "exec";
    static const char c_run[] URODATA = "run";
    static const char c_exit[] URODATA = "exit";

    static const char u_cat[] URODATA = "Usage: cat <filename>";
    static const char u_touch[] URODATA = "Usage: touch <filename>";
    static const char u_write[] URODATA = "Usage: write <filename> <text>";
    static const char u_exec[] URODATA = "Usage: exec <elf_filepath>";
    static const char term_msg[] URODATA = "[SHELL] Terminating shell process.";
    static const char unk_msg[] URODATA = "Unknown command: '%s'. Type 'help' for assistance.\n";

    char line[MAX_CMD_LEN];
    char *argv[MAX_ARGS];

    u_puts(welcome1);
    u_puts(welcome2);
    u_puts(welcome3);
    u_puts(welcome4);

    while (1) {
        u_printf(prompt);
        int len = u_readline(line, MAX_CMD_LEN);
        if (len <= 0){
            continue;
        }

        int argc = 0;
        char *ptr = line;

        while (*ptr && argc < MAX_ARGS) {
            while (*ptr == ' ' || *ptr == '\t'){
                *ptr++ = '\0';
            }

            if(*ptr == '\0'){
                break;
            }
            argv[argc++] = ptr;

            while (*ptr && *ptr != ' ' && *ptr != '\t'){
                ptr++;
            }

            if(*ptr != '\0'){
                *ptr++ = '\0';
            }
            
        }

        if (argc == 0) continue;

        if (u_strcmp(argv[0], c_help) == 0) {
            cmd_help();
        } else if (u_strcmp(argv[0], c_clear) == 0) {
            cmd_clear();
        } else if (u_strcmp(argv[0], c_echo) == 0) {
            cmd_echo(argc, argv);
        } else if (u_strcmp(argv[0], c_ps) == 0) {
            sys_ps();
        } else if (u_strcmp(argv[0], c_ls) == 0) {
            const char *path = (argc > 1) ? argv[1] : default_ram;
            sys_readdir(path);
        } else if (u_strcmp(argv[0], c_cat) == 0) {
            if (argc > 1) cmd_cat(argv[1]);
            else u_puts(u_cat);
        } else if (u_strcmp(argv[0], c_touch) == 0) {
            if (argc > 1) cmd_touch(argv[1]);
            else u_puts(u_touch);
        } else if (u_strcmp(argv[0], c_write) == 0) {
            if (argc > 2) cmd_write(argv[1], argv[2]);
            else u_puts(u_write);
        } else if (u_strcmp(argv[0], c_exec) == 0 || u_strcmp(argv[0], c_run) == 0) {
            if (argc > 1) cmd_exec(argv[1]);
            else u_puts(u_exec);
        } else if (u_strcmp(argv[0], c_exit) == 0) {
            u_puts(term_msg);
            sys_exit(0);
        } else {
            u_printf(unk_msg, argv[0]);
        }
    }
}