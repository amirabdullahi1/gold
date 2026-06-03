#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int main (int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <image.img />\n", argv[0]);
        exit(1);
    }

    /* Access test.img to read */
    FILE *image = fopen(argv[1], "rb");
    if (image == NULL) {
        perror("fopen");
        exit(1);
    }

    char magic[9] = {0};
    uint16_t block_size_lil;                                            // Number of bytes in each CSC360FS block   (lil)
    uint32_t fat_block0_lil;                                            // Block number of first FAT block          (lil)
    uint32_t root_block0_lil;                                           // Block number of first root dir block     (lil)
    uint32_t root_blocks_lil;                                           // Number of blocks in root dir             (lil)

    fread(magic, 1, 8, image);                                          // offset 0x00

    if (strcmp(magic, "CSC360FS") != 0) {
        exit(8);
    }

    fread(&block_size_lil, 1, 2, image);                                // offset 0x08
    fseek(image, 14, SEEK_SET);
    fread(&fat_block0_lil, 1, 4, image);                                // offset 0x0E
    fseek(image, 22, SEEK_SET);
    fread(&root_block0_lil, 1, 4, image);                               // offset 0x16
    fread(&root_blocks_lil, 1, 4, image);                               // offset 0x1A

    uint16_t block_size_big = htons(block_size_lil);                    // Number of bytes in each CSC360FS block   (big)
    uint32_t fat_byte0_big = htonl(fat_block0_lil) * block_size_big;    // Byte number of first FAT block           (big)
    uint32_t root_byte0_big = htonl(root_block0_lil) * block_size_big;  // Byte number of first root block          (big)
    uint32_t root_bytes_big = htonl(root_blocks_lil) * block_size_big;  // Number of bytes in root block            (big)

    uint32_t data_byte0_big = root_byte0_big + root_bytes_big;          // Byte number of first data block          (big)
    uint32_t next_byte0_big = root_byte0_big + root_bytes_big;          // Byte number of next data block           (big)

    fseek(image, root_byte0_big, SEEK_SET);                             // Seek image cursor to root byte 0

    char status = '\0';                                                 // Current entry status (F/D) or \0 if unused
    char entry[65] = {0};                                               // Current entry string
    char file_name[32] = {0};                                           // Current entry file name string
    char *cur_dir = strtok(argv[2], "/");                               // Current dir string (skips leading '/')

    uint32_t file_bytes_lil, file_bytes_big;                            // Number of bytes in file              (big/lil)
    uint32_t entry_block_lil, entry_block_big;                          // Block number of entry                (big/lil)

    uint16_t crt_Y_lil, crt_Y_big;                                      // Creation year                        (big/lil) 
    uint8_t crt_M, crt_D, crt_h, crt_m, crt_s;                          // Creation month, day, hour, minute and second

    while (ftell(image) < next_byte0_big) {                             // While image cursor at relevant byte
        fread(entry, 1, 64, image);

        if (ftell(image) > data_byte0_big && ftell(image) + 64 > next_byte0_big) {  // Image cursor at last relevant data byte
            fseek(image, (fat_byte0_big + entry_block_big * 4), SEEK_SET);          // Seek image cursor to entry block FAT entry 
            fread(&entry_block_lil, 1, 4, image);                                   // Read entry block FAT entry
            entry_block_big = htonl(entry_block_lil);

            // printf("entry_block_big %08x\n", entry_block_big);
            if (entry_block_big != 0xffffffff) {
                next_byte0_big = (entry_block_big + 1) * block_size_big;            // Set byte number of next data block
                fseek(image, (entry_block_big * block_size_big), SEEK_SET);         // Seek image cursor to first relevant data byte
            } else {
                fseek(image, next_byte0_big, SEEK_SET);                             // Seek image cursor to last relevant data byte
            }
        }

        switch (entry[0] & 0b00000111) {
            case 0b00000011: status = 'F'; break;
            case 0b00000101: status = 'D'; break;
            default: status = '\0'; continue;
        }

        /* Read creation date */
        memcpy(&file_bytes_lil, entry + 9, 4);
        file_bytes_big = htonl(file_bytes_lil);

        memcpy(&crt_Y_lil, entry + 13, 2);
        crt_Y_big = htons(crt_Y_lil);

        memcpy(&crt_M, entry + 15, 1);
        memcpy(&crt_D, entry + 16, 1);
        memcpy(&crt_h, entry + 17, 1);
        memcpy(&crt_m, entry + 18, 1);
        memcpy(&crt_s, entry + 19, 1);
        /* Creation date read */

        memcpy(file_name, entry + 27, 31);

        if (status == 'D' && strcmp((cur_dir ? cur_dir : ""), file_name) == 0) { // Subdir found
            memcpy(&entry_block_lil, entry + 1, 4);                     // Read subdir entry block 
            entry_block_big = htonl(entry_block_lil);

            next_byte0_big = (entry_block_big + 1) * block_size_big;    // Set byte number of next data block
            fseek(image, (entry_block_big * block_size_big), SEEK_SET); // Seek image cursor to first relevant data byte

            cur_dir = strtok(NULL, "/");                                // Parse next segment (current dir string = next segment)

            if (cur_dir == NULL) {                                      // Current dir string last segment
                argv[2] = "/";                                          // Indicate max dir depth reached
            }

            continue;
        }

        if (strcmp(argv[2], "/") == 0) {    // Print current dir entry file status, size, file name and creation date
            printf("%c %10u %30s %u/%u/%u %02u:%02u:%02u\n", status, file_bytes_big, file_name, crt_Y_big, crt_M, crt_D, crt_h, crt_m, crt_s);
        }
    }

    if (strcmp(argv[2], "/") != 0) {
        /* printf("Dir DNE!\n"); */ exit(2);
    }

    fclose(image);
    return 0;
}
