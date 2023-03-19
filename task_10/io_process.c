#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <unistd.h>
#include <fcntl.h>

#define STR_BUF_SIZE 200
#define MSG_KEY 12345

typedef struct {
    long type;
    char text[STR_BUF_SIZE];
} message_t;

#define MAX_MSG_SIZE sizeof(message_t)

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Неправильный запуск программы, аргументы указываются так: ./main.exe input.txt output.txt\n");
        return 0;
    }
    int msg_id;
    message_t message;
    int input_file;
    char buf[STR_BUF_SIZE];
    ssize_t msg_status;
    memset(buf, 0, sizeof(buf));
    // Создаем очередь сообщений
    msg_id = msgget(MSG_KEY, 0666 | IPC_CREAT);
    if (msg_id < 0) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    // Открываем файл для чтения
    input_file = open(argv[1], O_RDONLY);
    if (input_file < 0) {
        printf("Can\'t open input file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    memset(buf, 0, sizeof(buf));
    while (read(input_file, buf, STR_BUF_SIZE - 1) > 0) {
        //        printf("process buf: %s\n", buf);
        // Отправляем сообщение в очередь
        message.type = 1;
        memcpy(message.text, buf, STR_BUF_SIZE);
        msg_status = msgsnd(msg_id, &message, MAX_MSG_SIZE, 0);
        if (msg_status < 0) {
            printf("Can't send message");
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
        memset(buf, 0, sizeof(buf));
        //        printf("process msg: %s\n", message.text);
    }
    message.type = 1;
    memset(message.text, 0, sizeof(message.text));
    msg_status = msgsnd(msg_id, &message, MAX_MSG_SIZE, 0);
    if (msg_status < 0) {
        printf("Can't send message");
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    // Получаем ответное сообщение
    msg_status = msgrcv(msg_id, &message, MAX_MSG_SIZE, 2, 0);
    if (msg_status < 0) {
        printf("Can't receive message");
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }
    int output_file;
    if ((output_file = open(argv[2], O_WRONLY | O_CREAT, 0666)) < 0) {
        printf("Can\'t open output file\n");
        exit(-1);
    }
    printf("%s\n", message.text);
    ssize_t size = write(output_file, message.text, strlen(message.text)); // записываем в файл
    if (size != strlen(message.text)) {
        printf("Can\'t write all string\n");
        exit(-1);
    }
    if (close(output_file) < 0) {
        printf("Can\'t close file\n");
    }
    if (msgctl(msg_id, IPC_RMID, NULL)) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    return 0;
}