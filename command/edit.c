#include "stdio.h"
#include "../include/sys/const.h"
#include "type.h"
#include "string.h"


#define BUFFER_SIZE 1024

int string_to_int(const char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    while (str[i]<'0' || str[i]>'9') {
        i++;
    }

    // // 检测符号
    // if (str[i] == '-') {
    //     sign = -1;
    //     i++;
    // } else if (str[i] == '+') {
    //     i++;
    // }

    while (str[i] != '\0' && str[i] != '\n') {
        if(str[i]<'0' || str[i]>'9') {
            printf("Invalid number:%s\n", str);
            return -1;
        }
        int digit = str[i] - '0';
        result = result * 10 + digit;
        i++;
    }

    return result * sign;
}

// Function prototypes
int open_file(const char *filename, int flags);
void write_content(const char *filename, int *current_pos);
void delete_content(const char *filename, int *current_pos);
void show_content(const char *filename, int *current_pos);
void set_position(int *current_pos);
void clear_input_buffer();

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: edit <file>\n");
        return 0;
    }

    char *filename = argv[1];
    char operation;
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
        clear_input_buffer();

        switch (operation) {
            case '1':
                write_content(filename, &current_pos);
                break;
            case '2':
                delete_content(filename, &current_pos);
                break;
            case '3':
                show_content(filename, &current_pos);
                break;
            case '4':
                set_position(&current_pos);
                break;
            case '5':
                printf("Exiting editor.\n");
                return 0;
            default:
                printf("Invalid operation.\n");
                break;
        }
    }

    return 0;
}

// Open file helper function
int open_file(const char *filename, int flags) {
    int fd = open(filename, flags);
    if (fd == -1) {
        printf("edit: %s: No such file or directory\n", filename);
    }
    return fd;
}

// Clear input buffer
void clear_input_buffer() {
    char ch;
    while (read(0, &ch, 1) == 1 && ch != '\n');
}

// Write content to file
void write_content(const char *filename, int *current_pos) {
    printf("Input your content:\n");
    char content[BUFFER_SIZE];
    int bytes_read = read(0, content, BUFFER_SIZE - 1);
    if (bytes_read < 0) {
        printf("Failed to read content.\n");
        return;
    }
    content[bytes_read] = '\0';

    int fd = open_file(filename, O_RDWR);
    if (fd == -1) return;

    if (lseek(fd, *current_pos, SEEK_SET) == -1) {
        printf("Failed to seek to position.\n");
        close(fd);
        return;
    }

    int bytes_written = write(fd, content, strlen(content));
    if (bytes_written == -1) {
        printf("Failed to write to file.\n");
    } else {
        printf("%d bytes written.\n", bytes_written);
        *current_pos += bytes_written;
    }

    close(fd);
}

// Delete content from file
void delete_content(const char *filename, int *current_pos) {
    printf("Input how many bytes you want to delete:\n");
    char nr_char[10] = {0};

    if (read(0, nr_char, sizeof(nr_char) - 1) < 0) {
        printf("Failed to read delete count.\n");
        return;
    }
    nr_char[sizeof(nr_char) - 1] = '\0';

    int delete_count = string_to_int(nr_char);
    if (delete_count <= 0) {
        printf("Invalid delete count.\n");
        return;
    }

    int fd = open_file(filename, O_RDWR);
    if (fd == -1) return;

    char buffer[BUFFER_SIZE];

    if (lseek(fd, *current_pos + delete_count, SEEK_SET) == -1) {
        printf("Failed to seek.\n");
        close(fd);
        return;
    }

    int bytes_read = read(fd, buffer, BUFFER_SIZE);
    if (bytes_read == -1) {
        printf("Failed to read from file.\n");
        close(fd);
        return;
    }

    if (lseek(fd, *current_pos, SEEK_SET) == -1) {
        printf("Failed to seek.\n");
        close(fd);
        return;
    }

    if (write(fd, buffer, bytes_read) == -1) {
        printf("Failed to delete content.\n");
        close(fd);
        return;
    }

    if (ftruncate(fd, *current_pos + bytes_read) == -1) {
        printf("Failed to truncate file.\n");
        close(fd);
        return;
    } else {
        printf("Deleted %d bytes.\n", delete_count);
    }

    close(fd);
}

// Show content of file
void show_content(const char *filename, int *current_pos) {
    int fd = open_file(filename, O_RDWR);
    if (fd == -1) return;

    char buffer[BUFFER_SIZE];
    // if (lseek(fd, *current_pos, SEEK_SET) == -1) {
    //     printf("Failed to seek.\n");
    //     close(fd);
    //     return;
    // }

    int bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read == -1) {
        printf("Failed to read from file.\n");
    } else {
        buffer[bytes_read] = '\0';
        printf("Content:\n%s\n", buffer);
        *current_pos += bytes_read;
    }

    close(fd);
}

// Set current position
void set_position(int *current_pos) {
    printf("Input your position:\n");
    char pos_str[10];
    if (read(0, pos_str, sizeof(pos_str) - 1) < 0) {
        printf("Failed to read position.\n");
        return;
    }
    for (int i = 0; i < sizeof(pos_str) - 1; i++) {
        printf("%x", pos_str[i]);
    }
    // clear_input_buffer();
    pos_str[sizeof(pos_str) - 1] = '\0';

    int new_pos = string_to_int(pos_str);
    if (new_pos < 0) {
        printf("Invalid position.\n");
    } else {
        *current_pos = new_pos;
        printf("Current position set to %d.\n", *current_pos);
    }
}