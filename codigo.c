#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_PROCESSES 100

typedef struct PageTableEntry {
    int frame_number;
} PageTableEntry;

typedef struct Process {
    int process_id;
    int size;
    int num_pages;
    PageTableEntry *page_table;
} Process;

typedef struct Frame {
    unsigned char *data;
    int free;  // Flag para indicar se o frame está livre (1) ou ocupado (0)
} Frame;

typedef struct {
    Frame *frames;
    int num_frames;
    int page_size;
} PhysicalMemory;

void init_physical_memory(PhysicalMemory *pm, int total_size, int page_size) {
    pm->page_size = page_size;
    pm->num_frames = total_size / page_size;
    pm->frames = (Frame *)malloc(pm->num_frames * sizeof(Frame));
    if (pm->frames == NULL) {
        fprintf(stderr, "Erro ao alocar memória para os frames.\n");
        exit(1);
    }

    // Inicializa os frames como livres
    for (int i = 0; i < pm->num_frames; i++) {
        pm->frames[i].data = (unsigned char *)malloc(page_size * sizeof(unsigned char));
        if (pm->frames[i].data == NULL) {
            fprintf(stderr, "Erro ao alocar memória para os dados do frame.\n");
            exit(1);
        }
        memset(pm->frames[i].data, 0, page_size);  // Preenche o frame com zeros
        pm->frames[i].free = 1;  // Define o frame como livre
    }
}

Process *create_process(int process_id, int size, int page_size, PhysicalMemory *pm) {
    Process *p = (Process *)malloc(sizeof(Process));
    if (p == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o processo.\n");
        exit(1);
    }
    p->process_id = process_id;
    p->size = size;
    p->num_pages = (size + page_size - 1) / page_size;
    p->page_table = (PageTableEntry *)malloc(p->num_pages * sizeof(PageTableEntry));
    if (p->page_table == NULL) {
        fprintf(stderr, "Erro ao alocar memória para a tabela de páginas do processo.\n");
        exit(1);
    }

    // Aloca os frames para o processo
    int page_index = 0;
    for (int i = 0; i < pm->num_frames && page_index < p->num_pages; i++) {
        if (pm->frames[i].free) {
            p->page_table[page_index].frame_number = i;
            pm->frames[i].free = 0;  // Marca o frame como ocupado
            page_index++;
        }
    }

    if (page_index < p->num_pages) {
        printf("Not enough memory to allocate process %d\n", process_id);
        exit(1);
    }

    // Simula o preenchimento dos dados do processo
    for (int i = 0; i < p->num_pages; i++) {
        // Simula preenchimento com dados aleatórios
        for (int j = 0; j < pm->page_size && (i * page_size + j) < size; j++) {
            pm->frames[p->page_table[i].frame_number].data[j] = rand() % 256;
        }
    }

    return p;
}

void display_physical_memory(PhysicalMemory *pm) {
    int free_frames_count = 0;

    printf("Physical Memory:\n");
    for (int i = 0; i < pm->num_frames; i++) {
        printf("Frame %d: ", i);
        if (pm->frames[i].free) {
            printf("Free\n");
            free_frames_count++;
        } else {
            for (int j = 0; j < pm->page_size; j++) {
                printf("%02X ", pm->frames[i].data[j]);
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

    printf("Informe o tamanho da memória física: ");
    scanf("%d", &physical_memory_size);
    printf("Informe o tamanho da página (quadro): ");
    scanf("%d", &page_size);
    printf("Informe o tamanho máximo de um processo: ");
    scanf("%d", &max_process_size);

    init_physical_memory(&pm, physical_memory_size, page_size);

    Process *processes[MAX_PROCESSES];  // Array de ponteiros para Process
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
                    processes[process_count] = create_process(process_id, size, page_size, &pm);
                    process_count++;
                }
                break;
            }
            case 3: {
                int process_id;
                printf("Informe o ID do processo: ");
                scanf("%d", &process_id);
                if (process_id <= process_count && process_id > 0) {
                    display_page_table(processes[process_id - 1]);
                } else {
                    printf("Processo não encontrado.\n");
                }
                break;
            }
        }
    } while (choice != 4);

    // Libera a memória alocada
    for (int i = 0; i < process_count; i++) {
        free(processes[i]->page_table);
        free(processes[i]);
    }

    for (int i = 0; i < pm.num_frames; i++) {
        free(pm.frames[i].data);
    }
    free(pm.frames);

    return 0;
}
