#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#define BUFF_SIZE 5000
#define CHAR_RANGE 128

void applyFunction(char string[]) {
    bool str_1_data[CHAR_RANGE];
    bool str_2_data[CHAR_RANGE];
    memset(str_1_data, 0, sizeof(str_1_data));
    memset(str_2_data, 0, sizeof(str_2_data));

    size_t it = 0;
    while (string[it] != 126) {
        if (str_1_data[string[it]] != true) {
            str_1_data[string[it]] = true;
        }
        ++it;
    }
    ++it;
    while (it != strlen(string)) {
        if (str_2_data[string[it]] != true) {
            str_2_data[string[it]] = true;
        }
        ++it;
    }
    size_t res_it = 0;
    memset(string, 0, sizeof(char) * BUFF_SIZE);
    for (int i = 0; i < CHAR_RANGE; ++i) {
        if (str_1_data[i] == true && str_2_data[i] == true) {
            string[res_it] = (char) i;
            ++res_it;
        }
    }
}


int main(int argc, char *argv[]) {
    char str_buff[BUFF_SIZE + 1];  // char занимает один байт, тогда размер буфера будет 5000байт
    memset(str_buff, 0, sizeof(str_buff));
    if (argc != 3) {
        printf("Неправильный запуск программы, аргументы указываются так: ./main.exe input.txt output.txt\n");
        return 0;
    }

    int fd_1[2], fd_2[2];
    pid_t fork_1_res;
    ssize_t size;
    // Создаем неименованный канал процесс_1 -> процесс_2
    if (pipe(fd_1) < 0) {
        printf("Can\'t open first pipe\n");
        exit(-1);
    }
    // Создаем неименованный канал процесс_2 -> процесс_3
    if (pipe(fd_2) < 0) {
        printf("Can\'t open second pipe\n");
        exit(-1);
    }

    // создаем дочерний процесс_2
    fork_1_res = fork();
    if (fork_1_res < 0) {
        printf("Can\'t fork child (process_2)\n");
        exit(-1);
    } else if (fork_1_res > 0) { // процесс_1
        if (close(fd_1[0]) < 0) {
            printf("process_1: Can\'t close reading side of pipe\n");
            exit(-1);
        }
        int input_file;

        if ((input_file = open(argv[1], O_RDONLY, 0666)) < 0) {
            printf("Can\'t open input file %s\n", argv[1]);
            exit(-1);
        }
        // считываем из входного файла
        size = read(input_file, str_buff, BUFF_SIZE);
        if (close(input_file) < 0) {
            printf("Can\'t close input file\n");
        }
        size = write(fd_1[1], str_buff, BUFF_SIZE); // передача в канал_1
        if (size != BUFF_SIZE) {
            printf("Can\'t write all string to pipe\n (size = %zu)", size);
            exit(-1);
        }
        if (close(fd_1[1]) < 0) {
            printf("process_1: Can\'t close writing side of pipe\n");
            exit(-1);
        }
    } else {  // процесс_2

        // создаем процесс_3 дочерний второму
        pid_t fork_2_res = fork();
        if (fork_2_res < 0) {
            printf("Can\'t fork child (process_3)\n");
            exit(-1);
        } else if (fork_2_res > 0) { // процесс_2
            if (close(fd_1[1]) < 0) {
                printf("process_2: Can\'t close writing side of pipe\n");
                exit(-1);
            }
            // считываем из канала_1
            if (read(fd_1[0], str_buff, BUFF_SIZE) < 0) {
                printf("Can\'t read string from pipe\n");
                exit(-1);
            }
            // Выполняем функцию
            applyFunction(str_buff);
            if (close(fd_1[0]) < 0) {
                printf("process_2: Can\'t close reading side of pipe\n");
                exit(-1);
            }

            size = write(fd_2[1], str_buff, BUFF_SIZE); // передача в канал_2

            if (size != BUFF_SIZE) {
                printf("Can\'t write all string to pipe\n (size = %zu)", size);
                exit(-1);
            }
            if (close(fd_2[1]) < 0) {
                printf("process_2: Can\'t close writing side of pipe\n");
                exit(-1);
            }
        } else { // процесс_3
            if (close(fd_2[1]) < 0) {
                printf("process_3: Can\'t close writing side of pipe\n");
                exit(-1);
            }
            // считываем из канала_2
            if (read(fd_2[0], str_buff, BUFF_SIZE) < 0) {
                printf("Can\'t read string from pipe\n");
                exit(-1);
            }
            if (close(fd_2[0]) < 0) {
                printf("process_3: Can\'t close reading side of pipe\n");
                exit(-1);
            }
            int output_file;

            if ((output_file = open(argv[2], O_WRONLY | O_CREAT, 0666)) < 0) {
                printf("Can\'t open file\n");
                exit(-1);
            }
            printf("%s\n", str_buff);
            size = write(output_file, str_buff, strlen(str_buff)); // записываем в выходной файл
            if (size != strlen(str_buff)) {
                printf("Can\'t write all string\n");
                exit(-1);
            }
            if (close(output_file) < 0) {
                printf("Can\'t close file\n");
            }
        }
    }
    return 0;
}