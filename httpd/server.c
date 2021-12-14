#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 2828

void handle_request(int nfd)
{
   FILE *network = fdopen(nfd, "r");
   char *line = NULL;
   size_t size;
   ssize_t num;

   if (network == NULL)
   {
      perror("fdopen");
      return;
   }

   while ((num = getline(&line, &size, network)) >= 0)
   {
      printf("%s", line);
//      printf("[echo]: sending text (%s) back to client\n", line);
      write(nfd, line, size);
   }

   free(line);
   fclose(network);
}

void run_service(int fd)
{
   while (1)
   {
      int nfd = accept_connection(fd);
      if (nfd != -1)
      {
          int childProcessStatus = fork();
          if (childProcessStatus == -1) {
              perror("fork");
          } else if (childProcessStatus == 0) {
              printf("Connection established\n");
              handle_request(nfd);
              printf("Connection closed\n");
              exit(0);
          }
      }
   }
}

int main(void)
{
   int fd = create_service(PORT);

   if (fd != -1)
   {
      printf("listening on port: %d\n", PORT);
      run_service(fd);
      close(fd);
   }

   return 0;
}
