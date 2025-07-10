#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define SHA256_BASE 0x1000
#define IOWR(base, offset, data) printf("IOWR(0x%x, %d) = 0x%08x\n", base, offset, data)

uint64_t to_big_endian_64(uint64_t x) {
    uint64_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result |= ((x >> (8 * i)) & 0xFF) << (56 - 8 * i);
    }
    return result;
}

int main() {
    char input[1024];
    printf("Nhập message: ");
    fgets(input, sizeof(input), stdin);

    size_t input_len = strlen(input);
    if (input[input_len - 1] == '\n') input[--input_len] = '\0';

    uint64_t bit_len = input_len * 8;

    size_t total_len = input_len + 1 + 8;
    size_t padding_len = (64 - (total_len % 64)) % 64;
    total_len += padding_len;

    uint8_t* padded = calloc(total_len, 1);
    memcpy(padded, input, input_len);
    padded[input_len] = 0x80;

    uint64_t big_endian_len = to_big_endian_64(bit_len);
    memcpy(padded + total_len - 8, &big_endian_len, 8);

    size_t num_blocks = total_len / 64;
    for (size_t block = 0; block < num_blocks; ++block) {
        printf("\nBlock %zu (512-bit):\n", block + 1);
        for (int i = 0; i < 64; i += 4) {
            uint32_t word = (padded[block * 64 + i] << 24) |
                            (padded[block * 64 + i + 1] << 16) |
                            (padded[block * 64 + i + 2] << 8) |
                            (padded[block * 64 + i + 3]);
            IOWR(SHA256_BASE, i / 4, word); // offset theo từng từ 32-bit
        }
    }

    free(padded);
    return 0;
}
