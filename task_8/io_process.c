#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#define BUFF_SIZE 5000
#define CHAR_RANGE 128
#define PIPE_1_NAME "pipe_1.fifo"
#define PIPE_2_NAME "pipe_2.fifo"

int main(int argc, char *argv[]) {
    char str_buff[BUFF_SIZE + 1];  // char занимает один байт, тогда размер буфера будет 5000байт
    memset(str_buff, 0, sizeof(str_buff));
    if (argc != 3) {
        printf("Неправильный запуск программы, аргументы указываются так: ./main.exe input.txt output.txt\n");
        return 0;
    }
    // именованные каналы
    mknod(PIPE_1_NAME, S_IFIFO | 0666, 0);
    mknod(PIPE_2_NAME, S_IFIFO | 0666, 0);

    int fd_1, fd_2;
    ssize_t size;

    // создаем независимый процесс_2
    int input_file;

    if ((input_file = open(argv[1], O_RDONLY, 0666)) < 0) {
        printf("Can\'t open input file %s\n", argv[1]);
        exit(-1);
    }
    // считываем из входного файла
    read(input_file, str_buff, BUFF_SIZE);
    if (close(input_file) < 0) {
        printf("Can\'t close input file\n");
    }
    if ((fd_1 = open(PIPE_1_NAME, O_WRONLY)) < 0) {
        printf("Can\'t open FIFO for writing\n");
        exit(-1);
    }
    size = write(fd_1, str_buff, BUFF_SIZE); // передача в канал_1
    if (size != BUFF_SIZE) {
        printf("Can\'t write all string to FIFO\n (size = %zu)", size);
        exit(-1);
    }
    if (close(fd_1) < 0) {
        printf("parent: Can\'t close FIFO_1\n");
        exit(-1);
    }

    // процесс_1
    if ((fd_2 = open(PIPE_2_NAME, O_RDONLY)) < 0) {
        printf("Can\'t open FIFO for reading\n");
        exit(-1);
    }
    // считываем из канала_2
    if (read(fd_2, str_buff, BUFF_SIZE) < 0) {
        printf("Can\'t read string from FIFO_2\n");
        exit(-1);
    }
    if (close(fd_2) < 0) {
        printf("child: Can\'t close FIFO_2\n");
        exit(-1);
    }
    int output_file;

    if ((output_file = open(argv[2], O_WRONLY | O_CREAT, 0666)) < 0) {
        printf("Can\'t open output file\n");
        exit(-1);
    }
    printf("%s\n", str_buff);
    size = write(output_file, str_buff, strlen(str_buff)); // записываем в файл
    if (size != strlen(str_buff)) {
        printf("Can\'t write all string\n");
        exit(-1);
    }
    if (close(output_file) < 0) {
        printf("Can\'t close file\n");
    }
    return 0;
}