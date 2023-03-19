#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <stdbool.h>

#define CHAR_RANGE 128

bool str_1_data[CHAR_RANGE];
bool str_2_data[CHAR_RANGE];
bool tilda_flag = false;

#define STR_BUF_SIZE 200
#define MAX_MSG_SIZE sizeof(message_t)
#define MSG_KEY 12345

typedef struct {
    long type;
    char text[STR_BUF_SIZE];
} message_t;

void applyFunction(char string[]) {
    size_t it = 0;
    while (it != strlen(string)) {
        if (string[it] == 126) {
            tilda_flag = true;
        }
        if (tilda_flag) {
            if (str_2_data[string[it]] != true) {
                str_2_data[string[it]] = true;
            }
        } else {
            if (str_1_data[string[it]] != true) {
                str_1_data[string[it]] = true;
            }
        }
        ++it;
    }
}

void makeRes(char string[]) {
    size_t res_it = 0;
    memset(string, 0, sizeof(char) * STR_BUF_SIZE);
    for (int i = 0; i < CHAR_RANGE; ++i) {
        if (str_1_data[i] == true && str_2_data[i] == true) {
            string[res_it] = (char) i;
            ++res_it;
        }
    }
}

int main(int argc, char *argv[]) {
    int msg_id;
    message_t message;
    ssize_t msg_status;

    // Создаем очередь сообщений
    msg_id = msgget(MSG_KEY, 0666 | IPC_CREAT);
    if (msg_id < 0) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    while (true) {
        printf("main: start\n");
        // Получаем сообщение с частью текста из очереди
        msg_status = msgrcv(msg_id, &message, MAX_MSG_SIZE, 1, 0);
        if (msg_status < 0) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        if (msg_status - sizeof(long) < 0) {
            printf("main: break1\n");
            break;
        }
        if (strlen(message.text) == 0) {
            printf("main: break2\n");
            break;
        }
        // Обрабатываем сообщение
        applyFunction(message.text);
        //        printf("main: %s\n", message.text);
    }
    // Отправляем результат программы в сообщении
    makeRes(message.text);
    message.type = 2;
    msg_status = msgsnd(msg_id, &message, MAX_MSG_SIZE, 0);
    printf("main: sent\n");
    if (msg_status < 0) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    return 0;
    // завершаем
}