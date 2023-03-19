#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#define BUFF_SIZE 200
#define CHAR_RANGE 128
#define PIPE_1_NAME "pipe_1.fifo"
#define PIPE_2_NAME "pipe_2.fifo"

bool str_1_data[CHAR_RANGE];
bool str_2_data[CHAR_RANGE];
char res[CHAR_RANGE + 1];
bool tilda_flag = false;

char str_buff[BUFF_SIZE];  // char занимает один байт, тогда размер буфера будет 5000байт

void applyFunction() {
    size_t it = 0;
    while (it != strlen(str_buff)) {
        if (str_buff[it] == 126) {
            tilda_flag = true;
        }
        if (tilda_flag) {
            if (str_2_data[str_buff[it]] != true) {
                str_2_data[str_buff[it]] = true;
            }
        } else {
            if (str_1_data[str_buff[it]] != true) {
                str_1_data[str_buff[it]] = true;
            }
        }
        ++it;
    }
}

void makeRes() {
    size_t res_it = 0;
    for (int i = 0; i < CHAR_RANGE; ++i) {
        if (str_1_data[i] == true && str_2_data[i] == true) {
            res[res_it] = (char) i;
            ++res_it;
        }
    }
}

int main(int argc, char *argv[]) {
    memset(str_buff, 0, sizeof(str_buff));

    memset(str_1_data, 0, sizeof(str_1_data));
    memset(str_2_data, 0, sizeof(str_2_data));
    memset(res, 0, sizeof(res));


    // именованные каналы
    mknod(PIPE_1_NAME, S_IFIFO | 0666, 0);
    mknod(PIPE_2_NAME, S_IFIFO | 0666, 0);

    int fd_1, fd_2;
    ssize_t size;

    // процесс_2
    if ((fd_1 = open(PIPE_1_NAME, O_RDONLY)) < 0) {
        printf("Can\'t open FIFO for reading\n");
        exit(-1);
    }

    ssize_t read_size;
    do {
        read_size = read(fd_1, str_buff, BUFF_SIZE);
        //        printf("%s\n", str_buf);
        // считываем из канала_1
        if (read_size < 0) {
            printf("Can\'t read string from FIFO\n");
            exit(-1);
        }
        // Выполняем функцию
        applyFunction();
    } while (read_size == BUFF_SIZE);

    if (close(fd_1) < 0) {
        printf("child: Can\'t close FIFO\n");
        exit(-1);
    }

    if ((fd_2 = open(PIPE_2_NAME, O_WRONLY)) < 0) {
        printf("Can\'t open FIFO for writing\n");
        exit(-1);
    }
    // формируем результат;
    makeRes();
    size = write(fd_2, res, CHAR_RANGE + 1); // передача в канал_2
    printf("%s\n", res);

    if (size != CHAR_RANGE + 1) {
        printf("Can\'t write all string to FIFO_2\n (size = %zu)", size);
        exit(-1);
    }
    if (close(fd_2) < 0) {
        printf("parent: Can\'t close FIFO_2\n");
        exit(-1);
    }
    return 0;
}