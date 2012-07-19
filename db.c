/*
** Copyright 2010, Adam Shanks (@ChainsDD)
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include <sqlite3.h>

#include "su.h"

static sqlite3 *db_init(const char *name)
{
    sqlite3 *db;
    int rc;

    rc = sqlite3_open_v2(name, &db, SQLITE_OPEN_READONLY, NULL);
    if ( rc ) {
        LOGE("Couldn't open database: %s", sqlite3_errmsg(db));
        return NULL;
    }

    // Create an automatic busy handler in case the db is locked
    sqlite3_busy_timeout(db, 1000);
    return db;
}

static int db_check(sqlite3 *db, const struct su_context *ctx)
{
    char sql[4096];
    char *zErrmsg;
    char **result;
    int nrow,ncol;
    int allow = DB_INTERACTIVE;

    sqlite3_snprintf(
        sizeof(sql), sql,
        "SELECT _id,name,allow FROM apps WHERE uid=%u AND exec_uid=%u AND exec_cmd='%q';",
        ctx->from.uid, ctx->to.uid, get_command(&ctx->to)
    );

    if (strlen(sql) >= sizeof(sql)-1)
        return DB_DENY;
        
    int error = sqlite3_get_table(db, sql, &result, &nrow, &ncol, &zErrmsg);
    if (error != SQLITE_OK) {
        LOGE("Database check failed with error message %s", zErrmsg);
        if (error == SQLITE_BUSY) {
            LOGE("Specifically, the database is busy");
        }
        return DB_DENY;
    }
    
    if (nrow == 0 || ncol != 3)
        goto out;
        
    if (strcmp(result[0], "_id") == 0 && strcmp(result[2], "allow") == 0) {
        if (strcmp(result[5], "1") == 0) {
            allow = DB_ALLOW;
        } else if (strcmp(result[5], "-1") == 0){
            allow = DB_INTERACTIVE;
        } else {
            allow = DB_DENY;
        }
    }

out:
    sqlite3_free_table(result);
    
    return allow;
}

int database_check(const struct su_context *ctx)
{
	static const char *databases[] =
		{ REQUESTOR_DATABASE_PATH, PERMISSIONS_DATABASE_PATH};
    sqlite3 *db;
	size_t i;
    int dballow;

	for (i = 0; i < ARRAY_SIZE(databases); i++) {
	    LOGD("sudb - Opening database %s", databases[i]);
	    db = db_init(databases[i]);
		if (db)
			break;
		LOGD("sudb - Could not open database %s", databases[i]);
	}
    if (!db) {
        LOGD("sudb - Could not open database, prompt user");
   	    // if the database could not be opened, we can assume we need to
       	// prompt the user
       	return DB_INTERACTIVE;
   	}

    LOGD("sudb - Database opened");
    dballow = db_check(db, ctx);
    // Close the database, we're done with it. If it stays open, it will cause problems
    sqlite3_close(db);
    LOGD("sudb - Database closed");
    return dballow;
}
