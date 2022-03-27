#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int historyAmount = 0;

char *readLine(void)
{
   printf("ash> ");
   char *fullLine = malloc(1000);
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

void historyCommand(char *commandType, char **historyOfUser)
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
      char *historyWanted = malloc(1000);
      strcpy(historyWanted, commandType);

      // Converting to int to get the history string
      long histNum = strtol(historyWanted, &extraString, 10);
      char *fullCommandWanted = malloc(1000);

      // if doesnt exist, return
      if (historyOfUser[histNum-1]==NULL)
      {
         printf("That value has no history!\n");
         return;
      }

      strcpy(fullCommandWanted, historyOfUser[histNum-1]);

      // Running the history command
      char **historyArgs = malloc(1000);
      splitToArgs(fullCommandWanted, historyArgs);
      
      executeAndWait(historyArgs);

      free(historyArgs);
   }


}

void addToHistory(char *userString, char **historyOfUser)
{

   char *temp = malloc(1000);
   strcpy(temp, userString);

   historyOfUser[historyAmount] = temp;

   historyAmount++;
}

int main()
{

   // Char where we will store users full command line
   char *fullCommand = malloc(1000);

   // attaching program run
   char *OGdirectory = malloc(1000);
   getcwd(OGdirectory, 1000);

   // History variable
   char **historyCommands = malloc(1000);

   while (strcmp(fullCommand, "exit") != 0)
   {
      // Resetting the users old line
      memset(fullCommand, 0, sizeof(fullCommand));

      // Taking the users full line
      fullCommand = readLine();

      // Taking a constant version of full line
      char *constantFullCommand = malloc(1000);
      strcpy(constantFullCommand, fullCommand); 

      // Resetting an args 2d array
      char **userArgs = malloc(1000);
      // Filling the array with users args
      splitToArgs(fullCommand, userArgs);


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
         historyCommand(userArgs[1], historyCommands);
      }

      // Run built-in normal commands
      else
      {
         executeAndWait(userArgs);
      }

      free(userArgs);

   }

   return 0;
}