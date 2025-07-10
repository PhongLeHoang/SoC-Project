#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Hàm chuyển endian (little <-> big) 64-bit
uint64_t to_big_endian_64(uint64_t x) {
    uint64_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result |= ((x >> (8 * i)) & 0xFF) << (56 - 8 * i);
    }
    return result;
}

// In ra 1 block (512 bit = 64 byte) dưới dạng mảng uint32_t
void print_block(FILE *fp, uint8_t* block) {
    fprintf(fp, "{");
    for (int i = 0; i < 16; ++i) {  // 16 * 4 = 64 byte
        uint32_t word = (block[i*4] << 24) | (block[i*4 + 1] << 16) |
                        (block[i*4 + 2] << 8) | (block[i*4 + 3]);
        fprintf(fp, "0x%08x", word);
        if (i != 15) {
            fprintf(fp, ",");
        }
        if ((i + 1) % 4 == 0 && i != 15) {
            fprintf(fp, "\n ");
        }
    }
    fprintf(fp, "},\n");
}

int main() {
    char line[1024];
    FILE *fp = fopen("padded_message.hex", "w");
    FILE *input_fp = fopen("message.txt", "r");

    if (input_fp == NULL) {
        perror("Cannot open file(message.txt)");
        return 1;
    }
    if (fp == NULL) {
        perror("Cannot open file(padded_message.hex)");
        return 1;
    }

    while (fgets(line, sizeof(line), input_fp) != NULL) {
        // Xóa ký tự newline nếu có
        line[strcspn(line, "\r\n")] = 0;
        size_t input_len = strlen(line);

        // Tính tổng độ dài cần sau padding
        uint64_t bit_len = input_len * 8;
        size_t total_len = input_len + 1 + 8; // +1 byte 0x80, +8 byte độ dài
        size_t padding_len = (64 - (total_len % 64)) % 64;
        total_len += padding_len;

        uint8_t* padded = calloc(total_len, 1);
        memcpy(padded, line, input_len);
        padded[input_len] = 0x80; // bit '1'

        // Ghi độ dài vào 8 byte cuối cùng (big-endian)
        uint64_t big_endian_len = to_big_endian_64(bit_len);
        memcpy(padded + total_len - 8, &big_endian_len, 8);

        // In các block 64 byte
        size_t num_blocks = total_len / 64;
        for (size_t i = 0; i < num_blocks; ++i) {
            print_block(fp, padded + i * 64);
        }
        free(padded);
    }

    printf("\nPadded message written to padded_message.hex\n");
    fclose(fp);
    fclose(input_fp);
    return 0;
}
