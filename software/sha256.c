#include "sha256.h"
#include <string.h>
#include <stdio.h>
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

static const uint32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static void sha256_transform(uint32_t state[8], const uint8_t data[]) {
    uint32_t w[64], a, b, c, d, e, f, g, h, t1, t2;

    // Prepare message schedule
    for (int i = 0; i < 16; ++i)
        w[i] = (data[i * 4] << 24) | (data[i * 4 + 1] << 16) | 
               (data[i * 4 + 2] << 8) | (data[i * 4 + 3]);
    for (int i = 16; i < 64; ++i)
        w[i] = SIG1(w[i - 2]) + w[i - 7] + SIG0(w[i - 15]) + w[i - 16];

    // Initialize working variables
    a = state[0]; b = state[1]; c = state[2]; d = state[3];
    e = state[4]; f = state[5]; g = state[6]; h = state[7];

    // Compression function main loop
    for (int i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e,f,g) + k[i] + w[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    // Add the compressed chunk to the current hash value
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

void sha256(const uint8_t *data, size_t len, uint8_t hash[32]) {
    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85,
        0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c,
        0x1f83d9ab, 0x5be0cd19
    };

    uint8_t block[64];
    size_t full_blocks = len / 64;
    size_t rem = len % 64;

    // Process full blocks
    for (size_t i = 0; i < full_blocks; ++i)
        sha256_transform(state, data + i * 64);

    // Prepare final block with padding
    size_t total_len = len * 8;
    memset(block, 0, 64);
    memcpy(block, data + full_blocks * 64, rem);
    block[rem] = 0x80;

    if (rem >= 56) {
        sha256_transform(state, block);
        memset(block, 0, 64);
    }

    block[63] = total_len & 0xFF;
    block[62] = (total_len >> 8) & 0xFF;
    block[61] = (total_len >> 16) & 0xFF;
    block[60] = (total_len >> 24) & 0xFF;
    block[59] = (total_len >> 32) & 0xFF;
    block[58] = (total_len >> 40) & 0xFF;
    block[57] = (total_len >> 48) & 0xFF;
    block[56] = (total_len >> 56) & 0xFF;

    sha256_transform(state, block);

    // Produce final hash
    for (int i = 0; i < 8; ++i) {
        hash[i*4 + 0] = (state[i] >> 24) & 0xff;
        hash[i*4 + 1] = (state[i] >> 16) & 0xff;
        hash[i*4 + 2] = (state[i] >> 8) & 0xff;
        hash[i*4 + 3] = (state[i]) & 0xff;
    }
}
void hexstr_to_bytes(const char *hexstr, uint8_t *bytes, size_t *len) {
    size_t i;
    size_t hexlen = strlen(hexstr);
    if (hexstr[hexlen - 1] == '\n') hexlen--;  // Bỏ ký tự newline

    *len = hexlen / 2;
    for (i = 0; i < *len; ++i) {
        sscanf(hexstr + 2*i, "%2hhx", &bytes[i]);
    }
}
void write_digest(FILE *fp, uint8_t *hash) {
    for (int i = 0; i < 32; i++) {
        fprintf(fp, "%02x", hash[i]);
    }
    fprintf(fp,"\n");
}
int main() {
    FILE *fp = fopen("message.txt", "r");
    if (fp == NULL) {
        perror("Cannot open file");
        return 1;
    }
    FILE *output_fp = fopen("digest.txt", "w");
    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        // Xóa ký tự newline nếu có
        line[strcspn(line, "\r\n")] = 0;

        printf("original string: %s\n", line);

        // Tính toán SHA256 trực tiếp từ string ASCII
        uint8_t hash[32];
        sha256((const uint8_t *)line, strlen(line), hash);

        // In kết quả
        write_digest(output_fp, hash);
    }

    fclose(fp);
    return 0;
}