// Skylar Wells
// jwel929

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void runCommand(char **userArgs, char *constantFullCommand, char *OGdirectory, char **historyCommands);
void addToJobs(char *constantFullCommand, pid_t parseID);

int historyAmount = 0;
int processCounter = 1;
int jobsRunning = 0;
pid_t *jobIDs;
pid_t frontJob = -1;
pid_t mostRecentJob = -1;
char *currentCommand;
char **jobCommandsAmper;
void jobStates();


char *readLine(void)
{
   // If running from a terminal, show ash before typing
   if (isatty(STDIN_FILENO))
   {
      printf("ash> ");
   }
   
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
         }

         if (isatty(STDIN_FILENO))
         {
            // Code if terminal
         }
         // If running from a file, show ash> "command" after it's entered
         else
         {
            printf("ash> %s\n", fullLine);
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

void deleteJob(long jobNum)
{
   jobsRunning--;
   jobIDs[jobNum] = 0;
   if (jobNum == processCounter)
   {
      processCounter--;
   }
}

void signalHandler(int signalNumber)
{
   if (frontJob != -1)
   {
      kill(frontJob, SIGTSTP);
      addToJobs(currentCommand, frontJob);
      mostRecentJob = frontJob;
   }
   else
   {
      printf("\nNo foreground job\nash> ");
      fflush(stdout);
   }

   signal(SIGTSTP, signalHandler);
}

void wakeBackground(char *processNumInput)
{
   char *words = strdup(processNumInput);
   char *extraString = calloc(1000, 1);

   long processNum = strtol(words, &extraString, 10);

   if (processNum == 0)
   {
      kill(mostRecentJob, SIGCONT);
   }
   else
   {
      kill(jobIDs[processNum], SIGCONT);
   }

}

void wakeForeground(char *processNumInput)
{
   char *words = strdup(processNumInput);
   char *extraString = calloc(1000, 1);
   long processNum = strtol(words, &extraString, 10);

   if (processNum == 0)
   {
      currentCommand = jobCommandsAmper[processCounter];
      kill(mostRecentJob, SIGCONT);
      frontJob = mostRecentJob;
      deleteJob(processCounter);
   }
   else
   {
      currentCommand = jobCommandsAmper[processNum];
      kill(jobIDs[processNum], SIGCONT);
      frontJob = jobIDs[processNum];
      deleteJob(processNum);
   }

   int errorCode;
   waitpid(frontJob, &errorCode, WUNTRACED);
   frontJob = -1;

}

void killCertainProcess(char *processNumInput)
{
   char *words = strdup(processNumInput);
   char *extraString = calloc(1000, 1);
   long processNum = strtol(words, &extraString, 10);

   if (processNum == 0)
   {
      kill(mostRecentJob, SIGKILL);
      deleteJob(processCounter);
   }
   else
   {
      kill(jobIDs[processNum], SIGKILL);
      deleteJob(processNum);
   }

}

void addToJobs(char *constantFullCommand, pid_t parseID)
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
   int errCode;
   errCode = chdir(userString);
   if (errCode == -1)
   {
      printf("cd: %s: No such file or directory\n", userString);
   }
}

void executeAndWait(char **userArgs)
{
   frontJob = fork();

   if (frontJob == 0)
   {
      execvp(userArgs[0], userArgs);
   }

   else
   {
      int errorCode;
      waitpid(frontJob, &errorCode, WUNTRACED);
      frontJob = -1;
   }
}

void executeAndDontWait(char **userArgs, char *fullCommand)
{

   pid_t childID = fork();

   if (childID != 0)
   {
      addToJobs(fullCommand, childID);
   }

   if (childID == 0)
   {
      execvp(userArgs[0], userArgs);
      exit(0);
   }
   else
   {
      return;
   }
}

void jobStates()
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

   for (int i = processCounter; i > 0; i--)
   {
      if (jobIDs[i] == 0)
      {
         processCounter--;
      }
      else
      {
         break;
      }
   }


}

void historyCommand(char *commandType, char **historyOfUser, char *OGdirectory)
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

      runCommand(historyArgs, fullCommandWanted, OGdirectory, historyOfUser);
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

void executeWithPipes(char ***userArgs, int amper, int pipeNum, char *fullCommand)
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
         addToJobs(fullCommand, amperFork);
      }
   }
}

char ***createPipeArgInput(char **userArgs, int amper, char *constantFullCommand)
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

   executeWithPipes(pipedArgs, amper, maxPipes, constantFullCommand);
}

char fileRead(pid_t jobToCheck)
{
   FILE *statusFile;
   char fileName[1000];
   sprintf(fileName, "/proc/%d/stat", jobToCheck);

   statusFile = fopen(fileName, "r");

   char status;
   // Courtesy Chris Dodd at https://stackoverflow.com/a/25883231
   fscanf(statusFile, "%*s %*[^)]) %c", &status);

   fclose(statusFile);

   return status;
}

char *whichState(char stateChar)
{
   char *jobStatus = calloc(16384, 1);

   switch(stateChar)
   {
      case 'S':
         jobStatus = "Sleeping";
         break;

      case 'R':
         jobStatus = "Runnable";
         break;

      case 'T':
         jobStatus = "Stopped";
         break;

      case 'I':
         jobStatus = "Idle";
         break;

      case 'Z':
         jobStatus = "Zombie";
         break;

      case 'D':
         jobStatus = "Disk Sleep";
         break;

      case 't':
         jobStatus = "Tracing Stop";
         break;

      case 'X':
         jobStatus = "Dead";
         break;

      case 'P':
         jobStatus = "Parked";
         break;

      default:
         jobStatus = "Unknown";
         break;
   }

   return jobStatus;
}

void jobCommand()
{

   for (int i = 1; i <= processCounter; i++)
   {
      if (jobIDs[i] != 0)
      {
         char stateCharacter = fileRead(jobIDs[i]);
         char *stateOfJob = calloc(1000, 1);
         stateOfJob = whichState(stateCharacter);
         printf("[%d] <%s> %s\n", i, stateOfJob, jobCommandsAmper[i]);
      }
   }
}

void runCommand(char **userArgs, char *constantFullCommand, char *OGdirectory,
                char **historyCommands)
{

   // Checking if ampersand
   int amperValue = amperCheck(userArgs);
   // printf("Amper value: %d\n", amperValue);

   // Checking pipe locations and if pipe
   int pipeExists = pipeCheck(userArgs);

   // Adding this to command history
   if (strcmp(userArgs[0], "history") == 0 && userArgs[1] != NULL || strcmp(userArgs[0], "h") == 0 && userArgs[1] != NULL)
   {
      // We do not add "h #" commands to history
   }
   else
   {
      addToHistory(constantFullCommand, historyCommands);
   }

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
      historyCommand(userArgs[1], historyCommands, OGdirectory);
   }

   // If command is jobs
   else if (strcmp(userArgs[0], "jobs") == 0)
   {
      jobCommand();
   }

   else if (strcmp(userArgs[0], "bg") == 0)
   {
      if (userArgs[1] == NULL)
      {
         wakeBackground("0");
      }
      else
      {
         wakeBackground(userArgs[1]);
      }
   }

   else if (strcmp(userArgs[0], "fg") == 0)
   {
      if (userArgs[1] == NULL)
      {
         wakeForeground("0");
      }
      else
      {
         wakeForeground(userArgs[1]);
      }

   }

   else if (strcmp(userArgs[0], "kill") == 0)
   {
      if (userArgs[1] == NULL)
      {
         killCertainProcess("0");
      }
      else
      {
         killCertainProcess(userArgs[1]);
      }

   }

   // Run built-in normal commands
   else
   {
      if (pipeExists == 1)
      {
         createPipeArgInput(userArgs, amperValue, constantFullCommand);
      }
      else if (amperValue == 1)
      {
         executeAndDontWait(userArgs, constantFullCommand);
      }
      else
      {
         executeAndWait(userArgs);
      }
   }
}

int main()
{

   signal(SIGTSTP, signalHandler);

   jobCommandsAmper = calloc(16384, 1);

   currentCommand = calloc(16384, 1);

   jobIDs = calloc(16384, 1);

   // History variable
   char **historyCommands = calloc(16384, 1);

   // Char where we will store users full command line
   char *fullCommand = calloc(16384, 1);

   // attaching program run
   char *OGdirectory = calloc(16384, 1);
   getcwd(OGdirectory, 1024);

   while (strcmp(fullCommand, "exit") != 0)
   {
      jobStates();

      // Resetting the users old line
      memset(fullCommand, 0, sizeof(fullCommand));

      // Taking the users full line
      fullCommand = readLine();

      // Taking a constant version of full line
      char *constantFullCommand = calloc(16384, 1);
      strcpy(constantFullCommand, fullCommand);
      currentCommand = constantFullCommand;

      // Resetting an args 2d array
      char **userArgs = calloc(16384, 1);
      // Filling the array with users args
      splitToArgs(fullCommand, userArgs);

      runCommand(userArgs, constantFullCommand, OGdirectory, historyCommands);

      free(userArgs);
   }

   return 0;
}