/*******************************************************
 * Copyright (C) 2023
 *
 *  Created on: Oct 16, 2023
 *      Author: p.sydow
 *
 *******************************************************/


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "keyStorage.h"

static pthread_mutex_t lock;

/*************************************************************/
/* Helpers functions                                         */
/*************************************************************/

static struct SingleRecord* findKeyThreadUnsafe(struct KeyStorage *This, const char* p_url) {
    for (int inx = 0; inx < MAX_KEYS; inx++) {
        if (This->Private.keys[inx].url == NULL) {
            continue;
        }
        if (strcmp(This->Private.keys[inx].url, p_url) == 0) {
            return &This->Private.keys[inx];
        }
    }
    return NULL;
}

struct SingleRecord* findEmptyKeyThreadUnsafe(struct KeyStorage *This) {
    for (int inx = 0; inx < MAX_KEYS; inx++) {
        if (This->Private.keys[inx].url == NULL) {
            return &This->Private.keys[inx];
        }
    }
    return NULL;
}


/*************************************************************/
/* The memers of interface                                   */
/*************************************************************/
static int addKey(struct KeyStorage *This, struct SingleRecord *p_record) {
    pthread_mutex_lock(&lock);
    struct SingleRecord * placeToStore;

    /*************************************************************/
    /* Try to find already existing key                          */
    /*************************************************************/
    placeToStore =  findKeyThreadUnsafe(This, p_record->url);

    /*************************************************************/
    /* Try to find empty place in tab                            */
    /*************************************************************/

    if (placeToStore == NULL) {
        placeToStore = findEmptyKeyThreadUnsafe(This);
        if (placeToStore) {
        	This->Private.size++;
        }
    } else {
    	destroySingleRecord(placeToStore);
    }
    if (placeToStore == NULL) {
    	pthread_mutex_unlock(&lock);
        return -1;  // TODO(p.sydow): return enum indicate what is the problem
    }


    placeToStore->content = strdup(p_record->content);
    placeToStore->contentType = strdup(p_record->contentType);
    placeToStore->url = strdup(p_record->url);
    placeToStore->contentLength = p_record->contentLength;
    pthread_mutex_unlock(&lock);

    return 0;
}

static struct SingleRecord* getKey(struct KeyStorage *This, const char* p_key) {
    pthread_mutex_lock(&lock);
    struct SingleRecord *  theItem =  findKeyThreadUnsafe(This, p_key);


    pthread_mutex_unlock(&lock);
    return theItem;
}

static int getSize(struct KeyStorage *This) {
	volatile int size;
    pthread_mutex_lock(&lock);
    size = This->Private.size;
    pthread_mutex_unlock(&lock);
    return size;
}

static void dumpKeys(struct KeyStorage *This) {
	pthread_mutex_lock(&lock);
	printf("Keys:\n");
	for (int inx = 0; inx < MAX_KEYS; inx++) {
		if (This->Private.keys[inx].url != NULL) {
			printf("   [%s]\n",This->Private.keys[inx].url);
		}
	}
	pthread_mutex_unlock(&lock);
}


static int removeKey(struct KeyStorage *This, const char* p_key) {
    pthread_mutex_lock(&lock);
    //pthread_mutex_lock(&This->Private.lock);
    int ret = 0;
    struct SingleRecord *  theItem =  findKeyThreadUnsafe(This, p_key);
    if (theItem != NULL) {
        destroySingleRecord(theItem);
        assert(This->Private.size);
        This->Private.size--;
    } else {
        // Unable to delete record since it is not found
        ret = -1;  // TODO(p.sydow) use enum to indicate error
    }
    pthread_mutex_unlock(&lock);
    //pthread_mutex_unlock(&This->Private.lock);
    return ret;
}


/*************************************************************/
/* The interface for the KeyStorage                          */
/*************************************************************/

void destroySingleRecord(struct SingleRecord* This) {
    free(This->content); This->content = NULL;
    free(This->contentType); This->contentType = NULL;
    free(This->url); This->url = NULL;
}

struct KeyStorage createKeyStorage() {
    struct KeyStorage ret;
    memset(ret.Private.keys, 0 , sizeof(ret.Private.keys));
    ret.addKey = addKey;
    ret.getKey = getKey;
    ret.removeKey = removeKey;
    ret.getSize = getSize;
    ret.dumpKeys = dumpKeys;
    ret.Private.size = 0;
    return ret;
};

void destroyKeyStorage(struct KeyStorage *This) {
    //pthread_mutex_lock(&This->Private.lock);
    pthread_mutex_lock(&lock);
    for (int i=0; i<MAX_KEYS;i++) {
        destroySingleRecord(&This->Private.keys[i]);
    }
    This->Private.size = 0;
    pthread_mutex_unlock(&lock);
    //pthread_mutex_unlock(&This->Private.lock);
}


