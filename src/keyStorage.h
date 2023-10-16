/*******************************************************
 * Copyright (C) 2023
 *
 *  Created on: Oct 16, 2023
 *      Author: p.sydow
 *
 *******************************************************/
#pragma once

#include <stdio.h>
#include <pthread.h>


struct SingleRecord {
    char* content;
    char* url;
    int contentLength;
    char* contentType;
};

#define MAX_KEYS 100

/*************************************************************/
/* The interface for the KeyStorage                          */
/*************************************************************/
struct KeyStorage {
    int (*addKey)(struct KeyStorage *This, struct SingleRecord *p_record);
    struct SingleRecord* (*getKey)(struct KeyStorage *This, const char* key);
    int (*removeKey)(struct KeyStorage *This, const char* key);
    struct {
        struct SingleRecord keys[MAX_KEYS];
        pthread_mutex_t lock;
    } Private;
};

extern void destroyKeyStorage(struct KeyStorage *This);
extern struct KeyStorage createKeyStorage();
extern void destroySingleRecord(struct SingleRecord* This);


