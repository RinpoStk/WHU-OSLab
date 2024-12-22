#include "stdio.h"
#include "../include/sys/const.h"
#include "type.h"
#include "string.h"

#define BUFFER_SIZE 1024
#define MAX_LINES 1000
#define MAX_COLUMNS 80

char buffer[MAX_LINES][MAX_COLUMNS];
int current_line = 0;
int current_column = 0;
int total_lines = 0;

int string_to_int(const char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    while (str[i]<'0' || str[i]>'9') {
        i++;
    }

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
void write_content(const char *filename);
void delete_content(const char *filename);
void show_content(const char *filename);
void set_position();
void clear_input_buffer();
void display_current_position();
void move_cursor(char direction);
void fill_file_to_position(int fd, int position);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: edit <file>\n");
        return 0;
    }

    char *filename = argv[1];
    char operation;

    while (1) {
        display_current_position();
        printf("\nChoose your operation:\n");
        printf("1. Write\n");
        printf("2. Delete\n");
        printf("3. Show\n");
        printf("4. Choose Position\n");
        printf("5. Exit\n");
        printf("Enter your choice (or use w/a/s/d to move cursor): ");

        if (read(0, &operation, 1) != 1) {
            printf("Failed to read operation.\n");
            continue;
        }
        clear_input_buffer();

        switch (operation) {
            case '1':
                write_content(filename);
                break;
            case '2':
                delete_content(filename);
                break;
            case '3':
                show_content(filename);
                break;
            case '4':
                set_position();
                break;
            case '5':
                printf("Exiting editor.\n");
                return 0;
            case 'w':
            case 'a':
            case 's':
            case 'd':
                move_cursor(operation);
                break;
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
void write_content(const char *filename) {
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

    int position = current_line * MAX_COLUMNS + current_column;
    fill_file_to_position(fd, position);

    if (lseek(fd, position, SEEK_SET) == -1) {
        printf("Failed to seek to position.\n");
        close(fd);
        return;
    }

    int bytes_written = write(fd, content, strlen(content));
    if (bytes_written == -1) {
        printf("Failed to write to file.\n");
    } else {
        printf("%d bytes written.\n", bytes_written);
        current_column += bytes_written;
        if (current_column >= MAX_COLUMNS) {
            current_line++;
            current_column = 0;
        }
    }

    close(fd);
}

// Delete content from file
void delete_content(const char *filename) {
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

    int position = current_line * MAX_COLUMNS + current_column - delete_count;
    if (position < 0) {
        printf("Invalid delete position.\n");
        close(fd);
        return;
    }

    if (lseek(fd, position + delete_count, SEEK_SET) == -1) {
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

    if (lseek(fd, position, SEEK_SET) == -1) {
        printf("Failed to seek.\n");
        close(fd);
        return;
    }

    if (write(fd, buffer, bytes_read) == -1) {
        printf("Failed to delete content.\n");
        close(fd);
        return;
    }

    if (ftruncate(fd, position + bytes_read) == -1) {
        printf("Failed to truncate file.\n");
        close(fd);
        return;
    } else {
        printf("Deleted %d bytes.\n", delete_count);
        current_column -= delete_count;
        if (current_column < 0) {
            current_line--;
            current_column = MAX_COLUMNS + current_column;
        }
    }

    close(fd);
}

// Show content of file
void show_content(const char *filename) {
    int fd = open_file(filename, O_RDWR);
    if (fd == -1) return;

    char buffer[BUFFER_SIZE];
    if (lseek(fd, 0, SEEK_SET) == -1) { // 从文件开始位置读取
        printf("Failed to seek.\n");
        close(fd);
        return;
    }

    int bytes_read;
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }

    printf("\n");

    if (bytes_read == -1) {
        printf("Failed to read from file.\n");
    }

    close(fd);
}

// Set current position
void set_position() {
    printf("Input your line and column (format: line,column):\n");
    char pos_str[20];
    if (read(0, pos_str, sizeof(pos_str) - 1) < 0) {
        printf("Failed to read position.\n");
        return;
    }
    pos_str[sizeof(pos_str) - 1] = '\0';

    char *comma = strchr(pos_str, ',');
    if (comma == NULL) {
        printf("Invalid format. Use line,column.\n");
        return;
    }

    *comma = '\0';
    int new_line = string_to_int(pos_str);
    int new_column = string_to_int(comma + 1);

    if (new_line < 0 || new_column < 0 || new_column >= MAX_COLUMNS) {
        printf("Invalid position.\n");
    } else {
        current_line = new_line;
        current_column = new_column;
        printf("Current position set to line %d, column %d.\n", current_line, current_column);
    }
}

// Display current position
void display_current_position() {
    printf("Current position: line %d, column %d\n", current_line, current_column);
}

// Move cursor
void move_cursor(char direction) {
    switch (direction) {
        case 'a':
            if (current_column > 0) {
                current_column--;
            }
            break;
        case 's':
            if (current_line < MAX_LINES - 1) {
                current_line++;
            }
            break;
        case 'w':
            if (current_line > 0) {
                current_line--;
            }
            break;
        case 'd':
            if (current_column < MAX_COLUMNS - 1) {
                current_column++;
            }
            break;
    }
    display_current_position();
}

// Fill file with spaces up to the specified position
void fill_file_to_position(int fd, int position) {
    int file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        printf("Failed to get file size.\n");
        return;
    }

    if (position > file_size) {
        char spaces[BUFFER_SIZE];
        memset(spaces, ' ', BUFFER_SIZE);

        while (file_size < position) {
            int chunk_size = (position - file_size) < BUFFER_SIZE ? (position - file_size) : BUFFER_SIZE;
            if (write(fd, spaces, chunk_size) == -1) {
                printf("Failed to write spaces to file.\n");
                return;
            }
            file_size += chunk_size;
        }
    }
}