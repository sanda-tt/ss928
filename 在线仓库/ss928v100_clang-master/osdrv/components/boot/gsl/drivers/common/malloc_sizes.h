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
/***********************************************************************************
* [bootRam Space] start: 0x04010000, end: 0x0402a000, len: 0x1a000(104k).
* [bss + stack]   start: 0x04010000, end: 0x04010a00, len: 0xa00.
* [ddr init or key area] start: 0x04010a00, end: 0x04024600, len: 0x13c00.
* [malloc] start: 0x04024600, end: 0x04026ca0, len: 0x26a0.
***********************************************************************************/
/* [ddr init or key area] start: 0x04010a00, end: 0x04024600, len: 0x13c00. */
cache(0x8000,   2)	/* ddr init or key area reserved */
cache(0x3c00,   1)	/* ddr init or key area reserved */

/* [malloc] start: 0x04024600, end: 0x04026ca0 , len: 0x26a0 */
cache(0x100,	5)	/* rsa pss */
cache(0x200,	5)	/* usb device; rsa decrypt buf for n=4096 */
cache(0x400,	3)	/* rsa_exp_mod (RSA_KEY_LEN_4096*2) */
cache(0x50,     2)	/* usb device */
cache(0xb00,    1)	/* usb device */
/**********************************************************************************/
/**********************************************************************************/
