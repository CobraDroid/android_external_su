/*
** Copyright 2010, Adam Shanks (@ChainsDD)
** Copyright 2008, Zinx Verituse (@zinxv)
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

#ifndef SU_h 
#define SU_h 1

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "su"

#define REQUESTOR "com.noshufou.android.su"
#define REQUESTOR_DATA_PATH "/data/data/" REQUESTOR
#define REQUESTOR_CACHE_PATH "/dev/" REQUESTOR

#define REQUESTOR_DATABASES_PATH REQUESTOR_DATA_PATH "/databases/"
#define PERMISSIONS_DATABASE_PATH REQUESTOR_DATABASES_PATH "permissions.sqlite"
#define REQUESTOR_DATABASE_PATH REQUESTOR_DATABASES_PATH "su.db"

/* intent actions */
#define ACTION_REQUEST REQUESTOR ".REQUEST"
#define ACTION_RESULT  REQUESTOR ".RESULT"

#define DEFAULT_SHELL "/system/bin/sh"

#ifdef SU_LEGACY_BUILD
#define VERSION_EXTRA	"-legacy"
#else
#define VERSION_EXTRA	""
#endif

#define VERSION "3.2-gc" VERSION_EXTRA
#define VERSION_CODE 18

#define PROTO_VERSION 0

struct su_initiator {
    pid_t pid;
    unsigned uid;
    char bin[PATH_MAX];
    char args[4096];
};

struct su_request {
    unsigned uid;
    int login;
    int keepenv;
    char *shell;
    char *command;
    char **argv;
    int argc;
    int optind;
};

struct su_context {
    struct su_initiator from;
    struct su_request to;
    mode_t umask;
    int sdk_version;
    int htc;
};

enum {
    DB_INTERACTIVE,
    DB_DENY,
    DB_ALLOW
};

extern int database_check(const struct su_context *ctx);

extern int send_intent(const struct su_context *ctx,
                       const char *socket_path, int allow, const char *action);

static inline char *get_command(const struct su_request *to)
{
	return (to->command) ? to->command : to->shell;
}

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

#include <cutils/log.h>
#ifndef LOGE
#define LOGE(...) ALOGE(__VA_ARGS__)
#endif
#ifndef LOGD
#define LOGD(...) ALOGD(__VA_ARGS__)
#endif
#ifndef LOGW
#define LOGW(...) ALOGW(__VA_ARGS__)
#endif

#if 0
#undef LOGE
#define LOGE(fmt,args...) fprintf(stderr, fmt, ##args)
#undef LOGD
#define LOGD(fmt,args...) fprintf(stderr, fmt, ##args)
#undef LOGW
#define LOGW(fmt,args...) fprintf(stderr, fmt, ##args)
#endif

#define PLOGE(fmt,args...) LOGE(fmt " failed with %d: %s", ##args, errno, strerror(errno))
#define PLOGEV(fmt,err,args...) LOGE(fmt " failed with %d: %s", ##args, err, strerror(err))

#endif
