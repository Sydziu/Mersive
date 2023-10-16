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
#include "keyStorage.h"

static struct KeyStorage keyStg;
// Test scenario:
//     1. socket already in use
//     2. link disappear

// Using poll() instead of select()
// https://www.ibm.com/docs/en/i/7.1?topic=designs-using-poll-instead-select


static struct HttpResponse onHttpRequest(struct HttpRequest* p_request) {
    if (p_request->method == REQ_POST) {
    	struct SingleRecord record;
    	record.content = strdup(p_request->content);
    	record.contentLength = p_request->contentLength;
    	record.contentType = strdup(p_request->contentType);
    	record.url = strdup(p_request->url);

    	int rc = keyStg.addKey(&keyStg, &record);

    	if (rc !=0) {
    		// Unable to add key. capacity limit ?
    	    struct HttpResponse response = createSimpleHttpResponse("Not Found", 404);
    	    return response;
    	}
    	destroySingleRecord(&record);
    	struct HttpResponse response = createSimpleHttpResponse("OK", 200);
    	return response;
    }
    if (p_request->method == REQ_DELETE) {
    	int rc = keyStg.removeKey(&keyStg, p_request->url);
    	if (rc !=0) {
    		// Unable to remove key. Does not exist ?
    	    struct HttpResponse response = createSimpleHttpResponse("Not Found", 404);
    	    return response;
    	}
    	struct HttpResponse response = createSimpleHttpResponse("OK", 200);
    	return response;
    }
    if (p_request->method == REQ_GET) {
    	struct SingleRecord* record = keyStg.getKey(&keyStg, p_request->url);
    	if (record) {
        	struct HttpResponse response = createHttpResponse("OK", 200, record->content, record->contentType, record->contentLength);
        	return response;
    	}
	    struct HttpResponse response = createSimpleHttpResponse("Not Found", 404);
	    return response;
    }
    if (p_request->method == REQ_UNKNOWN) {
	    struct HttpResponse response = createSimpleHttpResponse("Not Found", 404);
	    return response;
    }

    printf("This app level  callback for http server.\n");
    struct HttpResponse response = createSimpleHttpResponse("Not Found", 404);
    return response;
}

int main() {
	keyStg = createKeyStorage();
    char *line = NULL;
    size_t len = 0;
    struct HttpServer server = createHttpServer();

    server.onHttpRequest = &onHttpRequest;

    server.start(&server, 8000);
    int finishMe = 0;
    while (finishMe == 0) {
        printf("[PS]> ");  // print prompt
        getline(&line, &len, stdin);
        chomp(line);
        //*********************************
        //      exit command
        //*********************************

        if ((strcmp(line, "exit") == 0) || (strcmp(line, "quit") == 0)) {
            finishMe = 1;

        } else if (strcmp(line, "") == 0){
            // nothing to do;
        } else if (strcmp(line, "report") == 0) {
        	keyStg.dumpKeys(&keyStg);
        	printf("Number of keys = %d\n", keyStg.getSize(&keyStg));
        }else {
            printf("Error: Unknown command: '%s'.\n", line);
        }

        free(line);
        line = NULL;
        len = 0;
    }
    server.stop(&server);
    destroyKeyStorage(&keyStg);
    return 0;
}
