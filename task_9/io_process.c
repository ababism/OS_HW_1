#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>

#define BUFF_SIZE 199
#define CHAR_RANGE 128
#define PIPE_1_NAME "pipe_1.fifo"
#define PIPE_2_NAME "pipe_2.fifo"

int main(int argc, char *argv[]) {
    char res[CHAR_RANGE + 1];
    char str_buff[BUFF_SIZE + 1];  // char занимает один байт, тогда размер буфера будет 5000байт
    memset(str_buff, 0, sizeof(str_buff));
    memset(res, 0, sizeof(res));
    if (argc != 3) {
        printf("Неправильный запуск программы, аргументы указываются так: ./main.exe input.txt output.txt\n");
        return 0;
    }
    // именованные каналы
    mknod(PIPE_1_NAME, S_IFIFO | 0666, 0);
    mknod(PIPE_2_NAME, S_IFIFO | 0666, 0);

    int fd_1, fd_2;
    ssize_t size;

    int input_file;

    if ((input_file = open(argv[1], O_RDONLY, 0666)) < 0) {
        printf("Can\'t open input file %s\n", argv[1]);
        exit(-1);
    }
    if ((fd_1 = open(PIPE_1_NAME, O_WRONLY)) < 0) {
        printf("Can\'t open FIFO for writing\n");
        exit(-1);
    }

    ssize_t read_size;
    do {
        memset(str_buff, 0, sizeof(str_buff));
        // считываем из входного файла
        read_size = read(input_file, str_buff, BUFF_SIZE);
        if (read_size == -1) {
            printf("Can\'t read file\n");
            exit(-1);
        }
        size = write(fd_1, str_buff, read_size + 1); // передача в канал_1
        if (size != read_size + 1) {
            printf("Can\'t write all string to FIFO\n (size = %zu)", size);
            exit(-1);
        }
        //        printf("%s\n", str_buf);
    } while (read_size == BUFF_SIZE);

    if (close(input_file) < 0) {
        printf("Can\'t close input file\n");
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
    size = read(fd_2, res, CHAR_RANGE + 1);
    if (size < 0) {
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
    printf("%s\n", res);
    size = write(output_file, res, strlen(res)); // записываем в файл
    if (size != strlen(res)) {
        printf("Can\'t write all string\n");
        exit(-1);
    }
    if (close(output_file) < 0) {
        printf("Can\'t close file\n");
    }
    return 0;
}