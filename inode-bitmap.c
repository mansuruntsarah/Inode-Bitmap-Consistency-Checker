#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 4096
#define TOTAL_BLOCKS 64

#define SUPERBLOCK_BLOCK 0
#define INODE_BITMAP_BLOCK 1
#define DATA_BITMAP_BLOCK 2
#define INODE_TABLE_START_BLOCK 3
#define INODE_TABLE_BLOCKS 5
#define FIRST_DATA_BLOCK 8
#define INODE_COUNT 80

#define INODE_SIZE 256
#define INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)
#define TOTAL_INODES (INODE_TABLE_BLOCKS * INODES_PER_BLOCK)

#define VSFS_MAGIC 0xD34D

// Superblock Structure
typedef struct {
    uint16_t magic;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t inode_bitmap_block;
    uint32_t data_bitmap_block;
    uint32_t inode_table_start;
    uint32_t data_block_start;
    uint32_t inode_size;
    uint32_t inode_count;
    char reserved[4058];
} __attribute__((packed)) Superblock;

// Inode Structure
typedef struct {
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint32_t links_count;
    uint32_t blocks_count;
    uint32_t direct_block;
    uint32_t indirect1;
    uint32_t indirect2;
    uint32_t indirect3;
    uint8_t reserved[156];
} inode_t;

// Global variables
Superblock sb;
inode_t inodes[TOTAL_INODES];
uint8_t inode_bitmap[BLOCK_SIZE];
uint8_t data_bitmap[BLOCK_SIZE];
int block_usage_count[TOTAL_BLOCKS] = {0};
FILE *fp;

// Utility function to check if a bitmap bit is set
int is_bitmap_set(uint8_t *bitmap, int index) {
    return (bitmap[index / 8] >> (index % 8)) & 1;
}

// Utility function to set or clear a bitmap bit
void set_bitmap(uint8_t *bitmap, int index, int value) {
    if (value)
        bitmap[index / 8] |= (1 << (index % 8));
    else
        bitmap[index / 8] &= ~(1 << (index % 8));
}

// Read and write functions for Superblock, Inodes, and Bitmaps
void read_superblock() {
    fseek(fp, SUPERBLOCK_BLOCK * BLOCK_SIZE, SEEK_SET);
    fread(&sb, sizeof(Superblock), 1, fp);
}

void write_superblock() {
    fseek(fp, SUPERBLOCK_BLOCK * BLOCK_SIZE, SEEK_SET);
    fwrite(&sb, sizeof(Superblock), 1, fp);
}

void read_bitmaps() {
    fseek(fp, INODE_BITMAP_BLOCK * BLOCK_SIZE, SEEK_SET);
    fread(inode_bitmap, 1, BLOCK_SIZE, fp);
    fseek(fp, DATA_BITMAP_BLOCK * BLOCK_SIZE, SEEK_SET);
    fread(data_bitmap, 1, BLOCK_SIZE, fp);
}

void write_bitmaps() {
    fseek(fp, INODE_BITMAP_BLOCK * BLOCK_SIZE, SEEK_SET);
    fwrite(inode_bitmap, 1, BLOCK_SIZE, fp);
    fseek(fp, DATA_BITMAP_BLOCK * BLOCK_SIZE, SEEK_SET);
    fwrite(data_bitmap, 1, BLOCK_SIZE, fp);
}

void read_inodes() {
    fseek(fp, INODE_TABLE_START_BLOCK * BLOCK_SIZE, SEEK_SET);
    fread(inodes, INODE_SIZE, TOTAL_INODES, fp);
}

void write_inodes() {
    fseek(fp, INODE_TABLE_START_BLOCK * BLOCK_SIZE, SEEK_SET);
    fwrite(inodes, INODE_SIZE, TOTAL_INODES, fp);
}

// Superblock Validator: Ensures the integrity of the superblock
void validate_superblock() {
    if (sb.magic != VSFS_MAGIC) {
        printf("Invalid magic number: 0x%04X. Fixing...\n", sb.magic);
        sb.magic = VSFS_MAGIC;
    }
    else{
    	printf("Valid magic number: 0x%04X.\n", sb.magic);
    }
    if (sb.block_size != BLOCK_SIZE) {
        printf("Invalid block size: %u. Fixing...\n", sb.block_size);
        sb.block_size = BLOCK_SIZE;
    }
    else{
    	printf("Valid block size: %u.\n", sb.block_size);
    }
    if (sb.total_blocks != TOTAL_BLOCKS) {
        printf("Invalid total blocks: %u. Fixing...\n", sb.total_blocks);
        sb.total_blocks = TOTAL_BLOCKS;
    }
    else{
    	printf("Valid total blocks: %u. \n", sb.total_blocks);
    }
    if (sb.inode_bitmap_block != INODE_BITMAP_BLOCK) {
        printf("Invalid inode bitmap block: %u. Fixing...\n", sb.inode_bitmap_block);
        sb.inode_bitmap_block = INODE_BITMAP_BLOCK;
    }
    else{
    	printf("Valid inode bitmap block: %u. \n", sb.inode_bitmap_block);
    }
    if (sb.data_bitmap_block != DATA_BITMAP_BLOCK) {
        printf("Invalid data bitmap block: %u. Fixing...\n", sb.data_bitmap_block);
        sb.data_bitmap_block = DATA_BITMAP_BLOCK;
    }
    else{
    	printf("Valid data bitmap block: %u.\n", sb.data_bitmap_block);
    }
    if (sb.inode_table_start != INODE_TABLE_START_BLOCK) {
        printf("Invalid inode table start: %u. Fixing...\n", sb.inode_table_start);
        sb.inode_table_start = INODE_TABLE_START_BLOCK;
    }
    else{
    	printf("Valid inode table start: %u.\n", sb.inode_table_start);
    }
    if (sb.data_block_start != FIRST_DATA_BLOCK) {
        printf("Invalid data block start: %u. Fixing...\n", sb.data_block_start);
        sb.data_block_start = FIRST_DATA_BLOCK;
    }
    else{
    	printf("Valid data block start: %u.\n", sb.data_block_start);
    }
    if (sb.inode_size != INODE_SIZE) {
        printf("Invalid inode size: %u. Fixing...\n", sb.inode_size);
        sb.inode_size = INODE_SIZE;
    }
    else{
    	printf("Valid inode size: %u.\n", sb.inode_size);
    }
    if (sb.inode_count > TOTAL_INODES) {
        printf("Inode count too high: %u. Fixing...\n", sb.inode_count);
        sb.inode_count = TOTAL_INODES;
    }
    else{
    	printf("Inode count: %u.\n", sb.inode_count);
    }
    write_superblock();
}

// Check and fix Inode Bitmap consistency
void check_and_fix_inode_bitmap(FILE *fp) {
    unsigned char inode_bitmap[BLOCK_SIZE];
    inode_t inode;
    int errors = 0;
    fseek(fp, BLOCK_SIZE * INODE_BITMAP_BLOCK, SEEK_SET);
    fread(inode_bitmap, 1, BLOCK_SIZE, fp);

    for (int i = 0; i < INODE_COUNT; i++) {
        long inode_offset = BLOCK_SIZE * INODE_TABLE_START_BLOCK + i * INODE_SIZE;
        fseek(fp, inode_offset, SEEK_SET);
        fread(&inode, sizeof(inode_t), 1, fp);
        int byte_index = i / 8;
        int bit_index = i % 8;
        int bit_set = (inode_bitmap[byte_index] >> bit_index) & 1;
        int valid_inode = (inode.links_count > 0) && (inode.dtime == 0);
        if (bit_set && !valid_inode) {
            printf("Inode %d changed to 0\n", i);
            inode_bitmap[byte_index] &= ~(1 << bit_index);
        }
        if (!bit_set && valid_inode) {
            printf("Inode %d set to 1\n", i);
            inode_bitmap[byte_index] |= (1 << bit_index);
        }
    }
    fseek(fp, BLOCK_SIZE * INODE_BITMAP_BLOCK, SEEK_SET);
    fwrite(inode_bitmap, 1, BLOCK_SIZE, fp);
}


// Check and fix Data Bitmap consistency
void fix_data_bitmap() {
    int inode_index = 0;
for (int i = 0; i < 56; i++) {
    if (inodes[i].direct_block < FIRST_DATA_BLOCK || inodes[i].direct_block >= TOTAL_BLOCKS) {
        inodes[i].links_count = 0;
        inodes[i].dtime = 1; // mark deleted
    }
}
    for (int i = 0; i < TOTAL_BLOCKS - FIRST_DATA_BLOCK; i++) {
        int block_num = i + FIRST_DATA_BLOCK;
        int found = 0;

        // Check if any valid inode references this block
        for (int j = 0; j < TOTAL_INODES; j++) {
            if (inodes[j].links_count > 0 && inodes[j].dtime == 0 &&
                inodes[j].direct_block == block_num) {
                found = 1;

                // If block is referenced but not marked in bitmap, fix it
                if (is_bitmap_set(data_bitmap, i)) {
                    //printf("Block %d used by inode %d \n", block_num, j);
                    set_bitmap(data_bitmap, i, 1);
                }
                break;
            }
        }

        // If not referenced by any inode
        if (!found) {
            // If it's marked used but not referenced, assign it
            if (is_bitmap_set(data_bitmap, i)) {
                // Assign it to a free/invalid inode
                while (inode_index < TOTAL_INODES &&
                       (inodes[inode_index].links_count > 0 && inodes[inode_index].dtime == 0)) {
                    inode_index++;
                }

                if (inode_index < TOTAL_INODES) {
                    //printf("Block %d is marked used but unreferenced. Assigning to inode %d\n", block_num, inode_index);
                    inodes[inode_index].links_count = 1;
                    inodes[inode_index].dtime = 0;
                    inodes[inode_index].direct_block = block_num;
                    set_bitmap(inode_bitmap, inode_index, 1);
                    inode_index++;
                } else {
                    //printf("Warning: No free inode to assign block %d\n", block_num);
                }
            } else {
                // Block is unused and unreferenced — mark it as used and assign
                while (inode_index < TOTAL_INODES &&
                       (inodes[inode_index].links_count > 0 && inodes[inode_index].dtime == 0)) {
                    inode_index++;
                }

                if (inode_index < TOTAL_INODES) {
                    //printf("Block %d is unreferenced and not marked. Fixing \n ", block_num);
                    set_bitmap(data_bitmap, i, 1);
                    inodes[inode_index].links_count = 1;
                    inodes[inode_index].dtime = 0;
                   inodes[inode_index].direct_block = block_num;
                   set_bitmap(inode_bitmap, inode_index, 1);
                    inode_index++;
                } else {
                    //printf("Block %d is referenced to inode %d\n", block_num, inode_index);
                }
            }
        }
    }
}




// Check and fix Data Bitmap consistency
void check_data_bitmap() {
    int inode_index = 0;
for (int i = 0; i < 56; i++) {
    if (inodes[i].direct_block < FIRST_DATA_BLOCK || inodes[i].direct_block >= TOTAL_BLOCKS) {
        inodes[i].links_count = 0;
        inodes[i].dtime = 1; // mark deleted
    }
}
    for (int i = 0; i < TOTAL_BLOCKS - FIRST_DATA_BLOCK; i++) {
        int block_num = i + FIRST_DATA_BLOCK;
        int found = 0;

        // Check if any valid inode references this block
        for (int j = 0; j < TOTAL_INODES; j++) {
            if (inodes[j].links_count > 0 && inodes[j].dtime == 0 &&
                inodes[j].direct_block == block_num) {
                found = 1;

                // If block is referenced but not marked in bitmap, fix it
                if (is_bitmap_set(data_bitmap, i)) {
                    printf("Block %d used by inode %d \n", block_num, j);
                    //set_bitmap(data_bitmap, i, 1);
                }
                break;
            }
        }

        // If not referenced by any inode
        if (!found) {
            // If it's marked used but not referenced, assign it
            if (is_bitmap_set(data_bitmap, i)) {
                // Assign it to a free/invalid inode
                while (inode_index < TOTAL_INODES &&
                       (inodes[inode_index].links_count > 0 && inodes[inode_index].dtime == 0)) {
                    inode_index++;
                }

                if (inode_index < TOTAL_INODES) {
                    printf("Block %d is marked used but unreferenced. Assigning to inode %d\n", block_num, inode_index);
                    inodes[inode_index].links_count = 1;
                    inodes[inode_index].dtime = 0;
                    inodes[inode_index].direct_block = block_num;
                    //set_bitmap(inode_bitmap, inode_index, 1);
                    inode_index++;
                } else {
                    printf("Warning: No free inode to assign block %d\n", block_num);
                }
            } else {
                // Block is unused and unreferenced — mark it as used and assign
                while (inode_index < TOTAL_INODES &&
                       (inodes[inode_index].links_count > 0 && inodes[inode_index].dtime == 0)) {
                    inode_index++;
                }

                if (inode_index < TOTAL_INODES) {
                    printf("Block %d is unreferenced and not marked. Fixing \n ", block_num);
                    //set_bitmap(data_bitmap, i, 1);
                    //inodes[inode_index].links_count = 1;
                    //inodes[inode_index].dtime = 0;
                    //inodes[inode_index].direct_block = block_num;
                   // set_bitmap(inode_bitmap, inode_index, 1);
                    inode_index++;
                } else {
                    printf("Block %d is referenced to inode %d\n", block_num, inode_index);
                }
            }
        }
    }
}

// Check for duplicate block usage
void check_and_fix_duplicate_blocks() {
printf("Check for Duplicate Blocks\n");
	int f=0;
    for (int i = FIRST_DATA_BLOCK; i < TOTAL_BLOCKS; i++) {
        if (block_usage_count[i] > 1) {
        f=1;
            printf("Block %d is referenced by multiple inodes. Fixing...\n", i);
            // To fix, we could clear the usage of this block from the bitmap
            for (int j = 0; j < TOTAL_INODES; j++) {
                if (inodes[j].direct_block == i) {
                    inodes[j].direct_block = 0;  // Clear the block reference (example fix)
                    printf("Inode %d's block reference to %d cleared.\n", j, i);
                }
            }
            set_bitmap(data_bitmap, i - FIRST_DATA_BLOCK, 0); // Clear the block in bitmap
        }
    }
    if (f==0){
    	printf("Duplicate Blocks not found\n");
    }
    else{
    	printf("Duplicate Blocks found\n");
    }
    
}

// Check for bad blocks (out of range)
void check_and_fix_bad_blocks() {
	int f=0;
	printf("Bad Block Checker\n");
    for (int i = 0; i < TOTAL_INODES; i++) {
        int blk = inodes[i].direct_block;
        if (blk >= TOTAL_BLOCKS ) {
        	f=1;
            printf("Inode %d references an invalid block %d. Fixing...\n", i, blk);
            inodes[i].direct_block = 0; // Clear invalid block reference
        }
    }
    if (f==0){
    printf("No bad block found\n");
    }
    else{
    printf("Bad block found\n");
    }
    

}

// Main function to process the file system image
int main() {
    fp = fopen("vsfs.img", "rb+");
    if (!fp) {
        perror("Failed to open vsfs.img");
        return 1;
    }

    // Read and validate the superblock
    read_superblock();
    validate_superblock();

    // Read bitmaps and inodes
    read_bitmaps();
    read_inodes();

    // Perform consistency checks and fix errors
    check_data_bitmap();
    check_and_fix_inode_bitmap(fp);
    fix_data_bitmap();
    check_and_fix_duplicate_blocks();
    check_and_fix_bad_blocks();

    // Write the corrected bitmaps and inodes
    write_bitmaps();
    write_inodes();

    fclose(fp);
    return 0;
}

