#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>

void saveLog(){
    FILE* file;
    char fileName[100];
    int i;
    for(i = 0; i < 100; i++)
        fileName[i] = NULL;
    char* fn = getenv("HOME");
    strcat(fileName, fn);
    strcat(fileName, "/");
    strcat(fileName, "log.txt");
    file = fopen(fileName,"ab+");
    if( file == NULL )
        fprintf(stderr, "Error.. Could not open Log File.\n");
    else{
        fprintf(file, "Child Process was terminated\n");
        fclose ( file );
    }
}

void signalHandler(int signal)
{
	if (signal==SIGCHLD)
        saveLog();
}


void saveHistory(char* command){
    FILE* file;
    char fileName[100];
    int i;
    for(i = 0; i < 100; i++)
        fileName[i] = NULL;
    char* fn = getenv("HOME");
    strcat(fileName, fn);
    strcat(fileName, "/");
    strcat(fileName, "history.txt");
    file = fopen(fileName,"ab+");
    if( file == NULL )
        fprintf(stderr, "Error.. Could not open History File.\n");
    else{
        fprintf(file, "%s", command);
        fclose ( file );
    }
}

char keys[100];
int values[100], ks = 0;

bool handleEnvVars_1(char*** argv, int argc){
    if(argc==1){
        if((*argv)[0][1]=='='){
            keys[ks] = (*argv)[0][0];
            int i, digitsNumber = 0, value = 0;
            bool isValue = true;
            for(i = 2; (*argv)[0][i]!='\0';i++){
                int diff = (*argv)[0][i]-'0';
                if(diff>-1&&diff<10)
                    digitsNumber++;
                else
                    isValue = false;
            }
            if(!isValue||(*argv)[0][2]=='\0'){
                fprintf(stderr, "Invalid Environment Variable Declaration\n");
                return false;
            }
            for(i = 2; (*argv)[0][i]!='\0';i++)
                value+=(int)pow(10, digitsNumber--)*((*argv)[0][i]-'0');
            value/=10;
            values[ks] = value;
            ks++;
            return true;
        }
    }
    int i, j, k;
    for(i=0;i<argc;i++){
        for(j=0;(*argv)[i][j];j++){
            if((*argv)[i][j]=='$'){
                char s = (*argv)[i][j+1];
                for(k =0; k <ks;k++){
                    if(keys[k]==s){
                        (*argv)[i][j] = (char)(values[k]+'0');
                        (*argv)[i][j+1]='\0';
                    }
                }
            }
        }
    }
    return false;
}

bool execute(char*** args, int argc, bool background){
    if(handleEnvVars_1(args, argc))
        return true;
    if(strcmp((*args)[0], "pwd\n") == 0){
        printf("%s\n", getenv("HOME"));
        return true;
    }
    if(strcmp((*args)[0], "cd")==0){
        int ret = chdir((*args)[1]);
        if(ret == -1)
            fprintf(stderr, "Invalid Path\n");
        return false;
    }

    char* str_1 = getenv("PATH");
    char str[500];
    strcpy(str, str_1);
    char *p;
    char* dirs[150];
    int counter = 0, i;
    p = strtok (str,":");
    while (p != NULL)
    {
        dirs[counter] = p;
        p = strtok (NULL, ":");
        counter++;
    }
    char* tmp = (*args)[0];
    char dirs_2[150][150];
    for(i = 0; i < counter; i++){
        strcpy(dirs_2[i], dirs[i]);
        strcat(dirs_2[i], "/");
        strcat(dirs_2[i], tmp);
        if(access(dirs_2[i], F_OK)!=-1){
            (*args)[0] = dirs_2[i];
            break;
        }
    }
    pid_t child_pid;
    int child_status;
    child_pid = fork();
    if(child_pid == 0){
        execv((*args)[0], (*args));
        fprintf(stderr, "Invalid command\n");
        return false;
    }else if(!background) {
        pid_t tpid;
        do {
            tpid = wait(&child_status);
        } while(tpid != child_pid);
    }
    return true;
}


bool execute_Alt(char*** argv, int argc, bool background){
    if(handleEnvVars_1(argv, argc))
        return true;
    if(strcmp((*argv)[0], "pwd\n") == 0){
        printf("%s\n", getenv("HOME"));
        return true;
    }
    if(strcmp((*argv)[0], "cd")==0){
        int ret = chdir((*argv)[1]);
        if(ret == -1)
            fprintf(stderr, "Invalid Path\n");
        return false;
    }
    pid_t child_pid;
    int child_status;
    child_pid = fork();
    if(child_pid == 0){
        execvp((*argv)[0], (*argv));
        fprintf(stderr, "Invalid command\n");
        return false;
    }else if(!background) {
        pid_t tpid;
        do {
            tpid = wait(&child_status);
        } while(tpid != child_pid);
    }
    return true;
}

int splitLine(char* line, char*** argv) {

    char* buffer;
    int argc = 0;

    buffer = (char*) malloc(strlen(line) * sizeof(char));
    strcpy(buffer,line);
    (*argv) = (char**) malloc(20 * sizeof(char**));

    (*argv)[argc++] = strtok(buffer, " ");
    while ((((*argv)[argc] = strtok(NULL, " ")) != NULL) &&
        (argc < 150)) ++argc;

    (*argv)[argc] = NULL;
    return argc;
}

void printHistory(){
    FILE *file;
    char* fn = getenv("HOME");
    char fileName[100];
    strcat(fileName, fn);
    strcat(fileName, "/");
    strcat(fileName, "history.txt");
    file = fopen(fileName,"r");
    if( file == NULL )
        fprintf(stderr, "Error.. Could not open history Log.\n");
    else{
        char line[512];
        printf("\n");
        while(fgets(line, sizeof(line), file) != NULL)
            printf("%s", line);
        fclose ( file );
    }
}

bool isValidSpaces(char* line){
    char* buffer;

    buffer = (char*) malloc(strlen(line) * sizeof(char));
    strcpy(buffer,line);

    int i, j;
    for(j = 0; line[j];j++)
        if(line[j]!=' ')
            break;
    char last = line[j];
    for(i = j; line[i];i++){
        if((line[i]==' '&&last == ' '))
            return false;
        last = line[i];
    }
    if(line[strlen(line)-2] == ' '||(strlen(line)>2&&line[strlen(line)-2] == '&'&&line[strlen(line)-3]==' '))
        return false;
    return true;
}


void parseCommand(char** command){
    char* line = (*command);
    if(line[0] == '#')
        return;
    if(strlen(line) == 1){
        fprintf(stderr, "Empty Command Line\n");
        return;
    }
    if(strlen(line) > 512){
        fprintf(stderr, "Very Long Command\n");
        return;
    }
    if(!isValidSpaces(line)){
        fprintf(stderr, "Invalid White-Space delimiter\n");
        return;
    }
    if(strcmp(line, "history\n") == 0){
        printHistory();
        return;
    }
    //line[strlen(line)-1] = '\0';
    char** args;
    int n = 0;
    n = splitLine(line, &args);
    if(n>0&&strlen(args[n-1])>0)
        args[n-1][strlen(args[n-1])-1] = '\0';
    bool background = false;
    if((args[n-1])[strlen(args[n-1])-1]=='&'){
        background = true;
        (args[n-1])[strlen(args[n-1])-1] = '\0';
    }

    if(execute(&args, n, background)){
        //Save History
        saveHistory(line);
    }
}


void interactiveMode(){

    while(1){
        char curDir[100];
        char line[512];
        char* command;
        getcwd(curDir, 100);
        printf("%s@%s $", getlogin(),curDir);

        if (fgets(line, sizeof(line), stdin) == NULL
                ||strcmp(line, "exit\n")==0||line==EOF)
            exit(0);

        command = (char*) malloc(strlen(line) * sizeof(char));
        strcpy(command, line);
        parseCommand(&command);
    }
}

void bashMode(char* fileName){
    FILE *file;
    file = fopen(fileName,"r");
    if( file == NULL )
    {
        fprintf(stderr, "Error while opening the file.\n");
        exit(0);
    }
    printf("\tExecuting %s\n", fileName);
    while (1)
    {
        char line [ 512 ];
        char curDir[100];
        char* command;
        getcwd(curDir, 100);

        if (fgets(line, sizeof(line), file) == NULL
                ||strcmp(line, "exit\n")==0||line==EOF){
            exit(0);
        }
        line[strlen(line)-1] = '\0';
        printf("%s@%s $%s", getlogin(),curDir, line);
        command = (char*) malloc(strlen(line) * sizeof(char));
        strcpy(command, line);
        parseCommand(&command);
    }
    fclose ( file );
}

int main(int argc, char** argv)
{
    chdir(getenv("HOME"));
    signal(SIGCHLD,signalHandler);

    if(access((*argv), F_OK)&&argc==2){
        char fileName[100];
        strcpy(fileName, argv[1]);
        bashMode(fileName);
    }else if(argc == 1)
        interactiveMode();
    else
        fprintf(stderr, "Invalid Operation Required\n");

    return 0;
}
