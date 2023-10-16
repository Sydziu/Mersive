#include "../src/httpServer.c"   // NOLINT dirty hack to test static fields
#include "../src/stringUtils.c"  // NOLINT dirty hack to test static fields
/*******************************************************
 * Copyright (C) 2020 Ftd Aero
 *
 * RequestParsing.c
 *
 *  Created on: Oct 14, 2023
 *      Author: p.sydow
 *
 *******************************************************/


#include "gtest/gtest.h"


TEST(httpRequestParser, POST) {
    char* buff = strdup(
                    "POST /foo HTTP/1.1\x0D\x0A""User-Agent: curl/7.29.0\x0D\x0A"
                    "Host: localhost:8000\x0D\x0A""Accept: */*\x0D\x0A"
                    "Content-Type: application/json\x0D\x0A"
                    "Content-Length: 16\x0D\x0A\x0D\x0A"
                    "{ \"secret\": 42 }");

    static struct HttpRequest request = createHttpRequest(buff, strlen(buff));
    EXPECT_EQ(request.method, REQ_POST);
    EXPECT_EQ(std::string(request.url), "/foo");
    EXPECT_EQ(std::string(request.contentType), "application/json");
    EXPECT_EQ(std::string(request.content), "{ \"secret\": 42 }");
    EXPECT_EQ(request.contentLength, 16);
    destroyHttpRequest(&request);
    free(buff);
}

TEST(httpRequestParser, POSTWrongEnd) {
    char* buff = strdup(
                    "POST /foo HTTP/1.1\x0D\x0A""User-Agent: curl/7.29.0\x0D\x0A"
                    "Host: localhost:8000\x0D\x0A""Accept: */*\x0D\x0A"
                    "Content-Type: application/json\x0D\x0A"
                    "Content-Length: 16\\x0D\x0A"
                    "{ \"secret\": 42 }");

    static struct HttpRequest request = createHttpRequest(buff, strlen(buff));
    EXPECT_EQ(request.method, REQ_UNKNOWN);
    destroyHttpRequest(&request);
    free(buff);
}


TEST(httpRequestParser, POSTZeroContent) {
    char* buff = strdup(
                    "POST /foo HTTP/1.1\x0D\x0A""User-Agent: curl/7.29.0\x0D\x0A"
                    "Host: localhost:8000\x0D\x0A""Accept: */*\x0D\x0A"
                    "Content-Type: application/json\x0D\x0A"
                    "Content-Length: 0\x0D\x0A\x0D\x0A"
                    "");

    static struct HttpRequest request = createHttpRequest(buff, strlen(buff));
    EXPECT_EQ(request.method, REQ_POST);
    EXPECT_EQ(std::string(request.url), "/foo");
    EXPECT_EQ(std::string(request.contentType), "application/json");
    EXPECT_EQ(std::string(request.content), "");
    EXPECT_EQ(request.contentLength, 0);
    destroyHttpRequest(&request);
    free(buff);
}

TEST(httpRequestParser, POSTContantSizeToSmall) {
    char* buff = strdup(
                    "POST /foo HTTP/1.1\x0D\x0A""User-Agent: curl/7.29.0\x0D\x0A"
                    "Host: localhost:8000\x0D\x0A""Accept: */*\x0D\x0A"
                    "Content-Type: application/json\x0D\x0A"
                    "Content-Length: 10\x0D\x0A\x0D\x0A"
                    "{ \"secret\": 42 }");

    static struct HttpRequest request = createHttpRequest(buff, strlen(buff));
    EXPECT_EQ(request.method, REQ_UNKNOWN);
    destroyHttpRequest(&request);
    free(buff);
}

TEST(httpRequestParser, POSTContantSizeToBig) {
    char* buff = strdup(
                    "POST /foo HTTP/1.1\x0D\x0A""User-Agent: curl/7.29.0\x0D\x0A"
                    "Host: localhost:8000\x0D\x0A""Accept: */*\x0D\x0A"
                    "Content-Type: application/json\x0D\x0A"
                    "Content-Length: 20\x0D\x0A\x0D\x0A"
                    "{ \"secret\": 42 }");

    static struct HttpRequest request = createHttpRequest(buff, strlen(buff));
    EXPECT_EQ(request.method, REQ_UNKNOWN);
    destroyHttpRequest(&request);
    free(buff);
}


TEST(httpRequestParser, POSTContentLengthNotNum) {
    char* buff = strdup(
                    "POST /foo HTTP/1.1\x0D\x0A""User-Agent: curl/7.29.0\x0D\x0A"
                    "Host: localhost:8000\x0D\x0A""Accept: */*\x0D\x0A"
                    "Content-Type: application/json\x0D\x0A"
                    "Content-Length: qwerty\x0D\x0A\x0D\x0A"
                    "");

    static struct HttpRequest request = createHttpRequest(buff, strlen(buff));
    EXPECT_EQ(request.method, REQ_POST);
    EXPECT_EQ(std::string(request.url), "/foo");
    EXPECT_EQ(std::string(request.contentType), "application/json");
    EXPECT_EQ(request.contentLength, 0);
    destroyHttpRequest(&request);
    free(buff);
}

TEST(httpRequestParser, POSTmoreSpacesAroundValues) {
    char* buff = strdup(
                    "POST /foo HTTP/1.1 \x0D\x0A"
                    "User-Agent: curl/7.29.0\x0D\x0A"
                    "Host: localhost:8000 \x0D\x0A"
                    "Accept: */* \x0D\x0A"
                    "Content-Type: application/json \x0D\x0A"
                    "Content-Length:  16  \x0D\x0A\x0D\x0A"
                    "{ \"secret\": 42 }");

    static struct HttpRequest request = createHttpRequest(buff, strlen(buff));
    EXPECT_EQ(request.method, REQ_POST);
    EXPECT_EQ(std::string(request.url), "/foo");
    EXPECT_EQ(std::string(request.contentType), "application/json ");
    EXPECT_EQ(request.contentLength, 16);
    destroyHttpRequest(&request);
    free(buff);
}
