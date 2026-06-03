#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int cget (FILE *cpy, FILE *img, uint32_t blk_szB, uint32_t fat_eB0, uint32_t blk_num, uint32_t cpy_szB) {
    uint32_t wrt_szB;
    uint32_t f_block_lil, f_block_big;                                  // Block number of current file block   (big/lil)
    char *wrt_buf = calloc(blk_szB, 1);                                 // Intermediary between copy and image 

    f_block_big = blk_num;

    while (cpy_szB != 0) {
        /* Read next image block of file - Write next copy block of file */
        fseek(img, (f_block_big * blk_szB), SEEK_SET);
        wrt_szB = cpy_szB > blk_szB ? blk_szB : cpy_szB;
        fread(wrt_buf, 1, wrt_szB, img);
        fwrite(wrt_buf, 1, wrt_szB, cpy);
        cpy_szB -= wrt_szB;
        // printf("cpy_szb %u\n", cpy_szB);
        fseek(img, (fat_eB0 + f_block_big * 4), SEEK_SET);
        fread(&f_block_lil, 1, 4, img);
        f_block_big = htonl(f_block_lil);
        /* Next image block of file read - Next copy block of file wrote */
    }

    if (f_block_big != 0xFFFFFFFF) {
        exit(1);
    }

    free(wrt_buf);
    return 0;
}

int miss (char *argv2) {
    argv2 += strspn(argv2, "/");
    char *req_file = argv2;
    char *req_path = argv2;
    char *slash = strrchr(argv2, '/');

    if (slash != NULL) {
        *slash = '\0';
        req_file = slash + 1;
    } else {
        req_path = "";
    }

    printf("Requested file %s not found in /%s.\n", req_file, req_path);
    exit(1);
}

int main (int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <image.img> <path> <filename>\n", argv[0]);
        exit(1);
    }

    /* Access image to read and copy to write */
    FILE *image = NULL, *copy = NULL;
    image = fopen(argv[1], "rb");
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
    char *cur_pth = strtok(strdup(argv[2]), "/");                       // Current path string (skips leading '/')    

    char *data = calloc(block_size_big, 1);                             // Intermediary between image and copy

    uint32_t file_bytes_lil, file_bytes_big;                            // Number of bytes in file              (big/lil)
    uint32_t entry_block_lil, entry_block_big;                          // Block number of entry                (big/lil)

    while (ftell(image) < next_byte0_big) {                             // While image cursor at relevant byte
        fread(entry, 1, 64, image);

        if (ftell(image) > data_byte0_big && ftell(image) + 64 > next_byte0_big) {  // Image cursor at last relevant data byte 
            fseek(image, (fat_byte0_big + entry_block_big * 4), SEEK_SET);          // Seek image cursor to entry block FAT entry 
            fread(&entry_block_lil, 1, 4, image);                                   // Read entry block FAT entry
            entry_block_big = htonl(entry_block_lil);

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
            default: continue;
        }

        memcpy(file_name, entry + 27, 31);

        if (status != '\0' && strcmp((cur_pth ? cur_pth : "/"), file_name) == 0) { // Subdir or file found
            memcpy(&entry_block_lil, entry + 1, 4);                     // Read entry block 
            memcpy(&file_bytes_lil, entry + 9, 4);                      // Read entry file size
            file_bytes_big = htonl(file_bytes_lil);
            entry_block_big = htonl(entry_block_lil);                   // Block number of entry

            next_byte0_big = (entry_block_big + 1) * block_size_big;    // Set byte number of next data block
            fseek(image, (entry_block_big * block_size_big), SEEK_SET); // Seek image cursor to first relevant data byte

            cur_pth = strtok(NULL, "/");                                // Parse next segment (current path string = next segment)

            if (cur_pth == NULL) {                                      // Current path string last segment 
                copy = fopen(argv[3], "wb");                            
                if (copy == NULL) {
                    perror("fopen");
                    exit(1);
                }
                cget(copy, image, block_size_big, fat_byte0_big, entry_block_big, file_bytes_big);
                break;
            }
        }
    }

    free(data); fclose(image);

    if (copy == NULL) {
        miss(argv[2]);
    } else {
        fclose(copy);
    }

    return 0;
}
