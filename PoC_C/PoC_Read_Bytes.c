#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach-o/loader.h> // For Mach-O structures
// #include <mach-o/swap.h>   // For byte swapping ( NOT A REAL THING, SWAP HEADERS ARE DEPRECATED )
// #include <mach-o/utils.h>

#define BUFFER_SIZE 1024*4 // One KB at a time

struct file_headers {
    unsigned long int text_offset;
    unsigned long int text_size;
    unsigned long int data_offset;
    unsigned long int data_size;
    size_t size;
    size_t capacity;
    uint64_t *other_offsets; // Pointer to dynamically allocated array
};

void file_headers_init(struct file_headers *headers) {
    headers->text_offset = 0;
    headers->text_size = 0;
    headers->data_offset = 0;
    headers->data_size = 0;
    headers->size = 0;
    headers->capacity = 0;
    headers->other_offsets = NULL; // Initialize to NULL
}
void update_other_offsets(struct file_headers *headers, uint64_t new_offset) {
    if (headers->size + 1 > headers->capacity) {
        size_t new_capacity = headers->capacity == 0 ? 4 : headers->capacity * 2;
        uint64_t *new_array = realloc(headers->other_offsets, new_capacity * sizeof(uint64_t));
        if (!new_array) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        headers->other_offsets = new_array;
        headers->capacity = new_capacity;
    }
    headers->other_offsets[headers->size++] = new_offset;
}

void free_file_headers(struct file_headers *headers) {
    if (!headers) return;
    free(headers->other_offsets);
    headers->other_offsets = NULL;
    headers->size = 0;
    headers->capacity = 0;
}

// Following guide at https://lowlevelbits.org/parsing-mach-o-files/ for this section of code

struct file_headers dump_segment_commands(FILE *obj_file, int offset, int is_swap, uint32_t ncmds);

struct file_headers dump_segments(FILE *obj_file); // Function prototype for dumping Mach-O segments

int main(int argc, char *argv[]) {

    // struct file_headers test;
    // file_headers_init(&test);

    // printf("Initial other_offsets: %p\n", (void*)test.other_offsets);
    // update_other_offsets(&test, 1);
    // update_other_offsets(&test, 2);
    // update_other_offsets(&test, 3);
    // printf("Capacity: %zu\n", test.capacity);
    // for (size_t i = 0; i < test.size; i++) {
    //     printf("Offset %zu: %llu\n", i, (unsigned long long)test.other_offsets[i]);
    // }
    // free_file_headers(&test);

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        fprintf(stderr, "Example: %s /path/to/macho/file\n", argv[0]);
        fprintf(stderr, "This program reads bytes from the specified file and dumps Mach-O segment information if applicable.\n");
        fprintf(stderr, "Use with flag --dump-segments to also dump segment details.\n");
        return EXIT_FAILURE;
    }

    FILE *file;
    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    struct file_headers headers;
    file_headers_init(&headers);

    // Open the file in binary mode
    file = fopen(argv[1], "rb");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    if (argc == 3 && strcmp(argv[2], "--dump-segments") == 0) {
        headers = dump_segments(file);
    } else {
        fprintf(stderr, "Unsure what to do with %s flag.\n", argv[2]);
        fprintf(stderr, "Usage: %s %s [--dump-segments]\n", argv[0], argv[1]);
        return EXIT_FAILURE;
    }


    // Read bytes from the file into the buffer
    bytesRead = fread(buffer, sizeof(char), BUFFER_SIZE, file);
    if (bytesRead < 0) {
        perror("Error reading file");
        fclose(file);
        return EXIT_FAILURE;
    }
    int beginning_of_16_bytes = 0; // Need to refactor this whole thing
    unsigned long address = 0x00000000;
    int displayed_text_header = 0; // Proabably not going to use this
    while((bytesRead = fread(buffer, sizeof(char), BUFFER_SIZE, file)) > 0) {
        // Process the read bytes (for demonstration, we will just print them)
        printf("Bytes read: %zu\n", bytesRead);
        for (size_t i = 0; i < bytesRead; i++) {
            if (i % 16 == 0) {
                // printf("%p:", (void *)(&buffer[i])); // Off set in memory
                if (address == headers.text_offset && address < headers.text_offset + headers.text_size) {
                    printf("\n== TEXT SEGMENT @ 0x%08lx size 0x%08lx ==\n", headers.text_offset, headers.text_size);
                } else if (address == headers.data_offset && headers.data_size > 0) {
                    printf("\n== DATA SEGMENT @ 0x%08lx size 0x%08lx ==\n", headers.data_offset, headers.data_size);
                } else {
                    for (size_t s = 0; s < headers.size; s++) {
                        if (address == headers.other_offsets[s]) {
                            printf("\n== OTHER SEGMENT @ 0x%08llx ==\n", (unsigned long long)headers.other_offsets[s]);
                            break;
                        }
                    }
                }
                printf("0x%08lx: ", (unsigned long)address); // Offset in file
            }
            // int skip_lines = 0;
            // if (beginning_of_16_bytes == 0) {
            //     if ((unsigned char)buffer[i] == 0x00) {
            //         while(i < bytesRead && buffer[i] == 0) {
            //             for (int j=0; j<16; j++) {
            //                 continue;
            //             }
            //             address += 16;
            //             i+= 16;
            //         }
            //         continue;
            //     }
            // }
            beginning_of_16_bytes = -1;
            printf("%02x ", (unsigned char)buffer[i]);
            if ((i + 1) % 16 == 0) {
                beginning_of_16_bytes = 0;
                printf("\n");
            }
            address++;
        }
        printf("\n");
        printf("Reading next chunk...\n");

    }

    // Process the read bytes (for demonstration, we will just print them)
    printf("Bytes read: %zu\n", bytesRead);
    for (size_t i = 0; i < bytesRead; i++) {
        if (i % 16 == 0) {
            // printf("%p:", (void *)(&buffer[i])); // Off set in memory
            printf("0x%08lx: ", (unsigned long)i); // Offset in file
        }
        printf("%02x ", (unsigned char)buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");

    // Close the file
    fclose(file);
    return EXIT_SUCCESS;
}

uint32_t read_magic(FILE *file, int offset) {
    uint32_t magic;
    if (fseek(file, offset, SEEK_SET) != 0) {
        perror("Error seeking to magic number");
        return 0;
    }
    if (fread(&magic, sizeof(uint32_t), 1, file) != 1) {
        perror("Error reading magic number");
        return 0;
    }
    return magic;
}

int is_magic_64(uint32_t magic) {
    return magic == MH_MAGIC_64 || magic == MH_CIGAM_64;
}

int should_swap_bytes(uint32_t magic) {
    return magic == MH_CIGAM || magic == MH_CIGAM_64;
}


void *load_bytes(FILE *obj_file, int offset, int size) {
    void *buf = calloc(1, size); // Allocate zero-initialized buffer
    if (!buf) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    fseek(obj_file, offset, SEEK_SET);
    if (fread(buf, 1, size, obj_file) != size) {
        fprintf(stderr, "Error reading bytes from file\n");
        free(buf);
        return NULL;
    }
    return buf;
}

// We are still checking in both of these functions, is the is_64 actually set.
// We really don't need it, but I want to see everything, and split everything up in a very easy to read format.

struct file_headers dump_mach_header_64(FILE *obj_file, int offset, int is_64, int is_swap) {
    if (is_64) {
        int header_size = sizeof(struct mach_header_64);
        struct mach_header_64 *header = load_bytes(obj_file, offset, header_size);
        if (!header) return (struct file_headers){0};
        // if (is_swap) macho_swap_mach_header_64(header, 0);
        printf("Mach Header 64-bit:\n");
        printf("  Header Size: %d bytes\n", header_size);
        printf("  Magic: 0x%08x\n", header->magic);
        printf("  CPU Type: %u\n", header->cputype);
        printf("  CPU Subtype: %u\n", header->cpusubtype);
        printf("  File Type: %u\n", header->filetype);
        printf("  Number of Commands: %u\n", header->ncmds);
        printf("  Size of Commands: %u\n", header->sizeofcmds);
        printf("  Flags: 0x%08x\n", header->flags);
        uint32_t ncmds = header->ncmds;
        free(header);

        return dump_segment_commands(obj_file, offset + header_size, is_swap, ncmds);
    }
}

void dump_mach_header_32(FILE *obj_file, int offset, int is_64, int is_swap) {
    if (!is_64) {
        int header_size = sizeof(struct mach_header);
        struct mach_header *header = load_bytes(obj_file, offset, header_size);
        if (!header) return;
        // if (is_swap) swap_mach_header(header, 0);
        printf("Mach Header 32-bit:\n");
        printf("  Header Size: %d bytes\n", header_size);
        printf("  Magic: 0x%08x\n", header->magic);
        printf("  CPU Type: %u\n", header->cputype);
        printf("  CPU Subtype: %u\n", header->cpusubtype);
        printf("  File Type: %u\n", header->filetype);
        printf("  Number of Commands: %u\n", header->ncmds);
        printf("  Size of Commands: %u\n", header->sizeofcmds);
        printf("  Flags: 0x%08x\n", header->flags);
        uint32_t ncmds = header->ncmds;
        free(header);
        dump_segment_commands(obj_file, offset + header_size, is_swap, ncmds);
    }
}

struct file_headers dump_segment_commands(FILE *obj_file, int offset, int is_swap, uint32_t ncmds) {
    uint64_t cur = (uint64_t)offset;
    struct file_headers result;
    file_headers_init(&result);
    
    for (uint32_t i = 0; i < ncmds; i++) {
        struct load_command *lc = load_bytes(obj_file, (int)cur, sizeof(struct load_command));
        if (!lc) return (struct file_headers){0};
        uint32_t cmd = lc->cmd;
        uint32_t cmdsize = lc->cmdsize;
        free(lc);

        if (cmd == LC_SEGMENT_64) {
            struct segment_command_64 *sc = load_bytes(obj_file, (int)cur, sizeof(struct segment_command_64));
            if (!sc) return (struct file_headers){0};

            printf("Segment Name: %s\n", sc->segname);

            if (strncmp(sc->segname, "__TEXT", 6) == 0 || strncmp(sc->segname, SEG_TEXT, 6) == 0) {
                result.text_offset = sc->fileoff;
                result.text_size = sc->filesize;
            } else if (strncmp(sc->segname, "__DATA", 6) == 0 || strncmp(sc->segname, SEG_DATA, 6) == 0) {
                result.data_offset = sc->fileoff;
                result.data_size = sc->filesize;
            } else if (strncmp(sc->segname, "__LINKEDIT", 10) == 0 || strncmp(sc->segname, SEG_LINKEDIT, 10) == 0) {
                // Skip LINKEDIT segment
            } else if (strncmp(sc->segname, "__PAGEZERO", 10) == 0 || strncmp(sc->segname, SEG_PAGEZERO, 10) == 0) {
                // Skip PAGEZERO segment
            } else {
                update_other_offsets(&result, sc->fileoff);
            }

            printf("  VM Address: 0x%016llx\n", (unsigned long long)sc->vmaddr);
            printf("  VM Size:    0x%016llx\n", (unsigned long long)sc->vmsize);
            printf("  File Off:   0x%016llx\n", (unsigned long long)sc->fileoff);
            printf("  File Size:  0x%016llx\n", (unsigned long long)sc->filesize);
            printf("  Sections:   %u\n", sc->nsects);

            /* iterate section_64 structures that follow the segment_command_64 */
            uint64_t sect_base = cur + sizeof(struct segment_command_64);
            for (uint32_t s = 0; s < sc->nsects; s++) {
                uint64_t sect_off = sect_base + (uint64_t)s * sizeof(struct section_64);
                struct section_64 *sec = load_bytes(obj_file, (int)sect_off, sizeof(struct section_64));
                if (!sec) continue;
                printf("    Section: %s,%s\n", sec->segname, sec->sectname);
                printf("      addr: 0x%016llx size: 0x%016llx offset: 0x%08x\n",
                       (unsigned long long)sec->addr,
                       (unsigned long long)sec->size,
                       sec->offset);
                free(sec);
            }

            free(sc);
        } else if (cmd == LC_SEGMENT) {
            struct segment_command *sc = load_bytes(obj_file, (int)cur, sizeof(struct segment_command));
            if (!sc) return (struct file_headers){0};

            printf("Segment Name: %s\n", sc->segname);
            printf("  VM Address: 0x%08x\n", sc->vmaddr);
            printf("  VM Size:    0x%08x\n", sc->vmsize);
            printf("  File Off:   0x%08x\n", sc->fileoff);
            printf("  File Size:  0x%08x\n", sc->filesize);
            printf("  Sections:   %u\n", sc->nsects);

            uint64_t sect_base = cur + sizeof(struct segment_command);
            for (uint32_t s = 0; s < sc->nsects; s++) {
                uint64_t sect_off = sect_base + (uint64_t)s * sizeof(struct section);
                struct section *sec = load_bytes(obj_file, (int)sect_off, sizeof(struct section));
                if (!sec) continue;
                printf("    Section: %s,%s\n", sec->segname, sec->sectname);
                printf("      addr: 0x%08x size: 0x%08x offset: 0x%08x\n",
                       sec->addr, sec->size, sec->offset);
                free(sec);
            }

            free(sc);
        }

        /* advance to next load command */
        if (cmdsize == 0) break; /* avoid infinite loop on malformed file */
        cur += cmdsize;
    }
    return result;
}

struct file_headers dump_segments(FILE *obj_file) {
    uint32_t magic = read_magic(obj_file, 0);
    if (is_magic_64(magic)) {
        printf("Mach-O 64-bit file detected (magic: 0x%08x)\n", magic);
        return dump_mach_header_64(obj_file, 0, 1, should_swap_bytes(magic));
    } else if (magic == MH_MAGIC || magic == MH_CIGAM) {
        printf("Mach-O 32-bit file detected (magic: 0x%08x)\n", magic);
        dump_mach_header_32(obj_file, 0, 0, should_swap_bytes(magic));  
    } else {
        fprintf(stderr, "Not a Mach-O file (magic: 0x%08x)\n", magic);
        return (struct file_headers){0};
    }
}