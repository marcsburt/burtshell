#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <dirent.h>

#define MAX_CMD_LEN 128

#define HISTORY_COUNT 10

int user_history(char *hist[], int current)

{

  int i = current;

  int hist_num = 1;

  do
  {

    if (hist[i])

    {

      printf("%4d %s\n", hist_num, hist[i]);

      hist_num++;
    }

    i = (i + 1) % HISTORY_COUNT;

  } while (i != current);

  return 0;
}

int clear_user_history(char *hist[])

{

  int i;

  for (i = 0; i < HISTORY_COUNT; i++)

  {

    free(hist[i]);

    hist[i] = NULL;
  }

  return 0;
}

int cd(char *pth)

{

  char path[MAX_CMD_LEN];

  strcpy(path, pth);

  char cwd[MAX_CMD_LEN];

  getcwd(cwd, sizeof(cwd));

  strcat(cwd, "/");

  strcat(cwd, path);

  printf("cd location before chdir is : %s\n", path);

  int ret = chdir(path);

  if (ret != 0)

    printf("Error: Could not change to the directory \n");

  else

    system("pwd");

  return 0;
}

int hasPrefix(char const *p, char const *q)

{

  int i = 0;

  for (i = 0; q[i]; i++)

  {

    if (p[i] != q[i])

      return -1;
  }

  return 0;
}

void run_ls_command()

{

  char *curr_dir = NULL;

  DIR *dp = NULL;

  struct dirent *dptr = NULL;

  unsigned int count = 0;

  curr_dir = getenv("PWD");

  if (NULL == curr_dir)

  {

    printf("\n ERROR : Could not get the working directory\n");

    return;
  }

  dp = opendir((const char *)curr_dir);

  if (NULL == dp)

  {

    printf("\n ERROR : Could not open the working directory\n");
  }

  printf("\n");

  for (count = 0; NULL != (dptr = readdir(dp)); count++)

  {

    // Check if the name of the file/folder begins with '.'

    // If yes, then do not display it.

    if (dptr->d_name[0] != '.')

      printf("%s ", dptr->d_name);
  }

  printf("\n");
}

int main()

{

  char cmd[MAX_CMD_LEN];

  char *hist[HISTORY_COUNT];

  char buf[1000];

  int i, current = 0;

  for (i = 0; i < HISTORY_COUNT; i++)

    hist[i] = NULL;

  char *tok;

  tok = strtok(cmd, " ");

  while (1)

  {

    bzero(cmd, MAX_CMD_LEN);

    printf("user@shell> ");

    fgets(cmd, MAX_CMD_LEN, stdin);

    if (cmd[strlen(cmd) - 1] == '\n')

    {

      cmd[strlen(cmd) - 1] = '\0';
    }

    if (hasPrefix(cmd, "cd") == 0)

    {

      tok = strchr(cmd, ' ');

      if (tok)

      {

        char *tempTok = tok + 1;

        tok = tempTok;

        char *locationOfNewLine = strchr(tok, '\n');

        if (locationOfNewLine)

        {

          *locationOfNewLine = '\0';
        }

        printf("cd location is : %s\n", tok);

        cd(tok);
      }

      else

      {

        system("pwd");
      }
    }

    free(hist[current]);

    hist[current] = strdup(cmd);

    current = (current + 1) % HISTORY_COUNT;

    if (strcmp(cmd, "history") == 0)

      user_history(hist, current);

    else if (strcmp(cmd, "hc") == 0)

      clear_user_history(hist);

    else if (strcmp(cmd, "exit") == 0)

      exit(0);

    else if (strcmp(cmd, "pwd") == 0)

    {

      char *path = getcwd(buf, 100);

      printf("%s\n", path);
    }

    else if (strcmp(cmd, "ls") == 0)

      run_ls_command();
  }

  clear_user_history(hist);

  return 0;
}