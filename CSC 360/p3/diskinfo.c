#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int main (int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <image.img>\n", argv[0]);
        exit(1);
    }

    /* Access test.img for readings. */
    FILE *image = fopen(argv[1], "rb");
    if (image == NULL) {
        perror("fopen");
        exit(1);
    }

    char magic[9] = {0};
    uint16_t block_size_lil;                                            // Number of bytes in each CSC360FS block   (lil)
    uint32_t fs_blocks_lil;                                             // Number of blocks in CSC360FS             (lil)
    uint32_t fat_block0_lil;                                            // Block number of first FAT block          (lil)
    uint32_t fat_blocks_lil;                                            // Number of blocks in FAT                  (lil)
    uint32_t root_block0_lil;                                           // Block number of first root dir block     (lil)
    uint32_t root_blocks_lil;                                           // Number of blocks in root dir             (lil)

    fread(magic, 1, 8, image);                                          // offset 0x00

    if (strcmp(magic, "CSC360FS") != 0) {
        exit(8);
    }

    fread(&block_size_lil, 1, 2, image);                                // offset 0x08
    fread(&fs_blocks_lil, 1, 4, image);                                 // offset 0x0A
    fread(&fat_block0_lil, 1, 4, image);                                // offset 0x0E
    fread(&fat_blocks_lil, 1, 4, image);                                // offset 0x12
    fread(&root_block0_lil, 1, 4, image);                               // offset 0x16
    fread(&root_blocks_lil, 1, 4, image);                               // offset 0x1A

    uint16_t block_size_big = htons(block_size_lil);                    // Number of bytes in each CSC360FS block   (big)
    uint32_t fs_blocks_big = htonl(fs_blocks_lil);                      // Number of blocks in CSC360FS             (big)
    uint32_t fat_block0_big = htonl(fat_block0_lil);                    // Block number of first FAT block          (big)
    uint32_t fat_blocks_big = htonl(fat_blocks_lil);                    // Number of blocks in FAT                  (big) 
    uint32_t root_block0_big = htonl(root_block0_lil);                  // Block number of first root dir block     (big)
    uint32_t root_blocks_big = htonl(root_blocks_lil);                  // Number of blocks in root dir             (big)

    printf("Super block information:\n");
    printf("Block size: %u\n", block_size_big);
    printf("Block count: %u\n", fs_blocks_big);
    printf("FAT starts: %u\n", fat_block0_big);
    printf("FAT blocks: %u\n", fat_blocks_big);
    printf("Root directory start: %u\n", root_block0_big);
    printf("Root directory blocks: %u\n", root_blocks_big);

    fseek(image, (fat_block0_big * block_size_big), SEEK_SET);          // Seek image cursor to FAT byte 0

    uint32_t fat_entry_lil;

    uint16_t avl_block_cnt = 0;                                         // Number of available blocks
    uint16_t res_block_cnt = 0;                                         // Number of reserved blocks
    uint16_t used_block_cnt = 0;                                        // Number of allocated blocks

    while (ftell(image) < root_block0_big * block_size_big) {           // While image cursor at relevant byte
        fread(&fat_entry_lil, 1, 4, image);
        switch (htonl(fat_entry_lil)) {
            case 0x00000000: avl_block_cnt++; break;
            case 0x00000001: res_block_cnt++; break;
            default: used_block_cnt++; break;
        }
    }

    printf("\nFAT information:\n");
    printf("Free Blocks: %u\n", avl_block_cnt);
    printf("Reserved Blocks: %u\n", res_block_cnt);
    printf("Allocated Blocks: %u\n", used_block_cnt);

    fclose(image);
    return 0;
}
