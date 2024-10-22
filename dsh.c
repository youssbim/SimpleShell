#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LINE 4096
#define MAX_ARGS 30
#define MAX_PATH 512
#define MAX_PROMPT 32
#define MAX_HISTORY 10

char _path[MAX_PATH] = "/bin/:/usr/bin";

void panic1(const char* msg){
    if(errno){
        fprintf(stderr, "PANIC: %s: %s\n\n", msg, strerror(errno));
    } else{
        fprintf(stderr, "PANIC: %s: \n\n", msg);
    }
    exit(EXIT_FAILURE);
}

// Limita la cronologia a MAX_HISTORY
void trim_history() {
    while (history_length > MAX_HISTORY) {
        HIST_ENTRY *entry = remove_history(0);
        if (entry) free_history_entry(entry);
    }
}

int prompt(char* buf, size_t buf_size, const char* prompt_string){
    // Presenta al prompt prompt_string
    char* input = readline(prompt_string);
    if (input == NULL) {
        return EOF;
    }
    // Estrae dallo stream input
    strncpy(buf, input, buf_size);
    // buf è una stringa valida che termina con carattere nullo
    buf[buf_size -1] = '\0';
    
    // Dealloca la memoria utilizzata per contenere l'input
    free(input);

    // Aggiunge il comando alla cronologia
    if (buf[0] != '\0') {
        add_history(buf); 	
        trim_history(); 
    }

    return 1;
}

void set_path(const char* new_path){
    if(new_path != NULL){
        // Change the path
        int cur_pos = 0;
        while(new_path[cur_pos] != '\0'){
            cur_pos++;
            if(cur_pos >= MAX_PATH -1 && new_path[cur_pos] != '\0'){
                fprintf(stderr, "Error: PATH string too long");
                return;
            }
        }
        if (cur_pos > 0) {
            memcpy(_path, new_path, cur_pos + 1);
        }
    }
    printf("%s\n", _path);
}

void path_lookup(char* abs_path, const char* rel_path){
    char* prefix;
    char buf[MAX_PATH];
    if (abs_path == NULL || rel_path == NULL)
        panic1("get_abs_path: parameter error");
    prefix = strtok(_path, ":");
    while(prefix != NULL){
        strcpy(buf, prefix);
        strcat(buf, rel_path);
        if(access(buf, X_OK) == 0){
            strcpy(abs_path, buf);
            return;
        }
        prefix = strtok(NULL, ":");
    }
    strcpy(abs_path, rel_path);
}

void exec_rel2abs(char** arg_list){
    if(arg_list[0][0] == '/'){
        // Assume abs path
        execv(arg_list[0], arg_list);
    } else{
        // Assume rel path
        char abs_path[MAX_PATH];
        path_lookup(abs_path, arg_list[0]);
        execv(abs_path, arg_list);
    }
}

void do_redir(const char* out_path, char** arg_list, const char* mode){
    if(out_path == NULL)
        panic1("do redir: no path");
    int pid = fork();
    if(pid > 0){
        int wpid = wait(NULL);
        if (wpid < 0) panic1("do_redir: wait");
    } else if (pid == 0){
        // Begin child code
        FILE* out = fopen(out_path, mode);
        if (out == NULL) {
            perror(out_path);
            exit(EXIT_FAILURE);
        }
        dup2(fileno(out), 1); // 1 == fileno(stdout)
        exec_rel2abs(arg_list);
        perror(arg_list[0]);
        exit(EXIT_FAILURE);
        // End child code
    } else {
        panic1("do_redir: fork");
    }
}

void do_pipe(size_t pipe_pos, char** arg_list){
    int pipefd[2];
    int pid;
    if(pipe(pipefd) < 0) panic1("do_pipe: pipe");
    // Left side of the pipe
    pid = fork();
    if(pid > 0){
        int wpid = wait(NULL);
        if (wpid < 0) panic1("do_pipe: wait");
    } else if (pid == 0){
        // Begin child code
        close(pipefd[0]);
        dup2(pipefd[1], 1);
        close(pipefd[1]);
        exec_rel2abs(arg_list);
        perror(arg_list[0]);
        exit(EXIT_FAILURE);
        // End child code
    } else {
        panic1("do_pipe: fork");
    }
    // Right side of the pipe
    pid = fork();
    if(pid > 0){
        close(pipefd[0]);
        close(pipefd[1]);
        int wpid = wait(NULL);
        if (wpid < 0) panic1("do_pipe: wait");
    } else if (pid == 0){
        // Begin child code
        close(pipefd[1]);
        dup2(pipefd[0], 0);
        close(pipefd[0]);
        exec_rel2abs(arg_list + pipe_pos + 1);
        perror(arg_list[pipe_pos + 1]);
        exit(EXIT_FAILURE);
        // End child code
    } else {
        panic1("do_pipe: fork");
    }
}

void do_exec(char** arg_list){
    int pid = fork();
    if(pid > 0){
        int wpid = wait(NULL);
        if (wpid < 0) panic1("do_exec: wait");
    } else if (pid == 0){
        // Begin child code
        exec_rel2abs(arg_list);
        perror(arg_list[0]);
        exit(EXIT_FAILURE);
        // End child code
    } else {
        panic1("do_exec: fork");
    }
}

int main(void){
    char input_buffer[MAX_LINE];
    size_t arg_count;
    char* arg_list[MAX_ARGS];
    char prompt_string[MAX_PROMPT] = "\0";
    
    if(isatty(0)){
        strcpy(prompt_string, "dsh$ \0");
    }
    
    while(prompt(input_buffer, MAX_LINE, prompt_string) >= 0){
        // Tokenize input
        arg_count = 0;
        arg_list[arg_count] = strtok(input_buffer, " ");
        if(arg_list[arg_count] == NULL){
            continue;
        }else{
            do{
                arg_count++;
                if(arg_count > MAX_ARGS) break;
                arg_list[arg_count] = strtok(NULL, " ");
            }while(arg_list[arg_count] != NULL);
        }
        // Guardia
#if USE_DEBUG_PRINTF
        // [DEBUG] print tokens
        printf("DEBUG: tokens");
        for(size_t i = 0; i < arg_count; i++){
            printf(" %s", arg_list[i]);
        }
        puts("");
#endif
        
        // Builtins
        if(strcmp(arg_list[0], "exit") == 0){
            break;
        }
        
        if(strcmp(arg_list[0], "setpath") == 0){
            set_path(arg_list[1]);
            continue;
        }
        {
            // Check for special characters
            size_t redir_pos = 0;
            size_t append_pos = 0;
            size_t pipe_pos = 0;
            for(size_t i = 0; i < arg_count; i++){
                if(strcmp(arg_list[i], ">") == 0){
                    redir_pos = i;
                    break;
                }
                if(strcmp(arg_list[i], ">>") == 0){
                    append_pos = i;
                    break;
                }
                if(strcmp(arg_list[i], "|") == 0){
                    pipe_pos = i;
                    break;
                }
                /* if strcmp ...*/
            }
            // Do shell ops
            // Redirect
            if(redir_pos != 0){
                // a1 a2 > a3
                arg_list[redir_pos] = NULL;
                // Effettuare la redirection
                do_redir(arg_list[redir_pos+1], arg_list, "w+");
            } else if (append_pos != 0){
                arg_list[append_pos] = NULL;
                do_redir(arg_list[append_pos+1], arg_list, "a+");
            } else if (pipe_pos != 0){
                arg_list[pipe_pos] = NULL;
                do_pipe(pipe_pos, arg_list);
                // else if() {} ...
            } else{
                // Exec
                do_exec(arg_list);
            }
        }
    }
    return EXIT_SUCCESS;
}
