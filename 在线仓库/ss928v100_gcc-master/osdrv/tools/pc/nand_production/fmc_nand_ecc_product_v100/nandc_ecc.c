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
#include "securec.h"
#include "nandc_ecc.h"
#include <stdio.h>
#include "oob_config.h"

/*
 *enc_data_last[24] initial value 0xFFFFFF ;
 *[  23    22    21     20   19    18   17    16   15    14  ]
 *P2048 P2048' P1024 P1024' P512 P512' P256 P256' P128 P128'
 *[ 13  12   11  10   9   8    7  6  5   4  3   2  1   0 ]
 *P64 P64' P32 P32' P16 P16' P8 P8' P4 P4' P2 P2' P1 P1'
 *addr said how many bytes
 */
void make_ecc_1bit(unsigned int addr, unsigned char data_in, unsigned char *enc_data,
		   const unsigned char *enc_data_last)
{
	int i;
	/* ecc_data_in to ECC calculate module */
	unsigned char ecc_l1[ECC_L1_LEN];
	unsigned char ecc_h1[ECC_H1_LEN];
	unsigned char nxt_ecc_reg[ENC_DATA_LEN];
	unsigned char ecc_p4ol;
	unsigned char ecc_p4el;
	unsigned char ecc_p2ol;
	unsigned char ecc_p2el;
	unsigned char ecc_p1ol;
	unsigned char ecc_p1el;
	unsigned char ecc_p4oh;
	unsigned char ecc_p4eh;
	unsigned char ecc_p2oh;
	unsigned char ecc_p2eh;
	unsigned char ecc_p1oh;
	unsigned char ecc_p1eh;
	unsigned char ecc_p8o;
	unsigned char ecc_p8e;
	unsigned char ecc_p4o;
	unsigned char ecc_p4e;
	unsigned char ecc_p2o;
	unsigned char ecc_p2e;
	unsigned char ecc_p1o;
	unsigned char ecc_p1e;
	unsigned char ecc_pr;
	unsigned char ecc_data_in[ECC_DATA_IN_LEN] = {0};
	unsigned char addr_in[ADDR_IN_LEN] = {0};
	for (i = 0; i < (ECC_DATA_IN_LEN / 2); i++) /* div 2:first 8 bytes init*/
		ecc_data_in[i] = (data_in >> i) & 0x1;

	for (i = 0; i < ADDR_IN_LEN; i++)
		addr_in[i] = (addr >> i) & 0x1;

	ecc_l1[0] = ecc_data_in[0] ^ ecc_data_in[1];
	ecc_l1[1] = ecc_data_in[0] ^ ecc_data_in[2];
	ecc_l1[2] = ecc_data_in[1] ^ ecc_data_in[3];
	ecc_l1[3] = ecc_data_in[2] ^ ecc_data_in[3];
	ecc_l1[4] = ecc_data_in[4] ^ ecc_data_in[5];
	ecc_l1[5] = ecc_data_in[4] ^ ecc_data_in[6];
	ecc_l1[6] = ecc_data_in[5] ^ ecc_data_in[7];
	ecc_l1[7] = ecc_data_in[6] ^ ecc_data_in[7];

	/* level one ECC calculate for high 8bit */

	ecc_h1[0] = ecc_data_in[8] ^ ecc_data_in[9];
	ecc_h1[1] = ecc_data_in[8] ^ ecc_data_in[10];
	ecc_h1[2] = ecc_data_in[9] ^ ecc_data_in[11];
	ecc_h1[3] = ecc_data_in[10] ^ ecc_data_in[11];
	ecc_h1[4] = ecc_data_in[12] ^ ecc_data_in[13];
	ecc_h1[5] = ecc_data_in[12] ^ ecc_data_in[14];
	ecc_h1[6] = ecc_data_in[13] ^ ecc_data_in[15];
	ecc_h1[7] = ecc_data_in[14] ^ ecc_data_in[15];

	/* level two ECC calculate for low 8bit */
	ecc_p4ol = ecc_l1[1] ^ ecc_l1[2];
	ecc_p2ol = ecc_l1[0] ^ ecc_l1[4];
	ecc_p1ol = ecc_l1[1] ^ ecc_l1[5];
	ecc_p1el = ecc_l1[2] ^ ecc_l1[6];
	ecc_p2el = ecc_l1[3] ^ ecc_l1[7];
	ecc_p4el = ecc_l1[5] ^ ecc_l1[6];

	/* level two ECC calculate for high 8bit */
	ecc_p4oh = ecc_h1[1] ^ ecc_h1[2];
	ecc_p2oh = ecc_h1[0] ^ ecc_h1[4];
	ecc_p1oh = ecc_h1[1] ^ ecc_h1[5];
	ecc_p1eh = ecc_h1[2] ^ ecc_h1[6];
	ecc_p2eh = ecc_h1[3] ^ ecc_h1[7];
	ecc_p4eh = ecc_h1[5] ^ ecc_h1[6];

	/* level three ECC calculate */
	ecc_p1o = ecc_p1ol ^ ecc_p1oh;
	ecc_p1e = ecc_p1el ^ ecc_p1eh;
	ecc_p2o = ecc_p2ol ^ ecc_p2oh;
	ecc_p2e = ecc_p2el ^ ecc_p2eh;
	ecc_p4o = ecc_p4ol ^ ecc_p4oh;
	ecc_p4e = ecc_p4el ^ ecc_p4eh;
	ecc_p8o = ecc_p2ol ^ ecc_p2el;
	ecc_p8e = ecc_p2oh ^ ecc_p2eh;

	/* line ECC calculate */
	ecc_pr  = ecc_p8o  ^ ecc_p8e;

	/* generate all row and column ecc result for one read/write cylce nxt_ecc_reg[23:0] = 24'h0; */
	nxt_ecc_reg[0] = ecc_p1o;
	nxt_ecc_reg[1] = ecc_p1e;
	nxt_ecc_reg[2] = ecc_p2o;
	nxt_ecc_reg[3] = ecc_p2e;
	nxt_ecc_reg[4] = ecc_p4o;
	nxt_ecc_reg[5] = ecc_p4e;

	nxt_ecc_reg[6] = (~addr_in[0]) & ecc_pr;
	nxt_ecc_reg[7] =   addr_in[0]  & ecc_pr;

	nxt_ecc_reg[8]  = (~addr_in[1]) & ecc_pr;
	nxt_ecc_reg[9]  =   addr_in[1]  & ecc_pr;
	nxt_ecc_reg[10] = (~addr_in[2]) & ecc_pr;
	nxt_ecc_reg[11] =   addr_in[2]  & ecc_pr;
	nxt_ecc_reg[12] = (~addr_in[3]) & ecc_pr;
	nxt_ecc_reg[13] =   addr_in[3]  & ecc_pr;
	nxt_ecc_reg[14] = (~addr_in[4]) & ecc_pr;
	nxt_ecc_reg[15] =   addr_in[4]  & ecc_pr;
	nxt_ecc_reg[16] = (~addr_in[5]) & ecc_pr;
	nxt_ecc_reg[17] =   addr_in[5]  & ecc_pr;
	nxt_ecc_reg[18] = (~addr_in[6]) & ecc_pr;
	nxt_ecc_reg[19] =   addr_in[6]  & ecc_pr;
	nxt_ecc_reg[20] = (~addr_in[7]) & ecc_pr;
	nxt_ecc_reg[21] =   addr_in[7]  & ecc_pr;
	nxt_ecc_reg[22] = (~addr_in[8]) & ecc_pr;
	nxt_ecc_reg[23] =   addr_in[8]  & ecc_pr;

	for (i = 0; i < ENC_DATA_LEN; i++)
		enc_data[i] = enc_data_last[i] ^ nxt_ecc_reg[i];
	return;
}

#define USER_DATA_LEN_16		16
#define USER_DATA_LEN_256		256
void ecc_1bit_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	unsigned int i;
	unsigned char enc_data_last[ENC_DATA_LEN] = {0x1};
	unsigned char enc_data[ENC_DATA_LEN];

	if (memset_s(enc_data_last, ENC_DATA_LEN, 0x1, ENC_DATA_LEN))
		return;

	for (i = 0; i < len; i++) {
		make_ecc_1bit(i, data[i], enc_data, enc_data_last);
		memcpy_s(enc_data_last, ENC_DATA_LEN, enc_data, ENC_DATA_LEN);
	}

	if (len == USER_DATA_LEN_16) {
		enc_data[14] = 0x1;
		enc_data[15] = 0x1;
	}

	if (len >= USER_DATA_LEN_256) {
		for (i = 0; i < 3; i++) /* 3 cycle: enc_data[0] ~ ecc_data[ENC_DATA_LEN - 1] */
			ecc_code[2 - i] = enc_data[i * 8]  + (enc_data[i * 8 + 1] << 1) +
					  (enc_data[i * 8 + 2] << 2) + (enc_data[i * 8 + 3] << 3) +
					  (enc_data[i * 8 + 4] << 4) + (enc_data[i * 8 + 5] << 5) +
					  (enc_data[i * 8 + 6] << 6) + (enc_data[i * 8 + 7] << 7);
	} else {
		for (i = 0; i < 2; i++) /* 2 cycle: enc_data[0] ~ ecc_data[(ENC_DATA_LEN / 2) - 1] */
			ecc_code[1 - i] = enc_data[i * 8]  + (enc_data[i * 8 + 1] << 1) +
					  (enc_data[i * 8 + 2] << 2) + (enc_data[i * 8 + 3] << 3) +
					  (enc_data[i * 8 + 4] << 4) + (enc_data[i * 8 + 5] << 5) +
					  (enc_data[i * 8 + 6] << 6) + (enc_data[i * 8 + 7] << 7);
	}
}

#define MAX_LFSR_BITS 2048

int lfsr_len;
char lfsr_poly[MAX_LFSR_BITS];
char lfsr_value[MAX_LFSR_BITS];

void inttolfsr(char *lfsr, int value)
{
	int i;
	for (i = 0; i <= lfsr_len; i++) {
		if (value & (1 << i))
			lfsr[i] = 1;
		else
			lfsr[i] = 0;
	}
}
void strtolfsr(char *lfsr, char *value)
{
	unsigned int i;
	char c;
	size_t len = strlen(value);

	for (i = 0; i < len; i++) {
		c = *(value + len - 1 - i);
		if (c == '1')
			lfsr[i] = 1;
		else
			lfsr[i] = 0;
	}
}

void lfsr_init(int len, char *poly, int value)
{
	lfsr_len = len;
	memset_s(lfsr_poly, MAX_LFSR_BITS, 0x00, MAX_LFSR_BITS);
	memset_s(lfsr_value, MAX_LFSR_BITS, 0x00, MAX_LFSR_BITS);
	strtolfsr(lfsr_poly, poly);
	inttolfsr(lfsr_value, value);
}

void parity_lfsr_shift(int din)
{
	char feedback;
	int i;

	feedback = lfsr_value[lfsr_len - 1] ^ din;

	for (i = lfsr_len - 1; i > 0; i--)
		lfsr_value[i] = (feedback & lfsr_poly[i]) ^ lfsr_value[i - 1];

	lfsr_value[0] = (feedback & lfsr_poly[0]);
}

void get_parity(char *parity)
{
	int i;
	unsigned int value;
	int shift;

	shift = 0;
	value = 0;
	for (i = lfsr_len - 1; i >= 0; i--) {
		value |= lfsr_value[i] << shift;
		shift++;
		if (shift == 8) {
			*parity = value;
			parity++;
			shift = 0;
			value = 0;
		}
	}
}

int ecc_parity_gen(const unsigned char *data, unsigned int bits, unsigned int ecc_level,
		   unsigned char *ecc_code)
{
	unsigned int i;
	unsigned char c;

	switch (ecc_level) {
	case ECC_LEVLE_4BIT:
		lfsr_init(ECC_LEN_MULTY_14 * ECC_LEVLE_4BIT,
			  "b1111111001111011100101111111111001010011100001000011110001110110010110011110001001110011110011010101110000101101",
			  0);
		break;
	case ECC_LEVLE_8BIT:
		lfsr_init(ECC_LEN_MULTY_14 * ECC_LEVLE_8BIT,
			  "b11001100001001000000110001110000010010101010001101001001000101001100001010100010100100010010000001011001010011110111111111010111111000111100001111010101100110000100010011101001111011011000100110101010100000110111011011001111",
			  0);
		break;
	case ECC_LEVLE_24BIT1K:
		lfsr_init(ECC_LEN_MULTY_14 * ECC_LEVLE_24BIT1K,
			  "b100011101001010011100000001001001000110110010000100111010010101101000101001001010111001011010001111011011101100111010000100110001111111001110011000011101000111010001101001001101100001011010010001010001001001110100011101000000100100001011011110100001010101101101110000010110100100110010010100110100011010101101011110101000011000011101111",
			  0);
		break;
	/* 28bit, todo */
	case ECC_LEVLE_40BIT1K:
		lfsr_init(ECC_LEN_MULTY_14 * ECC_LEVLE_40BIT1K,
			  "b11000000011111111000100110110001101000001101110001011101100101100110000110011111001100101101000001001001011001110101010011110110110111101001110101001111100100111111010100100111111011110001010011101111101100001111110101010011100110010001010110101000001011001100110110010010010101010010100010000000001100000100011101111101111011100011111100110011100010100101100111101100010111111010001000010000101011111110001011101111110111111010111011010010010001001101111100110001010011011010010100000111011000101011011100100100101000000000001010011100111011110010110111000001",
			  0);
		break;
	case ECC_LEVLE_64BIT1K:
		lfsr_init(ECC_LEN_MULTY_14 * ECC_LEVLE_64BIT1K,
			  "b11110111101100010100111001110011001101100000001111000111100111101110101100101110110101100000111111010111101000101000000101101100011001001001110111010011000111100000011110001110010011001001101100011111000100010001010111000101010100011111000100011101000100110100010100000110000101101000101111010000111111010100100001011101011010111010101011010001000101101101011011001110111111100001010111101110010100000010000111111011001110001011001011100000001100010000000000011101000010011001111110001000001010010001010101001011010011010111001100010001101001011011001100111111111100011100001110100001001000110011011110101100001010101011000101010010101001111000011010000100011010101010000101011101001010011101100011010101100001011011101100001111001101100110111011110011100001010100001110111101111101100010010001010110110101100010001110000000101110100100010111000111111101001110000100000001011110100001000001010101",
			  0);
		break;
	default:
		return -1;
	}

	for (i = 0; i < bits; i++) {
		c = *(data + (i >> 3));
		c = (c >> (i & 0x7)) & 0x1;
		parity_lfsr_shift(c);
	}

	get_parity(ecc_code);

	return 0;
}

void ecc_data_gen(unsigned char *data, unsigned int len)
{
	unsigned int i;
	unsigned char *value;

	for (i = 0; i < len; i++)
		data[i] = ~data[i];
}

int ecc_4bit_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_4BIT, ecc_code);
	for (i = 0; i < ECC_LEN_4BIT; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

int ecc_8bit_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_8BIT, ecc_code);
	for (i = 0; i < ECC_LEN_8BIT; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

int ecc_13bit_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_13BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_13BIT; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

int ecc_24bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_24BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_24BIT1K; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

int ecc_27bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_27BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_27BIT1K; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

int ecc_40bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_40BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_40BIT1K; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

int ecc_64bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_64BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_64BIT1K; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

int ecc_41bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_41BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_41BIT1K; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

int ecc_60bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_60BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_60BIT1K; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

int ecc_80bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_80BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_80BIT1K; i++)
		ecc_code[i] = ~ecc_code[i];
	return ret;
}

static int ecc_gen_8bit1k_2K(unsigned char *buf,
			     unsigned char *pagebuf,
			     struct oobuse_info *info,
			     unsigned char *ecc_buf,
			     unsigned int pagesize)
{
	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_8BIT1K, 0xFF, ECC_LEN_8BIT1K))
		return -1;
	ecc_4bit_gen(buf, DATA_LEN0_8BIT1K_2K, ecc_buf);
	if (memcpy_s(pagebuf + DATA_LEN0_8BIT1K_2K, ECC_LEN_8BIT1K, ecc_buf, ECC_LEN_8BIT1K))
		return -1;
	/* 1040,1032.etc These means the number of the data for ecc */
	if (memcpy_s(pagebuf + DATA_LEN0_8BIT1K_2K + ECC_LEN_8BIT1K, DATA_LEN1_8BIT1K_2K,
		     buf + DATA_LEN0_8BIT1K_2K, DATA_LEN1_8BIT1K_2K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_8BIT1K_2K + ECC_LEN_8BIT1K + DATA_LEN1_8BIT1K_2K, BB_LEN,
		     buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_8BIT1K_2K + ECC_LEN_8BIT1K + DATA_LEN1_8BIT1K_2K + BB_LEN,
		     ECC_LEN_8BIT1K, buf + DATA_LEN0_8BIT1K_2K + DATA_LEN1_8BIT1K_2K, DATA_LEN2_8BIT1K_2K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_8BIT1K_2K + ECC_LEN_8BIT1K + DATA_LEN1_8BIT1K_2K + BB_LEN +
		     DATA_LEN2_8BIT1K_2K + ECC_LEN_8BIT1K, CTRL_LEN_8BIT1K_2K, buf + pagesize + BB_LEN,
		     CTRL_LEN_8BIT1K_2K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_8BIT1K, 0xFF, ECC_LEN_8BIT1K))
		return -1;
	ecc_4bit_gen(buf + DATA_LEN0_8BIT1K_2K, DATA_LEN0_8BIT1K_2K, ecc_buf);
	if (memcpy_s(pagebuf + DATA_LEN0_8BIT1K_2K + ECC_LEN_8BIT1K + DATA_LEN1_8BIT1K_2K + BB_LEN +
		     DATA_LEN2_8BIT1K_2K, ECC_LEN_8BIT1K, ecc_buf, ECC_LEN_8BIT1K))
		return -1;
}

static int ecc_gen_8bit1k_4K(unsigned char *buf,
			     unsigned char *pagebuf,
			     struct oobuse_info *info,
			     unsigned char *ecc_buf,
			     unsigned int pagesize)
{
	unsigned int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;

	for (i = 0; i < TOTAL_NUMS_4K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_8BIT1K_4K + ECC_LEN_8BIT1K) * i, DATA_LEN0_8BIT1K_4K,
			     buf + DATA_LEN0_8BIT1K_4K * i, DATA_LEN0_8BIT1K_4K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_8BIT1K, 0xFF, ECC_LEN_8BIT1K))
			return -1;
		ecc_4bit_gen(buf + DATA_LEN0_8BIT1K_4K * i, DATA_LEN0_8BIT1K_4K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_8BIT1K_4K + ECC_LEN_8BIT1K) * i + DATA_LEN0_8BIT1K_4K,
			     ECC_LEN_8BIT1K, ecc_buf, ECC_LEN_8BIT1K))
			return -1;
	}

	if (memcpy_s(pagebuf + (DATA_LEN0_8BIT1K_4K + ECC_LEN_8BIT1K) * TOTAL_NUMS_4K,
		     DATA_LEN1_8BIT1K_4K,
		     buf + DATA_LEN0_8BIT1K_4K * TOTAL_NUMS_4K, DATA_LEN1_8BIT1K_4K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_8BIT1K_4K + ECC_LEN_8BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_8BIT1K_4K, BB_LEN,
		     buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_8BIT1K_4K + ECC_LEN_8BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_8BIT1K_4K + BB_LEN,
		     DATA_LEN2_8BIT1K_4K,
		     buf + DATA_LEN0_8BIT1K_4K * TOTAL_NUMS_4K + DATA_LEN1_8BIT1K_4K, DATA_LEN2_8BIT1K_4K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_8BIT1K_4K + ECC_LEN_8BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_8BIT1K_4K + BB_LEN
		     + DATA_LEN2_8BIT1K_4K + ECC_LEN_8BIT1K, CTRL_LEN_8BIT1K_4K,
		     buf + pagesize + BB_LEN, CTRL_LEN_8BIT1K_4K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_8BIT1K, 0xFF, ECC_LEN_8BIT1K))
		return -1;
	ecc_4bit_gen(buf + DATA_LEN0_8BIT1K_4K * TOTAL_NUMS_4K, DATA_LEN0_8BIT1K_4K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_8BIT1K_4K + ECC_LEN_8BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_8BIT1K_4K + BB_LEN
		     + DATA_LEN2_8BIT1K_4K, ECC_LEN_8BIT1K, ecc_buf, ECC_LEN_8BIT1K))
		return -1;
}

static int ecc_gen_16bit1k_2K(unsigned char *buf,
			      unsigned char *pagebuf,
			      struct oobuse_info *info,
			      unsigned char *ecc_buf,
			      unsigned int pagesize)
{
	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize +
		     info->oobuse))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_16BIT1K, 0xFF, ECC_LEN_16BIT1K))
		return -1;
	ecc_8bit_gen(buf, DATA_LEN0_16BIT1K_2K, ecc_buf);
	if (memcpy_s(pagebuf + DATA_LEN0_16BIT1K_2K, ECC_LEN_16BIT1K, ecc_buf, ECC_LEN_16BIT1K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_16BIT1K_2K + ECC_LEN_16BIT1K, DATA_LEN1_16BIT1K_2K,
		     buf + DATA_LEN0_16BIT1K_2K, DATA_LEN1_16BIT1K_2K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_16BIT1K_2K + ECC_LEN_16BIT1K + DATA_LEN1_16BIT1K_2K, BB_LEN,
		     buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_16BIT1K_2K + ECC_LEN_16BIT1K + DATA_LEN1_16BIT1K_2K + BB_LEN,
		     DATA_LEN2_16BIT1K_2K, buf + DATA_LEN0_16BIT1K_2K + DATA_LEN1_16BIT1K_2K, DATA_LEN2_16BIT1K_2K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_16BIT1K_2K + ECC_LEN_16BIT1K + DATA_LEN1_16BIT1K_2K + BB_LEN +
		     DATA_LEN2_16BIT1K_2K + ECC_LEN_16BIT1K, CTRL_LEN_16BIT1K_2K, buf + pagesize + BB_LEN,
		     CTRL_LEN_16BIT1K_2K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_16BIT1K, 0xFF, ECC_LEN_16BIT1K))
		return -1;
	ecc_8bit_gen(buf + DATA_LEN0_16BIT1K_2K, DATA_LEN0_16BIT1K_2K, ecc_buf);
	if (memcpy_s(pagebuf + DATA_LEN0_16BIT1K_2K + ECC_LEN_16BIT1K + DATA_LEN1_16BIT1K_2K + BB_LEN +
		     DATA_LEN2_16BIT1K_2K, ECC_LEN_16BIT1K, ecc_buf, ECC_LEN_16BIT1K))
		return -1;
}

static int ecc_gen_16bit1k_4K(unsigned char *buf,
			      unsigned char *pagebuf,
			      struct oobuse_info *info,
			      unsigned char *ecc_buf,
			      unsigned int pagesize)
{
	int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	for (i = 0; i < TOTAL_NUMS_4K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_16BIT1K_4K + ECC_LEN_16BIT1K) * i, DATA_LEN0_16BIT1K_4K,
			     buf + DATA_LEN0_16BIT1K_4K * i, DATA_LEN0_16BIT1K_4K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_16BIT1K, 0xFF, ECC_LEN_16BIT1K))
			return -1;
		ecc_8bit_gen(buf + DATA_LEN0_16BIT1K_4K * i, DATA_LEN0_16BIT1K_4K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_16BIT1K_4K + ECC_LEN_16BIT1K) * i + DATA_LEN0_16BIT1K_4K,
			     ECC_LEN_16BIT1K, ecc_buf, ECC_LEN_16BIT1K))
			return -1;
	}
	if (memcpy_s(pagebuf + (DATA_LEN0_16BIT1K_4K + ECC_LEN_16BIT1K) * TOTAL_NUMS_4K,
		     DATA_LEN1_16BIT1K_4K,
		     buf + DATA_LEN0_16BIT1K_4K * TOTAL_NUMS_4K, DATA_LEN1_16BIT1K_4K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_16BIT1K_4K + ECC_LEN_16BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_16BIT1K_4K,
		     BB_LEN, buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_16BIT1K_4K + ECC_LEN_16BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_16BIT1K_4K +
		     BB_LEN, DATA_LEN2_16BIT1K_4K,
		     buf + DATA_LEN0_16BIT1K_4K * TOTAL_NUMS_4K + DATA_LEN1_16BIT1K_4K, DATA_LEN2_16BIT1K_4K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_16BIT1K_4K + ECC_LEN_16BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_16BIT1K_4K +
		     BB_LEN + DATA_LEN2_16BIT1K_4K + ECC_LEN_16BIT1K, CTRL_LEN_16BIT1K_4K,
		     buf + pagesize + BB_LEN, CTRL_LEN_16BIT1K_4K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_16BIT1K, 0xFF, ECC_LEN_16BIT1K))
		return -1;
	ecc_8bit_gen(buf + DATA_LEN0_16BIT1K_4K * TOTAL_NUMS_4K, DATA_LEN0_16BIT1K_4K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_16BIT1K_4K + ECC_LEN_16BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_16BIT1K_4K +
		     BB_LEN + DATA_LEN2_16BIT1K_4K, ECC_LEN_16BIT1K, ecc_buf, ECC_LEN_16BIT1K))
		return -1;
}

static int ecc_gen_24bit1k_2K(unsigned char *buf,
			      unsigned char *pagebuf,
			      struct oobuse_info *info,
			      unsigned char *ecc_buf,
			      unsigned int pagesize)
{
	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_24BIT1K, 0xFF, ECC_LEN_24BIT1K))
		return -1;
	ecc_24bit1k_gen(buf, DATA_LEN0_24BIT1K_2K, ecc_buf);
	if (memcpy_s(pagebuf + DATA_LEN0_24BIT1K_2K, ECC_LEN_24BIT1K, ecc_buf, ECC_LEN_24BIT1K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_24BIT1K_2K + ECC_LEN_24BIT1K, DATA_LEN1_24BIT1K_2K,
		     buf + DATA_LEN0_24BIT1K_2K, DATA_LEN1_24BIT1K_2K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_24BIT1K_2K + ECC_LEN_24BIT1K + DATA_LEN1_24BIT1K_2K, BB_LEN,
		     buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_24BIT1K_2K + ECC_LEN_24BIT1K + DATA_LEN1_24BIT1K_2K + BB_LEN,
		     DATA_LEN2_24BIT1K_2K, buf + DATA_LEN0_24BIT1K_2K + DATA_LEN1_24BIT1K_2K, DATA_LEN2_24BIT1K_2K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_24BIT1K_2K + ECC_LEN_24BIT1K + DATA_LEN1_24BIT1K_2K + BB_LEN +
		     DATA_LEN2_24BIT1K_2K + ECC_LEN_24BIT1K, CTRL_LEN_24BIT1K_2K,
		     buf + pagesize + BB_LEN, CTRL_LEN_24BIT1K_2K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_24BIT1K, 0xFF, ECC_LEN_24BIT1K))
		return -1;
	ecc_24bit1k_gen(buf + DATA_LEN0_24BIT1K_2K, DATA_LEN0_24BIT1K_2K, ecc_buf);
	if (memcpy_s(pagebuf + DATA_LEN0_24BIT1K_2K + ECC_LEN_24BIT1K + DATA_LEN1_24BIT1K_2K + BB_LEN +
		     DATA_LEN2_24BIT1K_2K, DATA_LEN2_24BIT1K_2K, ecc_buf, ECC_LEN_24BIT1K))
		return -1;
}

static int ecc_gen_24bit1k_4K(unsigned char *buf,
			      unsigned char *pagebuf,
			      struct oobuse_info *info,
			      unsigned char *ecc_buf,
			      unsigned int pagesize)
{
	int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	for (i = 0; i < TOTAL_NUMS_4K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_4K + ECC_LEN_24BIT1K) * i, DATA_LEN0_24BIT1K_4K,
			     buf + DATA_LEN0_24BIT1K_4K * i, DATA_LEN0_24BIT1K_4K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_24BIT1K, 0xFF, ECC_LEN_24BIT1K))
			return -1;
		ecc_24bit1k_gen(buf + DATA_LEN0_24BIT1K_4K * i, DATA_LEN0_24BIT1K_4K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_4K + ECC_LEN_24BIT1K) * i + DATA_LEN0_24BIT1K_4K,
			     ECC_LEN_24BIT1K, ecc_buf, ECC_LEN_24BIT1K))
			return -1;
	}
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_4K + ECC_LEN_24BIT1K) * TOTAL_NUMS_4K,
		     DATA_LEN1_24BIT1K_4K,
		     buf + DATA_LEN0_24BIT1K_4K * TOTAL_NUMS_4K, DATA_LEN1_24BIT1K_4K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_4K + ECC_LEN_24BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_24BIT1K_4K,
		     BB_LEN, buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_4K + ECC_LEN_24BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_24BIT1K_4K +
		     BB_LEN, DATA_LEN2_24BIT1K_4K,
		     buf + DATA_LEN0_24BIT1K_4K * TOTAL_NUMS_4K + DATA_LEN1_24BIT1K_4K, DATA_LEN2_24BIT1K_4K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_4K + ECC_LEN_24BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_24BIT1K_4K +
		     BB_LEN + DATA_LEN2_24BIT1K_4K + ECC_LEN_24BIT1K, CTRL_LEN_24BIT1K_4K,
		     buf + pagesize + BB_LEN, CTRL_LEN_24BIT1K_4K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_24BIT1K, 0xFF, ECC_LEN_24BIT1K))
		return -1;
	ecc_24bit1k_gen(buf + DATA_LEN0_24BIT1K_4K * TOTAL_NUMS_4K, DATA_LEN0_24BIT1K_4K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_4K + ECC_LEN_24BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_24BIT1K_4K +
		     BB_LEN + DATA_LEN2_24BIT1K_4K, ECC_LEN_24BIT1K, ecc_buf, ECC_LEN_24BIT1K))
		return -1;
}

static int ecc_gen_24bit1k_8K(unsigned char *buf,
			      unsigned char *pagebuf,
			      struct oobuse_info *info,
			      unsigned char *ecc_buf,
			      unsigned int pagesize)
{
	int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	for (i = 0; i < TOTAL_NUMS_8K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_8K + ECC_LEN_24BIT1K) * i, DATA_LEN0_24BIT1K_8K,
			     buf + DATA_LEN0_24BIT1K_8K * i, DATA_LEN0_24BIT1K_8K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_24BIT1K, 0xFF, ECC_LEN_24BIT1K))
			return -1;
		ecc_24bit1k_gen(buf + DATA_LEN0_24BIT1K_8K * i, DATA_LEN0_24BIT1K_8K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_8K + ECC_LEN_24BIT1K) * i + DATA_LEN0_24BIT1K_8K,
			     ECC_LEN_24BIT1K, ecc_buf, ECC_LEN_24BIT1K))
			return -1;
	}
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_8K + ECC_LEN_24BIT1K) * TOTAL_NUMS_8K,
		     DATA_LEN1_24BIT1K_8K,
		     buf + DATA_LEN0_24BIT1K_8K * TOTAL_NUMS_8K, DATA_LEN1_24BIT1K_8K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_8K + ECC_LEN_24BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_24BIT1K_8K,
		     BB_LEN, buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_8K + ECC_LEN_24BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_24BIT1K_8K +
		     BB_LEN, DATA_LEN2_24BIT1K_8K,
		     buf + DATA_LEN0_24BIT1K_8K * TOTAL_NUMS_8K + DATA_LEN1_24BIT1K_8K, DATA_LEN2_24BIT1K_8K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_8K + ECC_LEN_24BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_24BIT1K_8K +
		     BB_LEN + DATA_LEN2_24BIT1K_8K + ECC_LEN_24BIT1K, CTRL_LEN_24BIT1K_8K,
		     buf + pagesize + BB_LEN, CTRL_LEN_24BIT1K_8K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_24BIT1K, 0xFF, ECC_LEN_24BIT1K))
		return -1;
	ecc_24bit1k_gen(buf + DATA_LEN0_24BIT1K_8K * TOTAL_NUMS_8K, DATA_LEN0_24BIT1K_8K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_8K + ECC_LEN_24BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_24BIT1K_8K +
		     BB_LEN + DATA_LEN2_24BIT1K_8K, ECC_LEN_24BIT1K, ecc_buf, ECC_LEN_24BIT1K))
		return -1;
}

static int ecc_gen_40bit1k_8K(unsigned char *buf,
			      unsigned char *pagebuf,
			      struct oobuse_info *info,
			      unsigned char *ecc_buf,
			      unsigned int pagesize)
{
	int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	for (i = 0; i < TOTAL_NUMS_8K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_8K + ECC_LEN_40BIT1K) * i, DATA_LEN0_40BIT1K_8K,
			     buf + DATA_LEN0_40BIT1K_8K * i, DATA_LEN0_40BIT1K_8K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_40BIT1K, 0xFF, ECC_LEN_40BIT1K))
			return -1;
		ecc_40bit1k_gen(buf + DATA_LEN0_40BIT1K_8K * i, DATA_LEN0_40BIT1K_8K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_8K + ECC_LEN_40BIT1K) * i + DATA_LEN0_40BIT1K_8K,
			     ECC_LEN_40BIT1K, ecc_buf, ECC_LEN_40BIT1K))
			return -1;
	}
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_8K + ECC_LEN_40BIT1K) * TOTAL_NUMS_8K,
		     DATA_LEN1_40BIT1K_8K,
		     buf + DATA_LEN0_40BIT1K_8K * TOTAL_NUMS_8K, DATA_LEN1_40BIT1K_8K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_8K + ECC_LEN_40BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_40BIT1K_8K,
		     BB_LEN, buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_8K + ECC_LEN_40BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_40BIT1K_8K +
		     BB_LEN, DATA_LEN2_40BIT1K_8K,
		     buf + DATA_LEN0_40BIT1K_8K * TOTAL_NUMS_8K + DATA_LEN1_40BIT1K_8K, DATA_LEN2_40BIT1K_8K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_8K + ECC_LEN_40BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_40BIT1K_8K +
		     BB_LEN + DATA_LEN2_40BIT1K_8K + ECC_LEN_40BIT1K, CTRL_LEN_40BIT1K_8K,
		     buf + pagesize + BB_LEN, CTRL_LEN_40BIT1K_8K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_40BIT1K, 0xFF, ECC_LEN_40BIT1K))
		return -1;
	ecc_40bit1k_gen(buf + DATA_LEN0_40BIT1K_8K * TOTAL_NUMS_8K, DATA_LEN0_40BIT1K_8K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_8K + ECC_LEN_40BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_40BIT1K_8K +
		     BB_LEN + DATA_LEN2_40BIT1K_8K, ECC_LEN_40BIT1K, ecc_buf, ECC_LEN_40BIT1K))
		return -1;
}

static int ecc_gen_40bit1k_16K(unsigned char *buf,
			       unsigned char *pagebuf,
			       struct oobuse_info *info,
			       unsigned char *ecc_buf,
			       unsigned int pagesize)
{
	int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	for (i = 0; i < TOTAL_NUMS_16K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * i, DATA_LEN0_40BIT1K_16K,
			     buf + DATA_LEN0_40BIT1K_16K * i, DATA_LEN0_40BIT1K_16K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_40BIT1K, 0xFF, ECC_LEN_40BIT1K))
			return -1;
		ecc_40bit1k_gen(buf + DATA_LEN0_40BIT1K_16K * i, DATA_LEN0_40BIT1K_16K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * i + DATA_LEN0_40BIT1K_16K,
			     ECC_LEN_40BIT1K, ecc_buf, ECC_LEN_40BIT1K))
			return -1;
	}

	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * TOTAL_NUMS_16K,
		     DATA_LEN0_40BIT1K_16K,
		     buf + DATA_LEN0_40BIT1K_16K * TOTAL_NUMS_16K, DATA_LEN0_40BIT1K_16K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN0_40BIT1K_16K +
		     ECC_LEN0_40BIT1K_16K, BB_LEN,
		     buf + pagesize, BB_LEN))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_40BIT1K, 0xFF, ECC_LEN_40BIT1K))
		return -1;
	ecc_40bit1k_gen(buf + DATA_LEN0_40BIT1K_16K * TOTAL_NUMS_16K, DATA_LEN0_40BIT1K_16K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN0_40BIT1K_16K,
		     ECC_LEN0_40BIT1K_16K, ecc_buf, ECC_LEN0_40BIT1K_16K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN0_40BIT1K_16K +
		     BB_LEN + ECC_LEN0_40BIT1K_16K, ECC_LEN1_40BIT1K_16K,
		     ecc_buf + TOTAL_NUMS_16K, ECC_LEN1_40BIT1K_16K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN0_40BIT1K_16K +
		     BB_LEN + ECC_LEN0_40BIT1K_16K + ECC_LEN1_40BIT1K_16K, DATA_LEN1_40BIT1K_16K,
		     buf + DATA_LEN0_40BIT1K_16K * TOTAL_NUMS_16K + DATA_LEN0_40BIT1K_16K, DATA_LEN1_40BIT1K_16K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN0_40BIT1K_16K +
		     BB_LEN + ECC_LEN0_40BIT1K_16K + ECC_LEN1_40BIT1K_16K + DATA_LEN1_40BIT1K_16K + ECC_LEN_40BIT1K,
		     CTRL_LEN_40BIT1K_16K, buf + pagesize + BB_LEN, CTRL_LEN_40BIT1K_16K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_40BIT1K, 0xFF, ECC_LEN_40BIT1K))
		return -1;
	ecc_40bit1k_gen(buf + DATA_LEN0_40BIT1K_16K * TOTAL_NUMS_16K + DATA_LEN0_40BIT1K_16K,
			DATA_LEN0_40BIT1K_16K,
			ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN0_40BIT1K_16K +
		     BB_LEN + ECC_LEN0_40BIT1K_16K + ECC_LEN1_40BIT1K_16K + DATA_LEN1_40BIT1K_16K,
		     ECC_LEN_40BIT1K, ecc_buf, ECC_LEN_40BIT1K))
		return -1;
}

static int ecc_gen_64bit1k_8K(unsigned char *buf,
			      unsigned char *pagebuf,
			      struct oobuse_info *info,
			      unsigned char *ecc_buf,
			      unsigned int pagesize)
{
	int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	for (i = 0; i < TOTAL_NUMS_8K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_8K + ECC_LEN_64BIT1K) * i, DATA_LEN0_64BIT1K_8K,
			     buf + DATA_LEN0_64BIT1K_8K * i, DATA_LEN0_64BIT1K_8K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_64BIT1K, 0xFF, ECC_LEN_64BIT1K))
			return -1;
		ecc_64bit1k_gen(buf + DATA_LEN0_64BIT1K_8K * i, DATA_LEN0_64BIT1K_8K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_8K + ECC_LEN_64BIT1K) * i + DATA_LEN0_64BIT1K_8K,
			     ECC_LEN_64BIT1K, ecc_buf, ECC_LEN_64BIT1K))
			return -1;
	}

	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_8K + ECC_LEN_64BIT1K) * TOTAL_NUMS_8K,
		     DATA_LEN1_64BIT1K_8K,
		     buf + DATA_LEN0_64BIT1K_8K * TOTAL_NUMS_8K, ECC_LEN_64BIT1K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_8K + ECC_LEN_64BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_64BIT1K_8K,
		     BB_LEN, buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_8K + ECC_LEN_64BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_64BIT1K_8K +
		     BB_LEN, DATA_LEN2_64BIT1K_8K,
		     buf + DATA_LEN0_64BIT1K_8K * TOTAL_NUMS_8K + DATA_LEN1_64BIT1K_8K, DATA_LEN2_64BIT1K_8K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_8K + ECC_LEN_64BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_64BIT1K_8K +
		     BB_LEN + DATA_LEN2_64BIT1K_8K + ECC_LEN_64BIT1K, CTRL_LEN_64BIT1K_8K,
		     buf + pagesize + BB_LEN, CTRL_LEN_64BIT1K_8K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_64BIT1K, 0xFF, ECC_LEN_64BIT1K))
		return -1;
	ecc_64bit1k_gen(buf + DATA_LEN0_64BIT1K_8K * TOTAL_NUMS_8K, DATA_LEN0_64BIT1K_8K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_8K + ECC_LEN_64BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_64BIT1K_8K +
		     BB_LEN + DATA_LEN2_64BIT1K_8K, ECC_LEN_64BIT1K, ecc_buf, ECC_LEN_64BIT1K))
		return -1;
}

static int ecc_gen_64bit1k_16K(unsigned char *buf,
			       unsigned char *pagebuf,
			       struct oobuse_info *info,
			       unsigned char *ecc_buf,
			       unsigned int pagesize)
{
	int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	for (i = 0; i < TOTAL_NUMS_16K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * i, DATA_LEN0_64BIT1K_16K,
			     buf + DATA_LEN0_64BIT1K_16K * i, DATA_LEN0_64BIT1K_16K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_64BIT1K, 0xFF, ECC_LEN_64BIT1K))
			return -1;
		ecc_64bit1k_gen(buf + DATA_LEN0_64BIT1K_16K * i, DATA_LEN0_64BIT1K_16K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * i + DATA_LEN0_64BIT1K_16K,
			     ECC_LEN_64BIT1K, ecc_buf, ECC_LEN_64BIT1K))
			return -1;
	}

	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * TOTAL_NUMS_16K,
		     DATA_LEN1_64BIT1K_16K,
		     buf + DATA_LEN0_64BIT1K_16K * TOTAL_NUMS_16K, DATA_LEN1_64BIT1K_16K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN1_64BIT1K_16K,
		     BB_LEN, buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN1_64BIT1K_16K +
		     BB_LEN, DATA_LEN2_64BIT1K_16K,
		     buf + DATA_LEN0_64BIT1K_16K * TOTAL_NUMS_16K + DATA_LEN1_64BIT1K_16K, DATA_LEN2_64BIT1K_16K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_64BIT1K, 0xFF, ECC_LEN_64BIT1K))
		return -1;
	ecc_64bit1k_gen(buf + DATA_LEN0_64BIT1K_16K * TOTAL_NUMS_16K, DATA_LEN0_64BIT1K_16K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN1_64BIT1K_16K +
		     BB_LEN + DATA_LEN2_64BIT1K_16K, ECC_LEN_64BIT1K, ecc_buf, ECC_LEN_64BIT1K))
		return -1;

	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN1_64BIT1K_16K +
		     BB_LEN + DATA_LEN2_64BIT1K_16K + ECC_LEN_64BIT1K, DATA_LEN3_64BIT1K_16K,
		     buf + DATA_LEN0_64BIT1K_16K * TOTAL_NUMS_16K + DATA_LEN1_64BIT1K_16K + DATA_LEN2_64BIT1K_16K,
		     DATA_LEN3_64BIT1K_16K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN1_64BIT1K_16K +
		     BB_LEN + DATA_LEN2_64BIT1K_16K + ECC_LEN_64BIT1K + DATA_LEN3_64BIT1K_16K + ECC_LEN_64BIT1K,
		     CTRL_LEN_64BIT1K_16K, buf + pagesize + BB_LEN, CTRL_LEN_64BIT1K_16K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_64BIT1K, 0xFF, ECC_LEN_64BIT1K))
		return -1;
	ecc_64bit1k_gen(buf + DATA_LEN0_64BIT1K_16K * TOTAL_NUMS_16K + DATA_LEN0_64BIT1K_16K,
			DATA_LEN0_64BIT1K_16K,
			ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN1_64BIT1K_16K +
		     BB_LEN + DATA_LEN2_64BIT1K_16K + ECC_LEN_64BIT1K + DATA_LEN3_64BIT1K_16K,
		     ECC_LEN_64BIT1K, ecc_buf, ECC_LEN_64BIT1K))
		return -1;
}

static int ecc_gen_8bit1k(unsigned char *buf,
			  unsigned char *pagebuf,
			  struct oobuse_info *info,
			  enum page_type pagetype)
{
	int ret = 0;
	unsigned char ecc_buf[ECC_LEN_8BIT1K];
	unsigned int pagesize;

	pagesize = get_pagesize(pagetype);

	if (pagetype == PT_PAGESIZE_2K)
		ret = ecc_gen_8bit1k_2K(buf, pagebuf, info, ecc_buf, pagesize);
	else if (pagetype == PT_PAGESIZE_4K)
		ret = ecc_gen_8bit1k_4K(buf, pagebuf, info, ecc_buf, pagesize);

	return ret;
}

static int ecc_gen_16bit1k(unsigned char *buf,
			   unsigned char *pagebuf,
			   struct oobuse_info *info,
			   enum page_type pagetype)
{
	int ret = 0;
	unsigned char ecc_buf[ECC_LEN_16BIT1K];
	unsigned int pagesize;

	pagesize = get_pagesize(pagetype);

	if (pagetype == PT_PAGESIZE_2K)
		ret = ecc_gen_16bit1k_2K(buf, pagebuf, info, ecc_buf, pagesize);
	else if (pagetype == PT_PAGESIZE_4K)
		ret = ecc_gen_16bit1k_4K(buf, pagebuf, info, ecc_buf, pagesize);

	return ret;
}

static int ecc_gen_24bit1k(unsigned char *buf,
			   unsigned char *pagebuf,
			   struct oobuse_info *info,
			   enum page_type pagetype)
{
	int ret = 0;
	unsigned char ecc_buf[ECC_LEN_24BIT1K];
	unsigned int pagesize;

	pagesize = get_pagesize(pagetype);

	if (pagetype == PT_PAGESIZE_2K)
		ret = ecc_gen_24bit1k_2K(buf, pagebuf, info, ecc_buf, pagesize);
	else if (pagetype == PT_PAGESIZE_4K)
		ret = ecc_gen_24bit1k_4K(buf, pagebuf, info, ecc_buf, pagesize);
	else if (pagetype == PT_PAGESIZE_8K)
		ret = ecc_gen_24bit1k_8K(buf, pagebuf, info, ecc_buf, pagesize);

	return ret;
}

static int ecc_gen_40bit1k(unsigned char *buf,
			   unsigned char *pagebuf,
			   struct oobuse_info *info,
			   enum page_type pagetype)
{
	int ret = 0;
	unsigned char ecc_buf[ECC_LEN_40BIT1K];
	unsigned int pagesize;

	pagesize = get_pagesize(pagetype);

	if (pagetype == PT_PAGESIZE_8K)
		ret = ecc_gen_40bit1k_8K(buf, pagebuf, info, ecc_buf, pagesize);
	else if (pagetype == PT_PAGESIZE_16K)
		ret = ecc_gen_40bit1k_16K(buf, pagebuf, info, ecc_buf, pagesize);
	return ret;
}

static int ecc_gen_64bit1k(unsigned char *buf,
			   unsigned char *pagebuf,
			   struct oobuse_info *info,
			   enum page_type pagetype)
{
	int ret = 0;
	unsigned char ecc_buf[ECC_LEN_64BIT1K];
	unsigned int pagesize;

	pagesize = get_pagesize(pagetype);

	if (pagetype == PT_PAGESIZE_8K)
		ret = ecc_gen_64bit1k_8K(buf, pagebuf, info, ecc_buf, pagesize);
	else if (pagetype == PT_PAGESIZE_16K)
		ret = ecc_gen_64bit1k_16K(buf, pagebuf, info, ecc_buf, pagesize);

	return ret;
}

int page_ecc_gen(unsigned char *pagebuf, enum page_type pagetype, enum ecc_type ecctype)
{
	unsigned char buf[MAX_PAGE_SIZE + MAX_OOB_SIZE];
	struct oobuse_info *info;
	int ret = 0;
	info = get_oobuse_info(pagetype, ecctype);
	if (info == NULL)
		return -1;

	switch (ecctype) {
	case ET_ECC_8BIT1K:
		ret = ecc_gen_8bit1k(buf, pagebuf, info, pagetype);
		break;
	case ET_ECC_16BIT1K:
		ret = ecc_gen_16bit1k(buf, pagebuf, info, pagetype);
		break;
	case ET_ECC_24BIT1K:
		ret = ecc_gen_24bit1k(buf, pagebuf, info, pagetype);
		break;
	case ET_ECC_40BIT1K:
		ret = ecc_gen_40bit1k(buf, pagebuf, info, pagetype);
		break;
	case ET_ECC_64BIT1K:
		ret = ecc_gen_64bit1k(buf, pagebuf, info, pagetype);
		break;
	default:
		printf("%s not support ecctype 0x%08x:\n", __FUNCTION__, ecctype);
		break;
	}

	return ret;
}
