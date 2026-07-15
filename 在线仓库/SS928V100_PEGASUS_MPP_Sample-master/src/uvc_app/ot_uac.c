/*
 * Copyright (c) 2025 HiSilicon (Shanghai) Technologies Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <pthread.h>
#include <unistd.h>
#include "ot_uac.h"
#include "ot_audio.h"

static int __init()
{
    return 0;
}

static int __open()
{
    return 0;
}

static int __close()
{
    ot_audio_shutdown();
    return 0;
}

static int __run()
{
    ot_audio_init();
    ot_audio_startup();

    return 0;
}

/* ---------------------------------------------------------------------- */

static ot_uac g_ot_uac = {
    .init = __init,
    .open = __open,
    .close = __close,
    .run = __run,
};

ot_uac *get_ot_uac()
{
    return &g_ot_uac;
}

void release_ot_uac(ot_uac *uvc) {}
