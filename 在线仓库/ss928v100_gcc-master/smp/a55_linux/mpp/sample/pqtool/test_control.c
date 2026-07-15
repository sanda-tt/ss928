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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>

extern int PQControlMain(int argc, char **argv);

static void *PQControlMainThread(void *arg)
{
    int argNum = 2;
    char *args[] = { "", "start"};
    PQControlMain(argNum, args);
    return NULL;
}

int main(int argc, char* argv[])
{
    // stream start
    pthread_t controlThreadId;

    printf("control_thread start\n");
    pthread_create(&controlThreadId, NULL, PQControlMainThread, NULL);
    printf("control_thread end\n");

    sleep(2); // 2s

    printf("---------------press enter key to exit!---------------\n");
    (td_void)getchar();

    // stream finish

    if (pthread_kill(controlThreadId, SIGINT) != 0) {
        perror("pthread_kill");
    }
    pthread_join(controlThreadId, NULL);

    return 0;
}
