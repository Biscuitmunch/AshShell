#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void runCommand(char **userArgs, char *constantFullCommand, char *OGdirectory, char **historyCommands, char **jobsCommandAmper, pid_t *jobIDs);

int historyAmount = 0;
int processCounter = 1;
int jobsRunning = 0;

char *readLine(void)
{
   printf("ash> ");
   char *fullLine = calloc(16384, 1);
   int adder = 0;

   while (1)
   {

      char in = fgetc(stdin);

      if (in == EOF || in == '\n')
      {
         // ending on null byte
         fullLine[adder] = '\0';

         if (fullLine[0] == '\0')
         {
            exit(0);
            printf("ash> ");
            continue;
         }

         return fullLine;
      }

      else
      {

         fullLine[adder] = in;
         adder++;
      }
   }
}

void addToJobs(char **jobCommandsAmper, char *constantFullCommand, pid_t *jobIDs, pid_t parseID)
{
   if (jobsRunning == 0)
   {
      processCounter = 1;
      jobsRunning++;
      jobCommandsAmper[processCounter] = strdup(constantFullCommand);
      jobIDs[processCounter] = parseID;
      printf("[%d] %d\n", processCounter, parseID);
   }
   else
   {
      jobsRunning++;
      processCounter++;
      int countdown = processCounter;
      while (jobIDs[countdown] == '\0')
      {
         countdown--;
      }
      countdown++;
      jobCommandsAmper[countdown] = strdup(constantFullCommand);
      jobIDs[countdown] = parseID;
      printf("[%d] %d\n", processCounter, parseID);
   }
}

void splitToArgs(char *fullLine, char **userArgs)
{

   char *stringToken = strtok(fullLine, " ");

   int i = 0;

   while (stringToken != NULL)
   {
      userArgs[i] = stringToken;
      stringToken = strtok(NULL, " ");
      i++;
   }
}

void cdCommand(char *userString)
{
   chdir(userString);
}

void executeAndWait(char **userArgs)
{
   pid_t childID = fork();

   if (childID == 0)
   {
      execvp(userArgs[0], userArgs);
   }

   else
   {
      int errorCode;
      waitpid(childID, &errorCode, 0);
   }
}

void executeAndDontWait(char **userArgs, char *fullCommand, char **jobCommandsAmper, pid_t *jobIDs)
{

   pid_t childID = fork();

   if (childID != 0)
   {
      addToJobs(jobCommandsAmper, fullCommand, jobIDs, childID);
   }

   if (childID == 0)
   {
      execvp(userArgs[0], userArgs);
      exit(0);

      // int processCounter = 1;

      // char *keepCommand = calloc(16384, 1);
      // strcpy(keepCommand, fullCommand);

      // while (amperProcesses[processCounter]!=0)
      // {
      //    processCounter++;
      // }

      // amperProcesses[processCounter] = processCounter;

      // printf("\b\b\b\b\b[%d] %d\nash> ", processCounter, getpid());
      // fflush(stdout);

      // int errorCode;
      // waitpid(childID, &errorCode, 0);

      // printf("\n[%d] <Done> %s\nash> ", processCounter, keepCommand);
      // exit(0);
      //}
   }

   else
   {
      return;
   }
}

void jobStates(pid_t *jobIDs, char **jobCommandsAmper)
{

   for (int i = 0; i <= processCounter; i++)
   {
      if (jobIDs[i] == 0)
      {
         continue;
      }
      else
      {
         int state;
         pid_t statusID = waitpid(jobIDs[i], &state, WNOHANG);
         if (statusID == jobIDs[i])
         {
            printf("[%d] <Done> %s\n", i, jobCommandsAmper[i]);
            jobsRunning--;
            jobIDs[i] = 0;
         }
      }
   }
}

void historyCommand(char *commandType, char **historyOfUser, char *OGdirectory, char **jobsCommandAmper, pid_t *jobIDs)
{
   if (historyOfUser[0] == NULL)
   {
      printf("No history\n");
      return;
   }

   if (commandType == NULL && historyAmount < 11)
   {
      for (int i = 1; i <= historyAmount; i++)
      {
         printf("%3d: %s\n", i, historyOfUser[i - 1]);

         if (historyOfUser[i] == NULL)
         {
            break;
         }
      }
   }
   else if (commandType == NULL)
   {
      for (int i = historyAmount - 9; i <= historyAmount; i++)
      {
         printf("%3d: %s\n", i, historyOfUser[i - 1]);

         if (historyOfUser[i] == NULL)
         {
            break;
         }
      }
   }

   else
   {
      // Getting the history value wanted
      char *extraString;
      char *historyWanted = calloc(16384, 1);
      strcpy(historyWanted, commandType);

      // Converting to int to get the history string
      long histNum = strtol(historyWanted, &extraString, 10);
      char *fullCommandWanted = calloc(16384, 1);

      // if doesnt exist, return
      if (historyOfUser[histNum - 1] == NULL)
      {
         printf("That value has no history!\n");
         return;
      }

      strcpy(fullCommandWanted, historyOfUser[histNum - 1]);

      // Running the history command
      char **historyArgs = calloc(16384, 1);
      splitToArgs(fullCommandWanted, historyArgs);

      runCommand(historyArgs, fullCommandWanted, OGdirectory, historyOfUser, jobsCommandAmper, jobIDs);
   }
}

void addToHistory(char *userString, char **historyOfUser)
{

   char *temp = calloc(16384, 1);
   strcpy(temp, userString);

   historyOfUser[historyAmount] = temp;

   historyAmount++;
}

int amperCheck(char **userArgs)
{
   int count = 0;
   while (userArgs[count] != NULL)
   {
      count++;
   }
   if (strcmp(userArgs[count - 1], "&") == 0)
   {
      userArgs[count - 1] = '\0';
      return 1;
   }
   else
   {
      return 0;
   }
}

int pipeCheck(char **userArgs)
{
   int count = 1;
   while (userArgs[count] != NULL)
   {
      if (strcmp(userArgs[count - 1], "|") == 0)
      {
         return 1;
      }
      count++;
   }

   return 0;
}

void shutOff(int pipeNum, int fildes[pipeNum][2])
{
   for (int i = 0; i < pipeNum; i++)
   {
      close(fildes[i][0]);
      close(fildes[i][1]);
   }
}

void executeWithPipes(char ***userArgs, int amper, int pipeNum, char *fullCommand, char **jobCommandsAmper, pid_t *jobIDs)
{
   pid_t *allChildren = calloc(16384, 1);

   int fildes[pipeNum][2];

   for (int i = 0; i < pipeNum; i++)
   {
      pipe(fildes[i]);
   }

   allChildren[0] = fork();

   if (allChildren[0] == 0)
   {
      dup2(fildes[0][1], STDOUT_FILENO);
      shutOff(pipeNum, fildes);
      execvp(userArgs[0][0], userArgs[0]);
      exit(0);
   }

   for (int i = 1; i < pipeNum; i++)
   {
      allChildren[i] = fork();
      if (allChildren[i] == 0)
      {
         dup2(fildes[i - 1][0], STDIN_FILENO);
         dup2(fildes[i][1], STDOUT_FILENO);
         shutOff(pipeNum, fildes);

         execvp(userArgs[i][0], userArgs[i]);
         exit(0);
      }
   }

   allChildren[pipeNum] = fork();

   if (allChildren[pipeNum] == 0)
   {
      dup2(fildes[pipeNum - 1][0], STDIN_FILENO);
      shutOff(pipeNum, fildes);
      execvp(userArgs[pipeNum][0], userArgs[pipeNum]);
      exit(0);
   }

   int errCode;
   shutOff(pipeNum, fildes);
   if (amper == 0)
   {
      for (int i = 0; i < pipeNum; i++)
      {
         waitpid(allChildren[i], &errCode, 0);
      }
   }
   else
   {
      pid_t amperFork = fork();

      if (amperFork == 0)
      {
         for (int i = 0; i < pipeNum; i++)
         {
            waitpid(allChildren[i], &errCode, 0);
         }
         exit(0);
      }

      else
      {
         addToJobs(jobCommandsAmper, fullCommand, jobIDs, amperFork);
      }
   }
}

char ***createPipeArgInput(char **userArgs, int amper, char *constantFullCommand, char **jobCommandsAmper, pid_t *jobIDs)
{
   int buffsize1 = 10;
   int buffsize2 = 10;
   char ***pipedArgs = malloc(buffsize1 * sizeof(char **));
   *pipedArgs = malloc(buffsize2 * sizeof(char *));
   int count = 0;
   int i = 0;
   int j = 0;
   int maxPipes = 0;
   while (userArgs[j] != NULL)
   {
      // printf("%s\n", userArgs[j]);

      if (strcmp(userArgs[j], "|") == 0)
      {
         // printf("found a pipe");
         i++;
         pipedArgs[count][i] = NULL;
         maxPipes++;
         j++;
         count++;
         if (count >= buffsize1)
         {
            buffsize1 += buffsize1;
            pipedArgs = realloc(pipedArgs, buffsize1 * sizeof(char **));
         }
         i = 0;
         buffsize2 = 10;
         pipedArgs[count] = malloc(buffsize2 * sizeof(char *));
         continue;
      }

      // printf("adding %s at %d to %d", userArgs[j], i, count);
      pipedArgs[count][i] = malloc(sizeof(char) * strlen(userArgs[j]));
      strcpy(pipedArgs[count][i], userArgs[j]);
      j++;
      i++;
      if (i >= buffsize2)
      {
         buffsize2 += buffsize2;
         pipedArgs[count] = realloc(pipedArgs[count], buffsize2 * sizeof(char *));
      }
   }

   pipedArgs[count][i] = NULL;

   executeWithPipes(pipedArgs, amper, maxPipes, constantFullCommand, jobCommandsAmper, jobIDs);
}

void jobCommand(uid_t *jobIDs, char **jobCommandsAmper)
{
   for (int i = 1; i <= processCounter; i++)
   {
      if (jobIDs[i] != 0)
      {
         printf("[%d] %d %s\n", i, jobIDs[i], jobCommandsAmper[i]);
      }
   }
}

void runCommand(char **userArgs, char *constantFullCommand, char *OGdirectory,
                char **historyCommands, char **jobCommandsAmper, pid_t *jobIDs)
{

   // Checking if ampersand
   int amperValue = amperCheck(userArgs);
   // printf("Amper value: %d\n", amperValue);

   // Checking pipe locations and if pipe
   int pipeExists = pipeCheck(userArgs);

   // Adding this to command history
   addToHistory(constantFullCommand, historyCommands);

   // If command is cd
   if (strcmp(userArgs[0], "cd") == 0)
   {
      if (userArgs[1] == NULL)
      {
         cdCommand(OGdirectory);
      }
      else
      {
         cdCommand(userArgs[1]);
      }
   }

   // If command is history
   else if (strcmp(userArgs[0], "history") == 0 || strcmp(userArgs[0], "h") == 0)
   {
      historyCommand(userArgs[1], historyCommands, OGdirectory, jobCommandsAmper, jobIDs);
   }

   // If command is jobs
   if (strcmp(userArgs[0], "jobs") == 0)
   {
      jobCommand(jobIDs, jobCommandsAmper);
   }

   // Run built-in normal commands
   else
   {
      if (pipeExists == 1)
      {
         createPipeArgInput(userArgs, amperValue, constantFullCommand, jobCommandsAmper, jobIDs);
      }
      else if (amperValue == 1)
      {
         executeAndDontWait(userArgs, constantFullCommand, jobCommandsAmper, jobIDs);
      }
      else
      {
         executeAndWait(userArgs);
      }
   }
}

int main()
{

   char **jobsCommandAmper = calloc(16384, 1);

   pid_t *jobIDs = calloc(16384, 1);

   // History variable
   char **historyCommands = calloc(16384, 1);

   // Char where we will store users full command line
   char *fullCommand = calloc(16384, 1);

   // attaching program run
   char *OGdirectory = calloc(16384, 1);
   getcwd(OGdirectory, 1000);

   while (strcmp(fullCommand, "exit") != 0)
   {
      jobStates(jobIDs, jobsCommandAmper);

      // Resetting the users old line
      memset(fullCommand, 0, sizeof(fullCommand));

      // Taking the users full line
      fullCommand = readLine();

      // Taking a constant version of full line
      char *constantFullCommand = calloc(16384, 1);
      strcpy(constantFullCommand, fullCommand);

      // Resetting an args 2d array
      char **userArgs = calloc(16384, 1);
      // Filling the array with users args
      splitToArgs(fullCommand, userArgs);

      runCommand(userArgs, constantFullCommand, OGdirectory, historyCommands, jobsCommandAmper, jobIDs);

      free(userArgs);
   }

   return 0;
}