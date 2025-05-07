/**
 * @file shellHandler.c
 * @brief Job control and process management for a simple shell.
 * 
 * Handles foreground/background jobs, process groups, signals, and I/O redirection.
 * Maintains a list of active jobs and provides functions to launch, track, and manage them.
 */

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "shellHandler.h"

static job *first_job = NULL;

static pid_t shell_pgid;
static struct termios shell_tmodes;
static int shell_terminal;
static int shell_is_interactive;

/**
 * @brief Updates the status of a process in the job list.
 * 
 * This function checks the status returned by waitpid() and updates the corresponding 
 * process in the job list (stopped, completed, or terminated by a signal).
 * 
 * @param pid The process ID to update.
 * @param status The status returned by waitpid().
 * @return 0 on success, -1 on error.
 */
int mark_process_status(pid_t pid, int status)
{
	job *j;
	process *p;

	if (pid > 0)
	{
		/* Update the record for the process. */
		for (j = first_job; j; j = j->next)
			for (p = j->first_process; p; p = p->next)
				if (p->pid == pid)
				{
					p->status = status;
					if (WIFSTOPPED(status))
						p->stopped = 1;
					else
					{
						p->completed = 1;
						if (WIFSIGNALED(status))
							fprintf(stderr, "%d: Terminated by signal %d.\n", (int)pid, WTERMSIG(p->status));
					}
					return 0;
				}
		fprintf(stderr, "No child process %d.\n", pid);
		return -1;
	}
	else if (pid == 0 || errno == ECHILD)
	{
		/* No processes ready to report. */
		return -1;
	}
	else
	{
		/* Other weird errors. */
		perror("waitpid");
		return -1;
	}
}

/**
 * @brief Waits for a job to report status changes.
 * 
 * Blocks until all processes in the job have reported their status (stopped or completed).
 * 
 * @param j The job to wait for.
 */
void wait_for_job(job *j)
{
	int status;
	pid_t pid;
	do
		pid = waitpid(WAIT_ANY, &status, WUNTRACED);
	while (!mark_process_status(pid, status) && !job_is_stopped(j) && !job_is_completed(j));
}

/**
 * @brief Prints formatted job information for the user.
 * 
 * Displays the job's PGID, status, and command string.
 * 
 * @param j The job to display.
 * @param status A string describing the job's status (e.g., "stopped", "completed").
 */
void format_job_info(job *j, const char *status)
{
	fprintf(stderr, "%ld (%s): %s\n", (long)j->pgid, status, j->command);
}

/**
 * @brief Notifies the user about job state changes.
 * 
 * Checks all jobs for stopped/completed processes, prints notifications, 
 * and removes terminated jobs from the active list.
 */
void do_job_notification(void)
{
	job *j, *jlast, *jnext;
	process *p;

	/* Update status information for child processes.  */
	// update_status ();

	jlast = NULL;
	for (j = first_job; j; j = jnext)
	{
		jnext = j->next;

		/* If all processes have completed, tell the user the job has
		   completed and delete it from the list of active jobs.  */
		if (job_is_completed(j))
		{
			format_job_info(j, "completed");
			if (jlast)
				jlast->next = jnext;
			else
				first_job = jnext;
			// free_job (j);
		}

		/* Notify the user about stopped jobs,
		   marking them so that we won't do this more than once.  */
		else if (job_is_stopped(j) && !j->notified)
		{
			format_job_info(j, "stopped");
			j->notified = 1;
			jlast = j;
		}

		/* Don't say anything about jobs that are still running.  */
		else
			jlast = j;
	}
}

/**
 * @brief Sets the first job in the global job list.
 * 
 * Internal function used to manage the linked list of active jobs.
 * 
 * @param job The job to set as the first in the list.
 */
void set_first_job(job *job)
{

	first_job = job;
}

/**
 * @brief Initializes shell settings for job control.
 * 
 * Configures interactive mode, signal handling, process group, and terminal control.
 * Must be called before any job management functions.
 */
void init_shell()
{
	/* see if we are running interactively. */
	shell_terminal = STDIN_FILENO;
	shell_is_interactive = isatty(shell_terminal);

	if (shell_is_interactive)
	{
		/*Loop until we are in the foreground*/
		while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
			kill(-shell_pgid, SIGTTIN);

		/*Ignore interactive and job-control signals. */
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		signal(SIGTSTP, SIG_IGN);
		signal(SIGTTIN, SIG_IGN);
		signal(SIGTTOU, SIG_IGN);
		signal(SIGCHLD, SIG_IGN);

		/* Put ourselves in our own process group. */
		shell_pgid = getpid();
		if (setpgid(shell_pgid, shell_pgid) < 0)
		{
			perror("Couldnt put the shell in its own process group");
			exit(1);
		}

		/* Grab control of the terminal. */
		tcsetpgrp(shell_terminal, shell_pgid);

		/* Save default terminal attributes for shell. */
		tcgetattr(shell_terminal, &shell_tmodes);
	}
}

/**
 * @brief Finds a job by its process group ID (PGID).
 * 
 * Searches the active job list for a job matching the specified PGID.
 * 
 * @param pgid The process group ID to search for.
 * @return Pointer to the job if found, NULL otherwise.
 */
job *find_job(pid_t pgid)
{
	job *j;

	for (j = first_job; j; j = j->next)
		if (j->pgid == pgid)
			return j;

	return NULL;
}

/**
 * @brief Checks if all processes in a job are stopped.
 * 
 * @param j The job to check.
 * @return 1 if all processes are stopped, 0 otherwise.
 */
int job_is_stopped(job *j)
{
	process *p;

	for (p = j->first_process; p; p = p->next)
		if (!p->completed && !p->stopped)
			return 0;
	return 1;
}

/**
 * @brief Checks if all processes in a job have completed.
 * 
 * @param j The job to check.
 * @return 1 if all processes are completed, 0 otherwise.
 */
int job_is_completed(job *j)
{
	process *p;

	for (p = j->first_process; p; p = p->next)
		if (!p->completed)
			return 0;
	return 1;
}

/**
 * @brief Launches a single process with redirection and signal handling.
 * 
 * Forks a new process, sets up I/O redirection, and executes the command.
 * 
 * @param p The process to launch.
 * @param pgid The process group ID (0 to create a new group).
 * @param infile Input file descriptor.
 * @param outfile Output file descriptor.
 * @param errfile Error file descriptor.
 * @param foreground 1 to run in foreground, 0 for background.
 */ 
void launch_process(process *p, pid_t pgid, int infile, int outfile, int errfile, int foreground)
{
	pid_t pid;
	/* Put the process into the process group and give the process group
				  the terminal, if appropriate.
				  This has to be done both by the shell and in the individual
				  child processes because of potential race conditions.  */
	if (shell_is_interactive)
	{
		pid = getpid();
		if (pgid == 0)
			pgid = pid;
		setpgid(pid, pgid);
		if (foreground)
			tcsetpgrp(shell_terminal, pgid);

		/* Set the handling for job contorl signals back to the default*/
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);
		signal(SIGTTIN, SIG_DFL);
		signal(SIGTTOU, SIG_DFL);
		signal(SIGCHLD, SIG_DFL);
	}

	/* Set the standard input/output channels of the new process. */
	if (infile != STDIN_FILENO)
	{
		dup2(infile, STDIN_FILENO);
		close(infile);
	}
	if (outfile != STDOUT_FILENO)
	{
		dup2(outfile, STDOUT_FILENO);
		close(outfile);
	}
	if (errfile != STDERR_FILENO)
	{
		dup2(errfile, STDERR_FILENO);
		close(errfile);
	}

	/* Exec the new process. Make sure we exit. */
	execvp(p->argv[0], p->argv);
	perror("execvp");
	exit(1);
}

/**
 * @brief Moves a job to the foreground.
 * 
 * Resumes the job if needed and waits for completion.
 * 
 * @param j The job to foreground.
 * @param cont 1 to send SIGCONT (resume), 0 otherwise.
 */
void put_job_in_foreground(job *j, int cont)
{
	/* Put job into the foreground */
	tcsetpgrp(shell_terminal, j->pgid);

	/* Send the job a continue signal, if necessary */
	if (cont)
	{
		tcsetattr(shell_terminal, TCSADRAIN, &j->tmodes);
		if (kill(-j->pgid, SIGCONT) < 0)
			perror("kill (SIGCONT)");
	}

	/* Wait for it to report. */
	wait_for_job(j);

	/* Put hte shell back in the foreground. */
	tcsetpgrp(shell_terminal, shell_pgid);

	/* Restore the shell's terminal modes. */
	tcgetattr(shell_terminal, &j->tmodes);
	tcsetattr(shell_terminal, TCSADRAIN, &shell_tmodes);
}

/**
 * @brief Moves a job to the background.
 * 
 * Resumes the job if needed without terminal control.
 * 
 * @param j The job to background.
 * @param cont 1 to send SIGCONT (resume), 0 otherwise.
 */
void put_job_in_background(job *j, int cont)
{
	/* Send the job a continue signal, if necessary. */
	if (cont)
		if (kill(-j->pgid, SIGCONT) < 0)
			perror("kill (SIGCONT)");
}

/**
 * @brief Marks a stopped job as running.
 * 
 * Clears the "stopped" flag for all processes in the job.
 * 
 * @param j The job to mark as running.
 */
void mark_job_as_running(job *j)
{
	process *p;

	for (p = j->first_process; p; p = p->next)
		p->stopped = 0;
	j->notified = 0;
}

/**
 * @brief Resumes a stopped job.
 * 
 * Continues the job in either foreground or background mode.
 * 
 * @param j The job to resume.
 * @param foreground 1 to run in foreground, 0 for background.
 */
void continue_job(job *j, int foreground)
{
	mark_job_as_running(j);
	if (foreground)
		put_job_in_foreground(j, 1);
	else
		put_job_in_background(j, 1);
}

/**
 * @brief Launches a job (forking processes, setting up pipes).
 * 
 * Handles process creation, I/O redirection, and foreground/background execution.
 * 
 * @param j The job to launch.
 * @param foreground 1 to run in foreground, 0 for background.
 */

void launch_job(job *j, int foreground)
{
	process *p;
	pid_t pid;
	int mypipe[2], infile, outfile;

	infile = j->stdin;
	for (p = j->first_process; p; p = p->next)
	{
		/*Set up pipes if necessary */
		if (p->next)
		{
			if (pipe(mypipe) < 0)
			{
				perror("pipe");
				exit(1);
			}
			outfile = mypipe[1];
		}
		else
			outfile = j->stdout;

		/* For the child proceses. */
		pid = fork();
		if (pid == 0)
		{
			/* This is the child process. */
			launch_process(p, j->pgid, infile, outfile, j->stderr, foreground);
		}
		else if (pid < 0)
		{
			/* The fork failed */
			perror("fork");
			exit(1);
		}
		else
		{
			/* This is the parent process. */
			p->pid = pid;
			if (shell_is_interactive)
			{
				if (!j->pgid)
					j->pgid = pid;
				setpgid(pid, j->pgid);
			}
		}

		/*Clean up after pipes */
		if (infile != j->stdin)
			close(infile);
		if (outfile != j->stdout)
			close(outfile);
		infile = mypipe[0];
	}

	//	format_job_info (j, "launched");
	if (!shell_is_interactive)
		wait_for_job(j);
	else if (foreground)
		put_job_in_foreground(j, 0);
	else
		put_job_in_background(j, 0);
}

/**
 * @brief Prints detailed information about a job.
 * 
 * Displays PGID, process IDs, and command arguments.
 * 
 * @param j The job to print.
 */
void print_job_info(job *j)
{
	fprintf(stderr, "Job ID: %ld\n", (long)j->pgid);
	process *p;
	for (p = j->first_process; p; p = p->next)
	{
		fprintf(stderr, "Process ID: %d\n", p->pid);
		for (int i = 0; p->argv[i] != NULL; i++)
		{
			fprintf(stderr, "%s ", p->argv[i]);
		}
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
}

/**
 * @brief Prints information about all active jobs.
 * 
 * Wrapper for print_job_info() on the entire job list.
 */
void print_jobs_info()
{
	job *j = first_job;
	if (j == NULL)
	{
		fprintf(stderr, "No jobs running.\n");
		return;
	}
	while (j != NULL)
	{
		print_job_info(j);
		j = j->next;
	}
}

/**
 * @brief Adds a job to the active job list.
 * 
 * @param new_job The job to add.
 */
void add_job(job *new_job)
{
	new_job->next = first_job;
	first_job = new_job;
}

/**
 * @brief Lists all active jobs (PGID and command).
 * 
 * Prints a compact overview of running/stopped jobs.
 */
void list_jobs()
{
	if (first_job == NULL)
	{
		printf("No jobs running.\n");
		return;
	}
	job *current = first_job;
	while (current != NULL)
	{

		printf("[%d] %s\n", current->pgid, current->command);
		current = current->next;
	}
}

/**
 * @brief Frees memory for a single job and its processes.
 * 
 * @param j The job to delete.
 */
void delete_job(job *j)
{
	if (j == NULL)
		return;

	process *p = j->first_process;
	while (p != NULL)
	{
		process *temp = p;
		p = p->next;
		free(temp->argv);
		free(temp);
	}
	free(j->command);
	free(j);
}

/**
 * @brief Cleans up all jobs in the global list.
 * 
 * Frees all job-related memory before shell exit.
 */
void delete_all_jobs()
{
	job *j = first_job;
	while (j != NULL)
	{
		job *temp = j;
		j = j->next;
		delete_job(temp);
	}
	first_job = NULL;
}
