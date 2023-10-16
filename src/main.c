/*******************************************************
 * Copyright (C) 2023
 *
 *  Created on: Oct 14, 2023
 *      Author: p.sydow
 *
 *******************************************************/



#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#define  MAX_TCP_PACKAGE_SIZE  1024

#include "httpServer.h"
#include "stringUtils.h"
// Test scenario:
//     1. socket already in use
//     2. link disappear

// Using poll() instead of select()
// https://www.ibm.com/docs/en/i/7.1?topic=designs-using-poll-instead-select


static struct HttpResponse onHttpRequest(struct HttpRequest* p_request) {
    if (p_request->method == REQ_POST) {
        printf("Post method\n");
    }
    if (p_request->method == REQ_DELETE) {
        printf("DELETE method\n");
    }
    if (p_request->method == REQ_GET) {
        printf("GET method\n");
    }
    if (p_request->method == REQ_UNKNOWN) {
        printf("UNKNOWN method\n");
    }

    printf("This app level  callback for http server.\n");
    struct HttpResponse response = createSimpleHttpResponse("Not Found", 404);
    return response;
}

int main() {
    char *line = NULL;
    size_t len = 0;
    struct HttpServer server = createHttpServer();

    server.onHttpRequest = &onHttpRequest;

    server.start(&server, "0.0.0.0", 5000);
    int finishMe = 0;
    while (finishMe == 0) {
        printf("[PS]> ");  // print prompt
        int lineSize = getline(&line, &len, stdin);
        chomp(line);
        //*********************************
        //      exit command
        //*********************************

        if ((strcmp(line, "exit") == 0) || (strcmp(line, "quit") == 0)) {
            finishMe = 1;

        } else if (strcmp(line, "") == 0){
            // nothing to do;
        }else {
            printf("Error: Unknown command: '%s'.\n", line);
        }

        free(line);
        line = NULL;
        len = 0;
    }
    server.stop(&server);
    return 0;
}
