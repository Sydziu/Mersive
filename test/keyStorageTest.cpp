#include "../src/keyStorage.c"
/*******************************************************
 * Copyright (C) 2023
 *
 * RequestParsing.c
 *
 *  Created on: Oct 14, 2023
 *      Author: p.sydow
 *
 *******************************************************/

#include <string>
#include "gtest/gtest.h"

TEST(keyStorage, update) {
    struct KeyStorage keyStg = createKeyStorage();
    EXPECT_EQ(keyStg.getSize(&keyStg), 0);
    struct SingleRecord record;
    memset(&record, 0, sizeof(record));

    /*************************************************************/
    /* Create and save first key                                 */
    /*************************************************************/
    record.content = strdup("content1");
    record.contentLength = 13;
    record.contentType = strdup("content-type1");
    record.url = strdup("url1");
    keyStg.addKey(&keyStg, &record);
    EXPECT_EQ(keyStg.getSize(&keyStg), 1);
    destroySingleRecord(&record);

    /*************************************************************/
    /* Create and save second key                                */
    /*************************************************************/
    record.content = strdup("content2");
    record.contentLength = 14;
    record.contentType = strdup("content-type2 ");
    record.url = strdup("url1");
    keyStg.addKey(&keyStg, &record);
    EXPECT_EQ(keyStg.getSize(&keyStg), 1);
    destroySingleRecord(&record);

    /*************************************************************/
    /* Retrieve first key                                        */
    /*************************************************************/
    struct SingleRecord* storedKey;
    storedKey = keyStg.getKey(&keyStg, "url1");
    ASSERT_TRUE(storedKey);
    EXPECT_EQ(std::string(storedKey->content), "content2");
    EXPECT_EQ(std::string(storedKey->contentType), "content-type2 ");
    EXPECT_EQ(std::string(storedKey->url), "url1");

    destroyKeyStorage(&keyStg);
}


TEST(keyStorage, tooMachInserts) {
	int keyIndex=0;
	char keyIndexBuff[32];
    struct KeyStorage keyStg = createKeyStorage();
    struct SingleRecord record;

	for (keyIndex = 0; keyIndex < 1000; keyIndex++) {
		snprintf(keyIndexBuff, sizeof(keyIndexBuff), "url-%d", keyIndex);
		memset(&record, 0, sizeof(record));

		/*************************************************************/
		/* Create and save  key                                      */
		/*************************************************************/
		record.content = strdup("content1");
		record.contentLength = 13;
		record.contentType = strdup("content-type1");
		record.url = strdup(keyIndexBuff);
		if (keyStg.addKey(&keyStg, &record) !=0) {
			destroySingleRecord(&record);
			// unable to add key. To many data
			break;
		}
		destroySingleRecord(&record);
	}

	EXPECT_EQ(keyIndex, 100); // Expect 100 keys to be inserted. This value is define by the MAX_KEYS
	EXPECT_EQ(keyStg.getSize(&keyStg), 100);

	destroyKeyStorage(&keyStg);
}

TEST(keyStorage, add_get_remove) {
    struct KeyStorage keyStg = createKeyStorage();

    struct SingleRecord record;
    memset(&record, 0, sizeof(record));

    /*************************************************************/
    /* Create and save first key                                 */
    /*************************************************************/
    record.content = strdup("content1");
    record.contentLength = 13;
    record.contentType = strdup("content-type1");
    record.url = strdup("url1");
    keyStg.addKey(&keyStg, &record);
    EXPECT_EQ(keyStg.getSize(&keyStg), 1);
    destroySingleRecord(&record);

    /*************************************************************/
    /* Create and save second key                                */
    /*************************************************************/
    record.content = strdup("content2");
    record.contentLength = 14;
    record.contentType = strdup("content-type2 ");
    record.url = strdup("url2");
    keyStg.addKey(&keyStg, &record);
    EXPECT_EQ(keyStg.getSize(&keyStg), 2);
    destroySingleRecord(&record);

    /*************************************************************/
    /* Retrieve  keys                                            */
    /*************************************************************/
    struct SingleRecord* storedKey;
    storedKey = keyStg.getKey(&keyStg, "url1");
    ASSERT_TRUE(storedKey != NULL);
    EXPECT_EQ(std::string(storedKey->content), "content1");
    EXPECT_EQ(std::string(storedKey->contentType), "content-type1");
    EXPECT_EQ(std::string(storedKey->url), "url1");

    storedKey = keyStg.getKey(&keyStg, "url2");
    ASSERT_TRUE(storedKey);
    EXPECT_EQ(std::string(storedKey->content), "content2");
    EXPECT_EQ(std::string(storedKey->contentType), "content-type2 ");
    EXPECT_EQ(std::string(storedKey->url), "url2");

    /*************************************************************/
    /* Remove key                                                */
    /*************************************************************/
    EXPECT_EQ(keyStg.removeKey(&keyStg, "url1"), 0);
    EXPECT_EQ(keyStg.getSize(&keyStg), 1);

    /*************************************************************/
    /* Retrieve unexisting key                                   */
    /*************************************************************/
    storedKey = keyStg.getKey(&keyStg, "url1");
    ASSERT_TRUE(storedKey == NULL);

    EXPECT_NE(keyStg.removeKey(&keyStg, "url1"), 0);
    destroyKeyStorage(&keyStg);
    EXPECT_EQ(keyStg.getSize(&keyStg), 0);
}
