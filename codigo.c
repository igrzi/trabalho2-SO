#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PROCESSES 100

typedef struct {
    int frame_number;
} PageTableEntry;

typedef struct {
    int process_id;
    int size;
    int num_pages;
    PageTableEntry *page_table;
} Process;

typedef struct {
    unsigned char *memory;
    int total_size;
    int page_size;
    int num_frames;
    int *free_frames;
} PhysicalMemory;

void init_physical_memory(PhysicalMemory *pm, int total_size, int page_size) {
    pm->total_size = total_size;
    pm->page_size = page_size;
    pm->num_frames = total_size / page_size;
    pm->memory = (unsigned char *)calloc(total_size, sizeof(unsigned char));
    pm->free_frames = (int *)malloc(pm->num_frames * sizeof(int));
    for (int i = 0; i < pm->num_frames; i++) {
        pm->free_frames[i] = 1; // 1 indicates the frame is free
    }
}

Process create_process(int process_id, int size, int page_size, PhysicalMemory *pm) {
    Process p;
    p.process_id = process_id;
    p.size = size;
    p.num_pages = (size + page_size - 1) / page_size; // Round up
    p.page_table = (PageTableEntry *)malloc(p.num_pages * sizeof(PageTableEntry));

    for (int i = 0; i < p.num_pages; i++) {
        int frame_found = 0;
        for (int j = 0; j < pm->num_frames; j++) {
            if (pm->free_frames[j] == 1) {
                p.page_table[i].frame_number = j;
                pm->free_frames[j] = 0;
                frame_found = 1;
                break;
            }
        }
        if (!frame_found) {
            printf("Not enough memory to allocate process %d\n", process_id);
            exit(1);
        }
    }
    
    for (int i = 0; i < size; i++) {
        pm->memory[p.page_table[i / page_size].frame_number * page_size + (i % page_size)] = rand() % 256;
    }

    return p;
}

void display_physical_memory(PhysicalMemory *pm) {
    int free_frames_count = 0;
    
    printf("Physical Memory:\n");
    for (int i = 0; i < pm->num_frames; i++) {
        printf("Frame %d: ", i);
        if (pm->free_frames[i] == 1) {
            printf("Free\n");
            free_frames_count++;
        } else {
            for (int j = 0; j < pm->page_size; j++) {
                printf("%02X ", pm->memory[i * pm->page_size + j]);
            }
            printf("\n");
        }
    }
    
    double percent_free = (double)free_frames_count / pm->num_frames * 100.0;
    printf("Memória livre: %.2f%%\n", percent_free);
}

void display_page_table(Process *p) {
    printf("\n");
    printf("Process %d Page Table:\n", p->process_id);
    printf("Tamanho do processo: %d bytes\n", p->size);
    for (int i = 0; i < p->num_pages; i++) {
        printf("Page %d -> Frame %d\n", i, p->page_table[i].frame_number);
    }
}

int main() {
    srand(time(NULL));
    PhysicalMemory pm;
    int physical_memory_size;
    int page_size;
    int max_process_size;

    // Get configurable parameters from user
    printf("Informe o tamanho da memória física: ");
    scanf("%d", &physical_memory_size);
    printf("Informe o tamanho da página (quadro): ");
    scanf("%d", &page_size);
    printf("Informe o tamanho máximo de um processo: ");
    scanf("%d", &max_process_size);

    init_physical_memory(&pm, physical_memory_size, page_size);

    Process processes[MAX_PROCESSES];
    int process_count = 0;
    
    int choice;
    do {
    
        printf("\n1. Visualizar memória\n");
        printf("2. Criar processo\n");
        printf("3. Visualizar tabela de páginas\n");
        printf("4. Sair\n");
        printf("Escolha: ");
        scanf("%d", &choice);
        printf("\n");

        switch (choice) {
            case 1:
                display_physical_memory(&pm);
                break;
            case 2: {
                int process_id, size;
                printf("Informe o ID do processo: ");
                scanf("%d", &process_id);
                printf("Informe o tamanho do processo: ");
                scanf("%d", &size);
                if (size > max_process_size) {
                    printf("Tamanho do processo excede o tamanho máximo permitido.\n");
                } else {
                    processes[process_count] = create_process(process_id, size, pm.page_size, &pm);
                    process_count++;
                }
                break;
            }
            case 3: {
                int process_id;
                printf("Informe o ID do processo: ");
                scanf("%d", &process_id);
                int found = 0;
                for (int i = 0; i < process_count; i++) {
                    if (processes[i].process_id == process_id) {
                        display_page_table(&processes[i]);
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    printf("Processo não encontrado.\n");
                }
                break;
            }
        }
    } while (choice != 4);

    free(pm.memory);
    free(pm.free_frames);
    for (int i = 0; i < process_count; i++) {
        free(processes[i].page_table);
    }

    return 0;
}
