#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define PAGE_SIZE 256
#define PHYSICAL_MEMORY_SIZE 65536
#define FRAME_SIZE 256
#define BACKING_STORE_FILE "BACKING_STORE.bin"

typedef struct {
    int page_number;
    int frame_number;
} TLBEntry;

int find_free_frame() {
    static int frame_number = 0;
    return frame_number++;
}

void update_tlb(int page_number, int frame_number, TLBEntry tlb[], int *tlb_next_entry) {
    tlb[*tlb_next_entry].page_number = page_number;
    tlb[*tlb_next_entry].frame_number = frame_number;
    *tlb_next_entry = (*tlb_next_entry + 1) % TLB_SIZE;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <logical_address_file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    FILE *logical_addresses = fopen(filename, "r");

    if (!logical_addresses) {
        perror("Unable to open logical address file");
        return 1;
    }

    FILE *backing_store = fopen(BACKING_STORE_FILE, "rb");
    if (!backing_store) {
        perror("Unable to open BACKING_STORE.bin");
        return 1;
    }

    TLBEntry tlb[TLB_SIZE];
    int tlb_next_entry = 0;

    int page_table[PAGE_TABLE_SIZE];
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        page_table[i] = -1; // Initialize page table with invalid entries
    }

    unsigned char physical_memory[PHYSICAL_MEMORY_SIZE];

    char buffer[PAGE_SIZE];

    int tlb_hits = 0;
    int page_faults = 0;
    int total_addresses = 0;

    unsigned int logical_address;

    while (fscanf(logical_addresses, "%u", &logical_address) != EOF) {
        total_addresses++;

        int page_number = (logical_address >> 8) & 0xFF;
        int offset = logical_address & 0xFF;

        // Check TLB for a hit
        int frame_number = -1;
        for (int i = 0; i < TLB_SIZE; i++) {
            if (tlb[i].page_number == page_number) {
                frame_number = tlb[i].frame_number;
                tlb_hits++;
                break;
            }
        }

        if (frame_number == -1) {
            frame_number = page_table[page_number];

            if (frame_number == -1) {
                fseek(backing_store, page_number * PAGE_SIZE, SEEK_SET);
                fread(buffer, sizeof(unsigned char), PAGE_SIZE, backing_store);

                frame_number = find_free_frame();
                page_table[page_number] = frame_number;

                int physical_address = frame_number * FRAME_SIZE;
                memcpy(&physical_memory[physical_address], buffer, PAGE_SIZE);

                page_faults++;
            }

            update_tlb(page_number, frame_number, tlb, &tlb_next_entry);
        }

        int physical_address = frame_number * FRAME_SIZE + offset;
        unsigned char value = physical_memory[physical_address];

        printf("Virtual address: %u Physical address: %d Value: %d\n", logical_address, physical_address, (char)value);
    }

    double page_fault_rate = (double)page_faults / total_addresses * 100.0;
    double tlb_hit_rate = (double)tlb_hits / total_addresses * 100.0;

    printf("Page-fault rate: %.2f%%\n", page_fault_rate);
    printf("TLB hit rate: %.2f%%\n", tlb_hit_rate);

    fclose(logical_addresses);
    fclose(backing_store);

    return 0;
}
