#include "stdio.h"
#include "../include/sys/const.h"
#include "type.h"
#include "string.h"

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: edit <file>\n");
        return 0;
    }

    char *filename = argv[1];
    char pos_str[10];
    char operation;
    int fd;
    int current_pos = 0;

    while (1) {
        printf("\nChoose your operation:\n");
        printf("1. Write\n");
        printf("2. Delete\n");
        printf("3. Show\n");
        printf("4. Choose Position\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");

        if (read(0, &operation, 1) != 1) {
            printf("Failed to read operation.\n");
            continue;
        }

        // 清除输入缓冲区中的多余字符
        char ch;
        while (read(0, &ch, 1) == 1 && ch != '\n');

        switch (operation) {
            case '1': { // Write
                printf("Input your content:\n");
                char content[BUFFER_SIZE];
                int bytes_read = read(0, content, BUFFER_SIZE - 1);
                if (bytes_read < 0) {
                    printf("Failed to read content.\n");
                    break;
                }
                content[bytes_read] = '\0';

                fd = open(filename, O_RDWR);
                if (fd == -1) {
                    printf("edit: %s: No such file or directory\n", filename);
                } else {
                    if (lseek(fd, current_pos, SEEK_SET) == -1) {
                        printf("Failed to seek to position.\n");
                        close(fd);
                        break;
                    }
                    int bytes_written = write(fd, content, strlen(content));
                    if (bytes_written == -1) {
                        printf("Failed to write to file.\n");
                    } else {
                        printf("%d bytes written.\n", bytes_written);
                        current_pos += bytes_written;
                    }
                    close(fd);
                }
                break;
            }
            case '2': { // Delete
                printf("Input how many bytes you want to delete:\n");
                if (read(0, pos_str, sizeof(pos_str) - 1) < 0) {
                    printf("Failed to read delete count.\n");
                    break;
                }
                pos_str[sizeof(pos_str) - 1] = '\0';
                int delete_count = atoi(pos_str);
                if (delete_count <= 0) {
                    printf("Invalid delete count.\n");
                    break;
                }

                fd = open(filename, O_RDWR);
                if (fd == -1) {
                    printf("edit: %s: No such file or directory\n", filename);
                } else {
                    char buffer[BUFFER_SIZE];
                    if (lseek(fd, current_pos + delete_count, SEEK_SET) == -1) {
                        printf("Failed to seek.\n");
                        close(fd);
                        break;
                    }
                    int bytes_read = read(fd, buffer, BUFFER_SIZE);
                    if (bytes_read == -1) {
                        printf("Failed to read from file.\n");
                        close(fd);
                        break;
                    }

                    if (lseek(fd, current_pos, SEEK_SET) == -1) {
                        printf("Failed to seek.\n");
                        close(fd);
                        break;
                    }
                    if (write(fd, buffer, bytes_read) == -1) {
                        printf("Failed to delete content.\n");
                    } else {
                        printf("Deleted %d bytes.\n", delete_count);
                    }

                    // 需要实现 truncate 系统调用以截断文件
                    close(fd);
                }
                break;
            }
            case '3': { // Show
                fd = open(filename, O_RDWR);
                if (fd == -1) {
                    printf("edit: %s: No such file or directory\n", filename);
                } else {
                    char buffer[BUFFER_SIZE];
                    if (lseek(fd, current_pos, SEEK_SET) == -1) {
                        printf("Failed to seek.\n");
                        close(fd);
                        break;
                    }
                    int bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
                    if (bytes_read == -1) {
                        printf("Failed to read from file.\n");
                    } else {
                        buffer[bytes_read] = '\0';
                        printf("Content:\n%s\n", buffer);
                        current_pos += bytes_read;
                    }
                    close(fd);
                }
                break;
            }
            case '4': { // Choose Position
                printf("Input your position:\n");
                if (read(0, pos_str, sizeof(pos_str) - 1) < 0) {
                    printf("Failed to read position.\n");
                    break;
                }
                pos_str[sizeof(pos_str) - 1] = '\0';
                int new_pos = atoi(pos_str);
                if (new_pos < 0) {
                    printf("Invalid position.\n");
                    break;
                }
                current_pos = new_pos;
                printf("Current position set to %d.\n", current_pos);
                break;
            }
            case '5': { // Exit
                printf("Exiting editor.\n");
                return 0;
            }
            default:
                printf("Invalid operation.\n");
                break;
        }
    }

    return 0;
}