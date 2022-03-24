#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

char *readLine(void)
{
   char *fullLine = malloc(1000);
   int adder = 0;

   while (1)
   {

      char in = fgetc(stdin);

      if (in == EOF || in == '\n')
      {
         // ending on null byte
         fullLine[adder] = '\0';
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
   
   for (int i = 1; i <= 10; i++)
   {
      printf("%d: %s\n", i, historyOfUser[i-1]);

      if (historyOfUser[i]==NULL)
      {
         break;
      }
   }

}

void addToHistory(char *userString, char **historyOfUser)
{

   for (int i = 9; i > 0; i--)
   {
      historyOfUser[i] = historyOfUser[i-1];
   }

   char *temp = malloc(1000);
   strcpy(temp, userString);

   historyOfUser[0] = temp;

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

      // Adding this to command history
      addToHistory(fullCommand, historyCommands);

      // Resetting an args 2d array
      char **userArgs = malloc(1000);
      // Filling the array with users args
      splitToArgs(fullCommand, userArgs);

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