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

#include <unistd.h>
#ifdef SU_LEGACY_BUILD
#include <utils/IBinder.h>
#include <utils/IServiceManager.h>
#include <utils/Parcel.h>
#else
#include <binder/IBinder.h>
#include <binder/IServiceManager.h>
#include <binder/Parcel.h>
#endif
#include <utils/String8.h>
#include <assert.h>

extern "C" {
#include "su.h"
}

using namespace android;

static const int BROADCAST_INTENT_TRANSACTION = IBinder::FIRST_CALL_TRANSACTION + 13;

static const int NULL_TYPE_ID = 0;

static const int VAL_STRING = 0;
static const int VAL_INTEGER = 1;

static const int START_SUCCESS = 0;

int send_intent(const struct su_context *ctx,
                const char *socket_path, int allow, const char *action)
{
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> am = sm->checkService(String16("activity"));
    assert(am != NULL);

    Parcel data, reply;
    data.writeInterfaceToken(String16("android.app.IActivityManager"));

    data.writeStrongBinder(NULL); /* caller */

    /* intent */
    data.writeString16(String16(action)); /* action */
    data.writeInt32(NULL_TYPE_ID); /* Uri - data */
    data.writeString16(NULL, 0); /* type */
    data.writeInt32(0); /* flags */
    if (ctx->sdk_version >= 4) {
        // added in donut
        data.writeString16(NULL, 0); /* package name - DONUT ONLY, NOT IN CUPCAKE. */
    }
    data.writeString16(NULL, 0); /* ComponentName - package */
    if (ctx->sdk_version >= 7) {
        // added in eclair rev 7
        data.writeInt32(0); /* Rect - the bounds of the sender */
    }
    data.writeInt32(0); /* Categories - size */
    if (ctx->sdk_version >= 15) {
        // added in IceCreamSandwich 4.0.3
        data.writeInt32(0); /* Selector */
    }
    { /* Extras */
        data.writeInt32(-1); /* dummy, will hold length */
        data.writeInt32(0x4C444E42); // 'B' 'N' 'D' 'L'
        int oldPos = data.dataPosition();
        { /* writeMapInternal */
            data.writeInt32(7); /* writeMapInternal - size */

            data.writeInt32(VAL_STRING);
            data.writeString16(String16("caller_uid"));
            data.writeInt32(VAL_INTEGER);
            data.writeInt32(ctx->from.uid);

            data.writeInt32(VAL_STRING);
            data.writeString16(String16("caller_bin"));
            data.writeInt32(VAL_STRING);
            data.writeString16(String16(ctx->from.bin));

            data.writeInt32(VAL_STRING);
            data.writeString16(String16("desired_uid"));
            data.writeInt32(VAL_INTEGER);
            data.writeInt32(ctx->to.uid);

            data.writeInt32(VAL_STRING);
            data.writeString16(String16("desired_cmd"));
            data.writeInt32(VAL_STRING);
            data.writeString16(String16(get_command(&ctx->to)));

            data.writeInt32(VAL_STRING);
            data.writeString16(String16("socket"));
            data.writeInt32(VAL_STRING);
            data.writeString16(String16(socket_path));
            
            data.writeInt32(VAL_STRING);
            data.writeString16(String16("allow"));
            data.writeInt32(VAL_INTEGER);
            data.writeInt32(allow);
            
            data.writeInt32(VAL_STRING);
            data.writeString16(String16("version_code"));
            data.writeInt32(VAL_INTEGER);
            data.writeInt32(VERSION_CODE);
        }
        int newPos = data.dataPosition();
        data.setDataPosition(oldPos - 8);
        data.writeInt32(newPos - oldPos); /* length */
        data.setDataPosition(newPos);
    }

    if (ctx->sdk_version >= 15 && ctx->htc)
        data.writeInt32(0); /* HtcFlags */
    data.writeString16(NULL, 0); /* resolvedType */

    data.writeStrongBinder(NULL); /* resultTo */
    data.writeInt32(-1); /* resultCode */
    data.writeString16(NULL, 0); /* resultData */

    data.writeInt32(-1); /* resultExtras */
    
    data.writeString16(String16("com.noshufou.android.su.RESPOND")); /* perm */
    data.writeInt32(0); /* serialized */
    data.writeInt32(0); /* sticky */
    
    status_t ret = am->transact(BROADCAST_INTENT_TRANSACTION, data, &reply);
    if (ret < START_SUCCESS) return -1;

    return 0;
}
