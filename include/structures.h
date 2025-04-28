#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* A process is a single process.  */
typedef struct process
{
    struct process *next;       /* next process in pipeline */
    char **argv;                /* for exec */
    pid_t pid;                  /* process ID */
    char completed;             /* true if process has completed */
    char stopped;               /* true if process has stopped */
    int status;                 /* reported status value */
} process;


/* A job is a pipeline of processes.  */
typedef struct job
{
    struct job *next;           /* next active job */
    char *command;              /* command line, used for messages */
    process *first_process;     /* list of processes in this job */
    pid_t pgid;                 /* process group ID */
    char notified;              /* true if user told about stopped job */
    struct termios tmodes;      /* saved terminal modes */
    int stdin, stdout, stderr;  /* standard i/o channels */
} job;




/* The active jobs are linked into a list.  This is its head.   */
job *first_job = NULL;



/* Find the active job with the indicated pgid.  */
job* find_job(pid_t pgid) {
    job *j;

    for (j = first_job; j; j = j->next) {
        if (j->pgid == pgid) {
            return j;
        }
    }
    return NULL;
}


/* Return true if all processes in the job have stopped or completed.  */
int job_is_stopped(job *j) {
    process *p;

    for (p = j->first_process; p; p = p->next) {
        if (!p->completed && !p->stopped) {
            return 0;
        }
    }
    return 1;
}


/* Return true if all processes in the job have completed.  */
int job_is_completed(job *j) {
    process *p;

    for (p = j->first_process; p; p = p->next) {
        if (!p->completed) {
            return 0;
        }
    }
    return 1;
}


// VISUEL (WEIL ES IST SCHÖN)
#define RST     "\033[0m"
#define Y       "\033[1;33m"
#define RED     "\033[38;5;210m"

#define G       "\033[1;32m"
#define B       "\033[1;34m"
#define M       "\033[1;35m"
#define C       "\033[1;36m"
#define RED_back "\033[41m"
#define BLA_back "\033[41m"
#define Bold    "\033[1m"
#define W_back "\033[47m"

void printbanner(void)
{
	/*
	printf(G"\n=== CELL TERMINAL INITIALIZED ===\n");
	printf("Imperfect Shell | v1.0 | the basic/simple idea of a shell\n");
	printf("Read-Evaluate/parse-Print/execute-Loop\n\n");
	*/

	// TAAG (Text to ASCII Art Generator). ANSI_shadow
    printf(RED
        "                                            \n"
  "███"RST G"╗"RST RED"   ███"RST G"╗"RED" █████"RST G"╗"RED" ██████"RST G"╗"RED"  ██████"RST G"╗"RED" ███████"G"╗\n"
  RED"████"RST G"╗"RED" ████"RST G"║"RED"██"RST G"╔══"RED"██"RST G"╗"RED"██"RST G"╔══"RED"██"RST G"╗"RED"██"RST G"╔═══"RED"██"RST G"╗"RED"██"RST G"╔════╝\n"
  RED"██"RST G"╔"RED"████"RST G"╔"RED"██"RST G"║"RED"███████"RST G"║"RED"██████"RST G"╔╝"RED"██"RST G"║"RED"   ██"G"║"RED"███████"G"╗\n"
  RED"██"RST G"║╚"RED"██"RST G"╔╝"RED"██"RST G"║"RED"██"RST G"╔══"RED"██"RST G"║"RED"██"RST G"╔══"RED"██"RST G"╗"RED"██"RST G"║"RED"   ██"RST G"║╚════"RED"██"RST G"║\n"
  RED"██"RST G"║"" "RST G"╚═╝"RED" ██"RST G"║"RED"██"RST G"║"RED"  ██"RST G"║"RED"██"RST G"║"RED"  ██"RST G"║╚"RED"██████"G"╔╝"RED"███████"G"║\n"
  RST G"╚═╝""     ""╚═╝╚═╝""  ""╚═╝╚═╝""  ""╚═╝"" ""╚═════╝"" ""╚══════╝\n"
    "                                            \n"
                                              RST);
}
