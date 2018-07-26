/***************************************************************************/ /**

  @file         main.c

  @author       Marc Burt

  @date         Thursday,  10 July 2018

  @brief        BurtShell

*******************************************************************************/

#include <sys/wait.h>
#include <sys/types.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

struct Node
{
  char data;
  struct Node *next;
};

typedef struct Node *hist;

/*
  Function Declarations for builtin shell commands:
 */
int burt_cd(char **args);
int burt_help(char **args);
int burt_time(char **args);
int burt_history(char **args);
int burt_clear_history(char **args);
int burt_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */

char *builtin_str[] = {
    "cd",
    "help",
    "time",
    "history",
    "clear_hist",
    "exit"};

int (*builtin_func[])(char **) = {
    &burt_cd,
    &burt_help,
    &burt_time,
    &burt_history,
    &burt_clear_history,
    &burt_exit};

int burt_num_builtins()
{
  return sizeof(builtin_str) / sizeof(char *);
}

#define BUF 128 /* can change the buffer size as well */
#define TOT 10  /* change to accomodate other sizes, change ONCE here */

int burt_cd(char **args)
{
  if (args[1] == NULL)
  {
    fprintf(stderr, "burt: expected argument to \"cd\"\n");
  }
  else
  {
    if (chdir(args[1]) != 0)
    {
      perror("burt");
    }
  }
  return 1;
}

int burt_help(char **args)
{
  int i;
  printf("Marc Burt's BurtShell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < burt_num_builtins(); i++)
  {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int burt_time(char **args)
{
  time_t mytime = time(NULL);
  char *time_str = ctime(&mytime);
  time_str[strlen(time_str) - 1] = '\0';
  printf("Current Time : %s\n", time_str);

  return 1;
}

void add_hist(char *line)
{
  FILE *hist_file;
  char *h = "history";
  char *b = "!!";
  char *bn = "!";
  hist_file = fopen("/tmp/history.txt", "a");
  if (hist_file == NULL)
  {
    perror("Error opening file");
  }
  else
  {
    if (line != NULL && strlen(line) != 0 && strcmp(line, h) != 0 && strcmp(line, b) != 0 && line[0] != '!')
    {
      fprintf(hist_file, "%s\n", line);
    }
  }
  fclose(hist_file);
}

int burt_clear_history(char **args)
{
  FILE *hist_file;
  hist_file = fopen("/tmp/history.txt", "w");
  fclose(hist_file);
  return 1;
}

int burt_history(char **args)
{

  char line[TOT][BUF];
  FILE *hist_file = NULL;
  int i = 0;
  int total = 0;
  int size;

  hist_file = fopen("/tmp/history.txt", "r");
  if (NULL != hist_file)
  {
    fseek(hist_file, 0, SEEK_END);
    size = ftell(hist_file);
    if (0 == size)
    {
      printf("History is empty\n");
      return 1;
    }
  }
  fclose(hist_file);
  free(hist_file);
  hist_file = fopen("/tmp/history.txt", "r");
  while (fgets(line[i], BUF, hist_file))
  {
    /* get rid of ending \n from fgets */
    line[i][strlen(line[i]) - 1] = '\0';
    i++;
  }

  total = i;

  for (i = 0; i < total; ++i)
  {
    printf("%d  ", i + 1);
    printf("%s\n", line[i]);
  }
  fclose(hist_file);

  return 1;
}

int burt_exit(char **args)
{
  return 0;
}

int burt_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0)
  {
    // Child process
    if (execvp(args[0], args) == -1)
    {
      perror("burt");
    }
    exit(EXIT_FAILURE);
  }
  else if (pid < 0)
  {
    // Error forking
    perror("burt");
  }
  else
  {
    // Parent process
    do
    {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int run_last_hist(char **args);
char **burt_split_line(char *line);

int burt_execute(char **args)
{
  int i;
  char *bang = "!!";
  if (args[0] == NULL)
  {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < burt_num_builtins(); i++)
  {
    if (strcmp(args[0], builtin_str[i]) == 0)
    {
      return (*builtin_func[i])(args);
    }
    if (strcmp(args[0], bang) == 0)
    {
      return run_last_hist(args);
    }
  }

  return burt_launch(args);
}

int run_last_hist(char **args)
{
  char line[TOT][BUF];
  FILE *hist_file = NULL;
  int i = 0;
  int x = 0;
  int total = 0;
  char **hist_args;

  hist_file = fopen("/tmp/history.txt", "r");
  while (fgets(line[i], BUF, hist_file))
  {
    /* get rid of ending \n from fgets */
    line[i][strlen(line[i]) - 1] = '\0';
    i++;
  }

  for (x = 0; x < burt_num_builtins(); x++)
  {
    if (strcmp(builtin_str[x], line[i - 1]) == 0)
    {
      return (*builtin_func[x])(args);
    }
  }
  hist_args = burt_split_line(line[i - 1]);
  printf("hist_args: %s\n", hist_args[0]);
  return burt_launch(hist_args);
}

#define BURT_RL_BUFSIZE 1024

char *burt_read_line()
{
  int bufsize = BURT_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;
  if (!buffer)
  {
    fprintf(stderr, "burt: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1)
  {
    // Read a character
    c = getchar();

    if (c == EOF)
    {
      exit(EXIT_SUCCESS);
    }
    else if (c == '\n')
    {
      buffer[position] = '\0';
      return buffer;
    }
    else
    {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize)
    {
      bufsize += BURT_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer)
      {
        fprintf(stderr, "burt: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define BURT_TOK_BUFSIZE 64
#define BURT_TOK_DELIM " \t\r\n\a"

char **burt_split_line(char *line)
{
  int bufsize = BURT_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token, **tokens_backup;
  char hist_token;

  if (!tokens)
  {
    fprintf(stderr, "burt: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, BURT_TOK_DELIM);
  while (token != NULL)
  {
    tokens[position] = token;
    position++;
    // printf("%s\n", tokens[position]);

    if (position >= bufsize)
    {
      bufsize += BURT_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens)
      {
        free(tokens_backup);
        fprintf(stderr, "burt: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, BURT_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

void burt_loop(void)
{
  char *line;
  char **args;
  int status;
  burt_clear_history(args);
  do
  {
    printf("MB> ");
    line = burt_read_line();
    add_hist(line);
    args = burt_split_line(line);
    status = burt_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  burt_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
