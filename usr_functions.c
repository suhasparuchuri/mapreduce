#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <regex.h>

#include "common.h"
#include "usr_functions.h"

#define ALPHABET_SZ 26
#define ITM_LINE_SZ 16

int letter_counter_map(DATA_SPLIT * split, int fd_out)
{
    size_t frequency[26];
    off_t start = lseek(split->fd, 0, SEEK_CUR);
    char read_buff[(split->size - start)];

    memset(frequency, 0, 26 * sizeof(size_t));
    if ((read(split->fd, read_buff, split->size - start)) < 0) {
        perror("Failed to read fd");
        return -1;
    }

    for (off_t i = 0; i < split->size - start; ++i) {
        if ('a' <= read_buff[i] && read_buff[i] <= 'z') {
            ++frequency[read_buff[i] - 'a'];
        } else if ('A' <= read_buff[i] && read_buff[i] <= 'Z') {
            ++frequency[(read_buff[i] + ' ') - 'a'];
        }
    }

    for (int i = 0; i < ALPHABET_SZ; ++i) {
        char write_buff[ITM_LINE_SZ];
        sprintf(write_buff, "%c:%zu\n", i + 'a', frequency[i]);
        if (write(fd_out, write_buff, strlen(write_buff)) < 0 ) {
            perror("Failed to write to fd");
            return -1;
        }
    }

    return 0;
}

int letter_counter_reduce(int * p_fd_in, int fd_in_num, int fd_out)
{
    size_t frequency[ALPHABET_SZ];
    memset(frequency, 0, ALPHABET_SZ * sizeof(size_t));
    for (int i = 0; i < fd_in_num; ++i) {
        int fd = *(p_fd_in + i);

        struct stat fd_stat;
        if ((fstat(fd, &fd_stat)) < 0) {
            fprintf(stderr, "[REDUCE fd@%d %d] ", i, fd);
            perror("Failed to stat file");
            return -1;
        }

        char read_buff[fd_stat.st_size];
        if ((read(fd, read_buff, fd_stat.st_size)) < 0) {
            perror("[REDUCE] Failed to read fd");
            return -1;
        }
        size_t pos_s = 0;
        for (size_t pos_e = 0; pos_e < fd_stat.st_size; ++pos_e) {
            if (read_buff[pos_e] == '\n') {
                char  _char = read_buff[pos_s];
                char _freq[16];
                memset(_freq, 0, 16);
                memcpy(_freq, read_buff + pos_s + 2, pos_e - (pos_s + 2));
                char *end;

                size_t freq = strtoll(_freq, &end, 10);
                if (*end == '\0') {
                    frequency[_char - 'a'] += freq;
                } else {
                    return -1;
                }
                pos_s = pos_e + 1;
            }
        }
    }

    for (int i = 0; i < ALPHABET_SZ; ++i) {
        char write_buff[ITM_LINE_SZ];
        sprintf(write_buff, "%c:%zu\n", i + 'a', frequency[i]);
        if (write(fd_out, write_buff, strlen(write_buff)) < 0 ) {
            perror("Failed to write to fd");
            return -1;
        }
    }

    return 0;
}

int add_to(char* payload, char*** lines, size_t len, size_t* lines_sz, size_t* lines_malloced) {
    char** _lines = *lines;

    if (*lines_sz == *lines_malloced) {
        *lines_malloced *= 2;
        char** new_lines = (char**)malloc(sizeof(char*) * (*lines_malloced));
        if (new_lines == NULL) {
            perror("Unable to allocate memory");
            return -1;
        }

        for (int i = 0; i < *lines_sz; ++i) {
            *(new_lines + i) = *(_lines + i);
        }
        free(_lines);
        *lines = new_lines;
        _lines = *lines;
    }

    *(_lines + (*lines_sz)) = (char*) malloc(sizeof(char) * strlen(payload));
    strncpy(*(_lines + (*lines_sz)), payload, len);
    (*lines_sz)++;
    return strlen(payload);
}

int word_finder_map(DATA_SPLIT * split, int fd_out)
{
    char** lines = (char**) malloc(sizeof(char*) * 8);
    size_t lines_malloced = 8;
    size_t lines_sz = 0;

    off_t start = lseek(split->fd, 0, SEEK_CUR);
    char read_buff[split->size - start];

    if ((read(split->fd, read_buff, split->size - start)) < 0) {
        perror("Failed to read fd");
        return -1;
    }

    size_t pos_s = 0;
    for (off_t pos_e = 0; pos_e < split->size - start; ++pos_e) {
        if (read_buff[pos_e] == '\n') {
            char buf[pos_e - pos_s + 2];
            strncpy(buf, read_buff + pos_s, pos_e - pos_s + 1);
            buf[pos_e - pos_s + 1] = '\0';
            char* needle = strstr(buf, (char*)split->usr_data);
            if (needle != NULL) {
                add_to(buf, &lines, strlen(buf), &lines_sz, &lines_malloced);
            }
            pos_s = pos_e + 1;
        }
    }

    for (int i = 0; i < lines_sz; ++i) {
        if (write(fd_out, *(lines + i), strlen(*(lines + i))) < 0 ) {
            perror("Failed to write to fd");
            return -1;
        }
    }

    for (int i = 0; i < lines_sz; ++i) {
        free(*(lines + i));
    }
    free(lines);
    return 0;
}

int word_finder_reduce(int * p_fd_in, int fd_in_num, int fd_out) {
    char** lines = (char**) malloc(sizeof(char*) * 8);
    size_t lines_malloced = 8;
    size_t lines_sz = 0;

    for (int i = 0; i < fd_in_num; ++i) {
        int fd = *(p_fd_in + i);

        struct stat fd_stat;
        if ((fstat(fd, &fd_stat)) < 0) {
            fprintf(stderr, "[REDUCE fd@%d %d] ", i, fd);
            perror("Failed to stat file");
            return -1;
        }

        char read_buff[fd_stat.st_size];
        if ((read(fd, read_buff, fd_stat.st_size)) < 0) {
            perror("[REDUCE] Failed to read fd");
            return -1;
        }

        size_t pos_s = 0;
        for (size_t pos_e = 0; pos_e < fd_stat.st_size; ++pos_e) {
            if (read_buff[pos_e] == '\n') {
                char buf[pos_e - pos_s + 2];
                strncpy(buf, read_buff + pos_s, pos_e - pos_s + 1);
                buf[pos_e - pos_s + 1] = '\0';
                add_to(buf, &lines, pos_e - pos_s + 2, &lines_sz, &lines_malloced);
                pos_s = pos_e + 1;
            }
        }
    }
    for (int i = 0; i < lines_sz; ++i) {
        if (write(fd_out, *(lines + i), strlen(*(lines + i))) < 0 ) {
            perror("Failed to write to fd");
            return -1;
        }
    }

    for (int i = 0; i < lines_sz; ++i) {
        free(*(lines + i));
    }
    free(lines);

    return 0;
}
