#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define N_SECTORS   63
#define N_HEADS     255
#define CYLINDER    (N_SECTORS * N_HEADS)
#define SECTOR      512
#define UNIT        (CYLINDER * SECTOR)

typedef struct partition_table_entry
{
    uint8_t         active;
    uint8_t         begin_chs[3];
    uint8_t         id;
    uint8_t         end_chs[3];
    uint8_t         start_lba[4];
    uint8_t         size[4];
} pte_t;

typedef struct master_boot_record
{
    uint8_t         code[440];
    uint8_t         serial[4];
    uint8_t         reserved[2];
    pte_t           ptable[4];
    uint8_t         signature[2];
} mbr_t;

static inline uint8_t
to_cylinder(uint32_t lba)
{
    return lba / CYLINDER;
}

static inline uint8_t
to_head(uint32_t lba)
{
    return lba % CYLINDER / N_SECTORS;
}

static inline uint8_t
to_sector(uint32_t lba)
{
    return lba % CYLINDER % N_SECTORS + 1;
}

static inline void
fill_pte(pte_t* pte, uint32_t start, uint32_t end, uint8_t id, uint8_t active)
{
    if (start > end) {
        return;
    }

    for (int i = 0; i < 4; i++) {
        pte->start_lba[i] = (start >> (i * 8)) & 0xff;
    }

    for (int i = 0; i < 4; i++) {
        pte->size[i] = ((end - start + 1) >> (i * 8)) & 0xff;
    }

    pte->begin_chs[0] = to_head(start);
    pte->begin_chs[1] = to_sector(start);
    pte->begin_chs[2] = to_cylinder(start);

    pte->end_chs[0] = to_head(end);
    pte->end_chs[1] = to_sector(end);
    pte->end_chs[2] = to_cylinder(end);

    pte->id = id;

    if (active != 0) {
        pte->active = 0x80;
    }
}


int
main(int argc, char* argv[])
{
    int         fd;
    struct stat info;
    uint32_t    size;
    uint32_t    start_lba;
    uint32_t    end_lba;
    mbr_t       mbr;

    if (argc < 2) {
        fprintf(stderr, "usage: mbrtool <file>\n");
        exit(EXIT_FAILURE);
    }

    fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (fstat(fd, &info) < 0) {
        perror("fstat");
        close(fd);
        exit(EXIT_FAILURE);
    }

    start_lba = 1 * N_SECTORS;

    size = info.st_size;
    size -= start_lba * SECTOR;
    if (size < UNIT) {
        fprintf(stderr, "file is too small.\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    end_lba = size / SECTOR;

    memset(&mbr, 0, sizeof(mbr_t));

    fill_pte(&mbr.ptable[0], start_lba, end_lba, 0x83, 1);

    mbr.signature[0] = 0x55;
    mbr.signature[1] = 0xaa;

    write(fd, &mbr, sizeof(mbr_t));

    close(fd);

    printf("%u\n", end_lba - start_lba + 1);

    exit(EXIT_SUCCESS);
}

