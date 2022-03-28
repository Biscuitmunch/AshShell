#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void runCommand(char **userArgs, char *constantFullCommand, char *OGdirectory,
 int *amperProcesses, char **historyCommands);

int historyAmount = 0;

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

void executeAndDontWait(char **userArgs, int *amperProcesses, char *fullCommand)
{
   pid_t childID = fork();

   if (childID == 0)
   {
      pid_t runnerID = fork();

      if (runnerID == 0)
      {
         execvp(userArgs[0], userArgs);
      }

      else
      {
         int processCounter = 1;
         
         char *keepCommand = calloc(16384, 1);
         strcpy(keepCommand, fullCommand);

         while (amperProcesses[processCounter]!=0)
         {
            processCounter++;
         }

         amperProcesses[processCounter] = processCounter;

         printf("\b\b\b\b\b[%d] %d\nash> ", processCounter, getpid());
         fflush(stdout);

         int errorCode;
         waitpid(childID, &errorCode, 0);

         printf("\n[%d] <Done> %s\nash> ", processCounter, keepCommand);
         exit(0);
      }

   }

   else
   {
      return;
   }
}

void historyCommand(char *commandType, char **historyOfUser, char *OGdirectory, int *amperProcesses)
{
   if (historyOfUser[0]==NULL)
   {
      printf("No history\n");
      return;
   }
   
   if (commandType==NULL && historyAmount < 11)
   {
      for (int i = 1; i <= historyAmount; i++)
      {
         printf("%d: %s\n", i, historyOfUser[i-1]);

         if (historyOfUser[i]==NULL)
         {
            break;
         }
      }
   }
   else if (commandType==NULL)
   {
      for (int i = historyAmount-9; i <= historyAmount; i++)
      {
         printf("%d: %s\n", i, historyOfUser[i-1]);

         if (historyOfUser[i]==NULL)
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
      if (historyOfUser[histNum-1]==NULL)
      {
         printf("That value has no history!\n");
         return;
      }

      strcpy(fullCommandWanted, historyOfUser[histNum-1]);

      // Running the history command
      char **historyArgs = calloc(16384, 1);
      splitToArgs(fullCommandWanted, historyArgs);
      
      runCommand(historyArgs, fullCommandWanted, OGdirectory, amperProcesses, historyOfUser);

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
   while (userArgs[count]!=NULL)
   {
      count++;
   }
   if (strcmp(userArgs[count-1], "&") == 0)
   {
      userArgs[count-1] = '\0';
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
   while (userArgs[count]!=NULL)
   {
      if (strcmp(userArgs[count-1], "|") == 0)
      {
         return 1;
      }
      count++;
   }

   return 0;

}


void executeWithPipes(char ***userArgs, int amper, int pipeNum)
{
   pid_t child1, child2;
   int fildes[2];

   pipeNum--;

   if (pipeNum == 0)
   {
      execvp(userArgs[pipeNum][0], userArgs[pipeNum]);
      exit(0);
   }
   //pipeNum = 1;

   pipe(fildes);
   child1 = fork();
   if (child1 == 0)
   {
      // printf("hello1v2\n");
      // printf("%s\n", userArgs[pipeNum-1][0]);
      // printf("%s\n", userArgs[pipeNum-1][1]);
      dup2(fildes[1], STDOUT_FILENO);
      close(fildes[0]);
      close(fildes[1]);

      executeWithPipes(userArgs, amper, pipeNum);

   }
   else
   {

      int errorMsg;
      dup2(fildes[0], STDIN_FILENO);
      close(fildes[0]);
      close(fildes[1]);
      waitpid(child1, &errorMsg, 0);

      execvp(userArgs[pipeNum][0], userArgs[pipeNum]);
      exit(0);
      
      // Creating a second child under the same parent
      // child2 = fork();

      // if (child2 == 0)
      // {
      //    dup2(fildes[0], STDIN_FILENO);
      //    close(fildes[0]);
      //    close(fildes[1]);
      //    // printf("hello2\n");
      //    // printf("%s\n", userArgs[pipeNum][0]);
      //    // printf("%s\n", userArgs[pipeNum][1]);
      //    execvp(userArgs[pipeNum][0], userArgs[pipeNum]);
      //    // printf("hello2v2\n");
      //    exit(0);
      // }
      // else
      // {
      //    // printf("hello3\n");

      //    close(fildes[0]);
      //    close(fildes[1]);
      //    int errorMsg;
      //    waitpid(child2, &errorMsg, 0);
      // }

   }



}

char ***createPipeArgInput(char **userArgs, int amper)
{
   int buffsize1 = 10;
   int buffsize2 = 10;
   char ***pipedArgs = malloc(buffsize1*sizeof(char **));
   *pipedArgs = malloc(buffsize2*sizeof(char *));
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
            pipedArgs = realloc(pipedArgs, buffsize1*sizeof(char **));
         }
         i = 0;
         buffsize2 = 10;
         pipedArgs[count] = malloc(buffsize2*sizeof(char*));
         continue;

      }
      
      // printf("adding %s at %d to %d", userArgs[j], i, count);
      pipedArgs[count][i] = malloc(sizeof(char)*strlen(userArgs[j]));
      strcpy(pipedArgs[count][i], userArgs[j]);
      j++;
      i++;
      if (i >= buffsize2)
      {
         buffsize2 += buffsize2;
         pipedArgs[count] = realloc(pipedArgs[count], buffsize2*sizeof(char *));
      }

   }

   pipedArgs[count][i] = NULL;

   int numOfPipes = 0;


   
   pid_t runningPipeCommand = fork();

   if (runningPipeCommand == 0)
   {
      executeWithPipes(pipedArgs, amper, maxPipes + 1);
   }
   else
   {
      if (amper == 0)
      {
         int errorMsg;
         waitpid(runningPipeCommand, &errorMsg, 0);
      }
   }

}

void runCommand(char **userArgs, char *constantFullCommand, char *OGdirectory,
 int *amperProcesses, char **historyCommands)
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
      historyCommand(userArgs[1], historyCommands, OGdirectory, amperProcesses);
   }

   // Run built-in normal commands
   else
   {
      if (pipeExists == 1)
      {
         createPipeArgInput(userArgs, amperValue);
      }
      else if (amperValue == 1)
      {
         executeAndDontWait(userArgs, amperProcesses, constantFullCommand);
      }
      else
      {
         executeAndWait(userArgs);
      }
   }
}

int main()
{
   // process control
   int *amperProcesses = calloc(16384, 1);

   // History variable
   char **historyCommands = calloc(16384, 1);

   // Char where we will store users full command line
   char *fullCommand = calloc(16384, 1);

   // attaching program run
   char *OGdirectory = calloc(16384, 1);
   getcwd(OGdirectory, 1000);

   while (strcmp(fullCommand, "exit") != 0)
   {
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

      runCommand(userArgs, constantFullCommand, OGdirectory, amperProcesses, historyCommands);

      free(userArgs);

   }

   return 0;
}