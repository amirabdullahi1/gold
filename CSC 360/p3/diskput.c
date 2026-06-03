#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

int eput (char *put_pth, char **pth_tok, uint32_t *blk_arr, uint32_t *blx_avl, FILE *cpy, FILE *img, uint32_t blk_szB, uint32_t fat_eB0, uint32_t avl_eB0) {
    char e[65] = {0};                                                   // Current entry string
    uint32_t e_block_lil, e_block_big;                                  // Block number of entry                (big/lil)
    uint32_t f_bytes_lil, f_bytes_big;                                  // Number of bytes in file              (big/lil)
    uint32_t f_blocks_lil, f_blocks_big;                                // Number of blocks in file             (big/lil)

    while (put_pth != NULL) {
        /* Read entry's next block for dir/file data */
        e_block_big = blk_arr[--(*blx_avl)];
        e_block_lil = ntohl(e_block_big);
        memcpy(e + 1, &e_block_lil, 4);

        memcpy(e + 27, put_pth, 31);

        put_pth = strtok_r(NULL, "/", pth_tok);                         // Parse next segment (current path string = next segment)

        if (put_pth == NULL) {
            e[0] = 0b00000011; // F status

            /* Measure bytes and block size of file */
            fseek(cpy, 0, SEEK_END); 
            f_bytes_big = ftell(cpy);
            fseek(cpy, 0, SEEK_SET);
            f_blocks_big = (f_bytes_big + blk_szB - 1) / blk_szB;
            /* Bytes and block size of file measured */
        } else {
            e[0] = 0b00000101; // D status

            /* Measure bytes and block size of dir */
            f_bytes_big = 0;
            f_blocks_big = 0;
            /* Bytes and block size of dir measured */

            fseek(img, (fat_eB0 + e_block_big * 4), SEEK_SET);
            fprintf(img, "%c%c%c%c", 0xFF, 0xFF, 0xFF, 0xFF);
            fflush(img); // Last FAT entry of dir indicated
        }

        f_blocks_lil = ntohl(f_blocks_big);                             // Read entry block size  
        memcpy(e + 5, &f_blocks_lil, 4);
        f_bytes_lil = ntohl(f_bytes_big);                               // Read entry byte size                     
        memcpy(e + 9, &f_bytes_lil, 4);

        time_t t = time(NULL); 
        struct tm *crt_mod_time = localtime(&t);
        uint16_t crt_mod_year = ntohs(crt_mod_time->tm_year + 1900);

        /* Read creation time */
        memcpy(e + 13, &crt_mod_year, 2);
        e[15] = crt_mod_time->tm_mon + 1;
        e[16] = crt_mod_time->tm_mday;
        e[17] = crt_mod_time->tm_hour;
        e[18] = crt_mod_time->tm_min;
        e[19] = crt_mod_time->tm_sec;
        /* Creation time read */

        /* Read modded time */
        memcpy(e + 20, &crt_mod_year, 2);
        e[22] = crt_mod_time->tm_mon + 1;
        e[23] = crt_mod_time->tm_mday;
        e[24] = crt_mod_time->tm_hour;
        e[25] = crt_mod_time->tm_min;
        e[26] = crt_mod_time->tm_sec;
        /* Modded time read */

        memset(e + 58, 0xFF, 6);

        fseek(img, avl_eB0, SEEK_SET);
        fwrite(e, 1, 64, img);
        fflush(img); // Next dir entry of file appended

        avl_eB0 = e_block_big * blk_szB;

        fseek(img, (e_block_big * blk_szB), SEEK_SET);                  // Seek image cursor to next relevant data byte
    }

    return f_bytes_big;
}

int cput (uint32_t *blk_arr, uint32_t *blx_avl, FILE *cpy, FILE *img, uint32_t blk_szB, uint32_t fat_eB0, uint32_t cpy_szB) {
    uint32_t wrt_szB;
    uint32_t f_block_lil, f_block_big;                                  // Block number of current file block   (big/lil)
    char *wrt_buf = calloc(blk_szB, 1);                                 // Intermediary between copy and image 

    while (cpy_szB != 0) {
        /* Next copy block of file awaits - Next image block of file awaits */
        fseek(img, (fat_eB0 + f_block_big * 4), SEEK_SET);
        f_block_big = blk_arr[(*blx_avl)--];
        f_block_lil = ntohl(f_block_big);
        fwrite(&f_block_lil, 1, 4, img);
        fflush(img); // Next FAT entry of file indicated

        /* Read next copy block of file - Write next img block of file */
        wrt_szB = cpy_szB > blk_szB ? blk_szB : cpy_szB;
        fseek(img, (f_block_big * blk_szB), SEEK_SET);
        fread(wrt_buf, 1, wrt_szB, cpy);
        fwrite(wrt_buf, 1, wrt_szB, img);
        fflush(img);
        cpy_szB -= wrt_szB;
        /* Next copy block of file read - Next image block of file wrote */
    }

    /* Last copy block of file read - Last image block of file wrote */
    fseek(img, (fat_eB0 + f_block_big * 4), SEEK_SET);
    fprintf(img, "%c%c%c%c", 0xFF, 0xFF, 0xFF, 0xFF);
    fflush(img); // Last FAT entry of file indicated

    free(wrt_buf); 
    return 0;
}

int main (int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <image.img> <filename> <path>\n", argv[0]);
        exit(1);
    }

    /* Access test.img to read+write */
    FILE *image = fopen(argv[1], "r+b");
    if (image == NULL) {
        perror("fopen");
        exit(1);
    }

    /* Access copy.? to read */
    FILE *copy = fopen(argv[2], "rb");
    if (copy == NULL) {
        printf("Source file %s not found.\n", argv[2]);
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

    /* Find available blocks */
    uint32_t fat_entry_lil;
    uint32_t avl_block_arr[6400] = {0};
    uint32_t cur_block_cnt = 0, avl_block_cnt = 0;
    
    fseek(image, fat_byte0_big, SEEK_SET);  // Seek image cursor to FAT byte 0
    
    while (ftell(image) < root_byte0_big) { // While image cursor at relevant byte
        fread(&fat_entry_lil, 1, 4, image);
        if (htonl(fat_entry_lil) == 0x00000000) {
            avl_block_arr[avl_block_cnt++] = cur_block_cnt;
        }   
        cur_block_cnt++;
    }
    /* Available blocks found */

    char status = '\0';                                                 // Current entry status (F/D) or \0 if unused
    char entry[65] = {0};                                               // Current entry string
    char file_name[32] = {0};                                           // Current entry file name string
    
    char *pth_ptr;
    char *cur_pth = strtok_r(strdup(argv[3]), "/", &pth_ptr);           // Current path string (skips leading '/')    

    uint32_t file_bytes_big;                                            // Number of bytes in file                  (big)
    uint32_t file_blocks_big;                                           // Number of blocks in file                 (big)
    
    /* Find max block size of copy including dirs */
    file_blocks_big = 0;

    char *dir_ptr; 
    char *cur_dir = strtok_r(strdup(argv[3]), "/", &dir_ptr);           // Current dir string (skips leading '/')    
    while (cur_dir != NULL) {
        file_blocks_big++;
        cur_dir = strtok_r(NULL, "/", &dir_ptr);  
    }

    if (file_blocks_big == 0) {
        exit(1);        // File escapes root dir 
    }
    file_blocks_big--; // Subtract for root dir

    fseek(copy, 0, SEEK_END);                                            
    file_bytes_big = ftell(copy);                                       
    fseek(copy, 0, SEEK_SET);
    file_blocks_big += (file_bytes_big + block_size_big - 1) / block_size_big;
    /* Max block size of copy including dirs found */
    
    uint32_t cur_entry_byte0_big = UINT32_MAX;                          // Byte number of current dir entry         (big)
    uint32_t avl_entry_byte0_big = UINT32_MAX;                          // Byte number of available dir entry       (big)
    uint32_t dir_entry_block_big = UINT32_MAX;                          // Block number of directory entry          (big)

    uint32_t entry_block_lil, entry_block_big;                          // Block number of entry                (big/lil)

    fseek(image, root_byte0_big, SEEK_SET);                             // Seek image cursor to root dir byte 0

    while (ftell(image) < next_byte0_big) {                             // While image cursor at relevant byte
        cur_entry_byte0_big = ftell(image);                             // Byte number of potentially available first entry byte
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
            default: status = '\0'; avl_entry_byte0_big = cur_entry_byte0_big; break;
        }
        
        memcpy(file_name, entry + 27, 31);

        if (status == 'D' && strcmp((cur_pth ? cur_pth : "/"), file_name) == 0) { // Subdir found
            memcpy(&entry_block_lil, entry + 1, 4);                     // Read subdir entry block 
            entry_block_big = htonl(entry_block_lil);
            dir_entry_block_big = entry_block_big;
            next_byte0_big = (entry_block_big + 1) * block_size_big;    // Set byte number of next data block
            fseek(image, (entry_block_big * block_size_big), SEEK_SET); // Seek image cursor to first relevant data byte
            
            cur_pth = strtok_r(NULL, "/", &pth_ptr);                    // Parse next segment (current path string = next segment)

            if (cur_pth == NULL) {                                      // Current path string last segment (file names can't match)
                exit(2);
            }

            avl_entry_byte0_big = 0;                                    // Byte number of available subdir entry    (big)
            file_blocks_big--;                                          // One less directory block needed

            continue;
        }

        if (status == 'F' && strcmp((cur_pth ? cur_pth : "/"), file_name) == 0) { // File found (file names can't match)
            exit(3);
        }
    }

    uint32_t dir_entry_block_lil;                                       // Block number of directory entry          (lil)

    /* Full dir entries - Extend dir to next available block */ 
    if (avl_entry_byte0_big == 0) {
        fseek(image, (fat_byte0_big + dir_entry_block_big * 4), SEEK_SET);
        dir_entry_block_big = avl_block_arr[--avl_block_cnt];
        dir_entry_block_lil = ntohl(dir_entry_block_big);
        fwrite(&dir_entry_block_lil, 1, 4, image);
        fflush(image); // Next FAT entry of dir indicated
        avl_entry_byte0_big = dir_entry_block_big * block_size_big;
        fseek(image, (fat_byte0_big + dir_entry_block_big * 4), SEEK_SET);
        fprintf(image, "%c%c%c%c", 0xFF, 0xFF, 0xFF, 0xFF);
        fflush(image); // Last FAT entry of dir indicated
    } 
    /* Open dir entries - Extended dir to next available block */ 

    /* Root filled too much to put copy */
    if (avl_entry_byte0_big == UINT32_MAX) {
        printf("Root Full!\n"); exit(4);
    }

    /* Disk filled too much to put copy */
    if (file_blocks_big > avl_block_cnt) {
        printf("Disk Full!\n"); exit(5);
    }

    file_bytes_big = eput(cur_pth, &pth_ptr, avl_block_arr, &avl_block_cnt, copy, image, block_size_big, fat_byte0_big, avl_entry_byte0_big);
    cput(avl_block_arr, &avl_block_cnt, copy, image, block_size_big, fat_byte0_big, file_bytes_big);

    fclose(image); fclose(copy);
    return 0;
}
