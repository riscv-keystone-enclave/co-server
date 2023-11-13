//******************************************************************************
// Copyright (c) 2018, The Regents of the University of California (Regents).
// All Rights Reserved. See LICENSE for license details.
//------------------------------------------------------------------------------
#include <syscall.h>

#include "eapp_utils.h"
#include "edge_call.h"
#include "string.h"
#include "enclave-printf.h"
#define LOG(fmt, ...) \
    e_printf("[%s:%d]@%s: " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
// #include "aes.h"

enum ocall_ids {
    OCALL_PRINT_STRING = 1,
    OCALL_HTTP_ENCRYPT,
};

uint8_t data[4096];
int data_2_encrypt_size = 64;


unsigned long
ocall_print_string(char* string);

unsigned long
ocall_http_encrypt();

int
main() {
    LOG("Enclave: main\n");
    while (1) {
        ocall_http_encrypt();
    }

    EAPP_RETURN(0);
}


static int
encrypt_ecb(uint8_t* data, uint32_t data_len) {
    // uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    //                  0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};

    // struct AES_ctx ctx;

    // AES_init_ctx(&ctx, key);
    // for (int i = 0; i < data_len / 16; i++)
    //     AES_ECB_encrypt(&ctx, data + i * 16);
    for(uint32_t i = 0; i < data_len; ++i)
    {
        data[i] = (uint8_t)i & 0x3f;
    }
    return 0;
}

unsigned long
ocall_print_string(char* string) {
    unsigned long retval;
    ocall(
        OCALL_PRINT_STRING, string, strlen(string) + 1, &retval,
        sizeof(unsigned long));
    // LOG("Enclave: ocall_print_string ret: %lu\n", retval);
    return retval;
}

unsigned long
ocall_http_encrypt() {
    // LOG("encrypting data, size=%d\n", data_2_encrypt_size);
    encrypt_ecb(data, data_2_encrypt_size);
    unsigned long retval = 0;
    ocall(
        OCALL_HTTP_ENCRYPT, data, data_2_encrypt_size, data,
        data_2_encrypt_size);
    if(data_2_encrypt_size != 64)
    {
        LOG("data_2_encrypt_size != 64\n");
        retval = -1;
    }
    return retval;
}
