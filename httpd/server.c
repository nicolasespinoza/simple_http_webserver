#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "util/array_list.h"
#include "util/toolkit.h"

void send_reply(int response_code, int content_length) {
    // TODO: BEFORE THE FINAL TEXT IS SENT, VERIFY CLIENT IS STILL CONNECTED
    // TODO: uh maybe send all of these with an HTML casing, meaning <html><h1>ETC ETC</h1></html>
    if (response_code == 400) {
        printf("HTTP/1.0 400 Bad Request\n");
    } else if (response_code == 403) {
        printf("HTTP/1.0 403 Permission Denied\n");
    } else if (response_code == 404) {
        printf("HTTP/1.0 404 Not Found\n");
    } else if (response_code == 500) {
        printf("HTTP/1.0 500 Internal Error\n"); // TODO: not covered
    } else if (response_code == 200) {
        // don't forget this is supposed to be sent to the client, not printed...
        printf("HTTP/1.0 200 OK\r\n");
        printf("Content-Type: text/html\r\n");
        printf("Content-Length: %d\r\n", content_length);
        printf("\r\n");
    } else {
        printf("HTTP/1.0 501 Not Implemented\n");
    }
}

void end_request_cleanup(FILE* file, struct arraylist* split_request_text, char* expected_valid_file_name) {
    if (file != NULL) {
        fclose(file);
    }
    if (expected_valid_file_name != NULL) {
        free(expected_valid_file_name);
    }
    array_list_cleanup(split_request_text);
}

void handle_client_request(char* client_request) {
    struct arraylist* split_request_text = split(client_request, " ");

    // permitted syntax:
    // GET <file name> <http version>       3 items
    // HEAD <file name> <http version>      3 items

    if (split_request_text->number_of_items < 3) { // malformed request / invalid syntax
        send_reply(400, -1);
        end_request_cleanup(NULL, split_request_text, NULL);
        return;
    }

    char* request_type = split_request_text->data[0];
    if (split_request_text->number_of_items > 3 || // if syntax has more than 3 arguments
        (strcmp(request_type, "GET") != 0 && strcmp(request_type, "HEAD") != 0)) { // if request isn't HEAD or GET
        send_reply(501, -1);
        end_request_cleanup(NULL, split_request_text, NULL);
        return;
    }

    char* expected_valid_file_name = strdup(split_request_text->data[1]);
    if (strcmp(expected_valid_file_name, "/") == 0 || // checks if only "/" is passed as the file name
            contains_double_dot(expected_valid_file_name)) { // checks if file name contains ".." to access invalid areas
        send_reply(400, -1);
        end_request_cleanup(NULL, split_request_text, expected_valid_file_name);
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
        end_request_cleanup(file, split_request_text, expected_valid_file_name);
        return;
    }

    int file_size = get_file_size(file_name);
    int is_get = strcmp(request_type, "GET") == 0;

    send_reply(200, file_size);
    if (is_get) {
        print_file_contents(file); // remember to adapt this for HTML wrapping
    }
    end_request_cleanup(file, split_request_text, expected_valid_file_name);

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

   while ((num = getline(&line, &size, network)) >= 0)
   {
      printf("%s", line);
//      write(nfd, line, size); // backup. origin
       write(nfd, "lol test", size);
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

int main(int number_of_arguments, char* arguments[]) {
//    handle_client_request("GET /test.txt HTTP/1.0");

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
