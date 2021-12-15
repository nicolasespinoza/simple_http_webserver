#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "util/array_list.h"
#include "util/toolkit.h"

void send_html_error(char* message) {
    printf("<html>\n\t<h1>%s</h1>\n</html>\n", message);
}

void send_reply(int response_code, int content_length) {
    // TODO: BEFORE THE FINAL TEXT IS SENT, VERIFY CLIENT IS STILL CONNECTED
    if (response_code == 400) {
        send_html_error("HTTP/1.0 400 Bad Request");
    } else if (response_code == 403) {
        send_html_error("HTTP/1.0 403 Permission Denied");
    } else if (response_code == 404) {
        send_html_error("HTTP/1.0 404 Not Found");
    } else if (response_code == 500) {
        send_html_error("HTTP/1.0 500 Internal Error"); // TODO: not covered
    } else if (response_code == 200) {
        printf("HTTP/1.0 200 OK\r\n");
        printf("Content-Type: text/html\r\n");
        printf("Content-Length: %d\r\n", content_length);
        printf("\r\n");
    } else {
        send_html_error("HTTP/1.0 501 Not Implemented");
    }
}

void end_request_cleanup(FILE* file, struct arraylist* split_request_text, char* expected_valid_file_name,
        int previous_std_out, int client_file_descriptor) {
    if (file != NULL) {
        fclose(file);
    }
    if (expected_valid_file_name != NULL) {
        free(expected_valid_file_name);
    }
    array_list_cleanup(split_request_text);

//    close(client_file_descriptor);

    // restore stdout file descriptors
    dup2(previous_std_out, 1);
    close(previous_std_out);

//    exit(0); exiting just makes it to the connection never closes
}

void handle_client_request(char* client_request, int client_file_descriptor) {
    int previous_std_out = dup(1); // previous stdout
    dup2(client_file_descriptor, STDOUT_FILENO);
    close(client_file_descriptor); // this closes the connection

//    printf("Client request: [%s]\n", client_request);

    struct arraylist* split_request_text = split(client_request, " ");

    // permitted syntax:
    // GET <file name> <http version>       3 items
    // HEAD <file name> <http version>      3 items

    if (split_request_text->number_of_items < 3) { // malformed request / invalid syntax
        send_reply(400, -1);
        end_request_cleanup(NULL, split_request_text, NULL, previous_std_out, client_file_descriptor);
        return;
    }

    char* request_type = split_request_text->data[0];
    if (split_request_text->number_of_items > 3 || // if syntax has more than 3 arguments
        (strcmp(request_type, "GET") != 0 && strcmp(request_type, "HEAD") != 0)) { // if request isn't HEAD or GET
        send_reply(501, -1);
        end_request_cleanup(NULL, split_request_text, NULL, previous_std_out, client_file_descriptor);
        return;
    }

    char* expected_valid_file_name = strdup(split_request_text->data[1]);
    if (strcmp(expected_valid_file_name, "/") == 0 || // checks if only "/" is passed as the file name
            contains_double_dot(expected_valid_file_name)) { // checks if file name contains ".." to access invalid areas
        send_reply(400, -1);
        end_request_cleanup(NULL, split_request_text, expected_valid_file_name, previous_std_out, client_file_descriptor);
        return;
    }

    char* file_name;
    if (expected_valid_file_name[0] == '/') { // if expected file name starts with "/" ie "GET /index.html 1.0"
        file_name = expected_valid_file_name + 1; // removes leading "/"
    } else {
        file_name = expected_valid_file_name;
    }

    FILE* file;
    errno = 0;
    file = fopen(file_name, "r");
    if (file == NULL) {
        if (errno == ENOENT) { // file not found
            send_reply(404, -1);
        } else if (errno == EACCES) { // cannot access / no permissions
            send_reply(403, -1);
        }
        end_request_cleanup(file, split_request_text, expected_valid_file_name, previous_std_out, client_file_descriptor);
        return;
    }

    int file_size = get_file_size(file_name);
    int is_get = strcmp(request_type, "GET") == 0;

    send_reply(200, file_size);
    if (is_get) {
        print_file_contents(file);
//        printf("\n");
    }
    end_request_cleanup(file, split_request_text, expected_valid_file_name, previous_std_out, client_file_descriptor);

}

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

   while ((num = getline(&line, &size, network)) >= 0) {
       handle_client_request(line, nfd);
       close(nfd);
       // close file descriptor for client?
       //exit(0); // should exit after the first line is processed. All other lines will not be processed
//            printf("Done\n");
//            write(nfd, "done", 10);
//            close(nfd);
//            exit(0);
//        }
//       handle_client_request(line, nfd);

//      write(nfd, line, size); // backup. origin
//       write(nfd, "lol test", size);
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
              int pid = getpid();
              printf("Connection established (ID: %d)\n", pid);
              handle_request(nfd);

              close(fd);
              close(nfd);

              printf("Connection closed (ID: %d)\n", pid);

              exit(0);
          } else {
              wait(0);

              close(nfd);
          }
      }
   }
}

int main(int number_of_arguments, char* arguments[]) {
    if (number_of_arguments == 2) {
        int port = string_to_int(arguments[1], 1);
        int fd = create_service(port);
        if (fd != -1)
        {
            printf("listening on port: %d\n", port);
            run_service(fd);
            close(fd);
        }
    } else {
        printf("Syntax: ./httpd <port>\n");
    }
    return 0;
}
