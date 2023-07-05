#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

void receiveFilesFromServer(int serverSocket, const char* folderPath) {
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead, bytesWritten;
    FILE* file;

    // Đảm bảo thư mục đích tồn tại
    system("mkdir -p folderPath");

    // Nhận tên tệp từ server
    while (1) {
        bytesRead = recv(serverSocket, buffer, BUFFER_SIZE, 0);
        if (bytesRead <= 0) {
            printf("Error receiving file name.\n");
            break;
        }

        // Kiểm tra kết thúc danh sách
        if (strncmp(buffer, "END", 3) == 0) {
            break;
        }

        // Tạo đường dẫn tới tệp
        char filePath[256];
        snprintf(filePath, sizeof(filePath), "%s/%s", folderPath, buffer);

        // Mở tệp để ghi dữ liệu
        file = fopen(filePath, "wb");
        if (file == NULL) {
            printf("Error opening file.\n");
            continue;
        }

        // Nhận dữ liệu từ server và ghi vào tệp
        while (1) {
            bytesRead = recv(serverSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead <= 0) {
                printf("Error receiving file data.\n");
                break;
            }

            // Kiểm tra kết thúc dữ liệu
            if (strncmp(buffer, "END", 3) == 0) {
                break;
            }

            // Ghi dữ liệu vào tệp
            bytesWritten = fwrite(buffer, 1, bytesRead, file);
            if (bytesWritten < bytesRead) {
                printf("Error writing to file.\n");
                break;
            }
        }

        // Đóng tệp
        fclose(file);
    }
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddress;

    const int port = 12345;
    const char* folderPath = "/path/to/save/files";

    // Tạo socket client
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        printf("Error creating client socket.\n");
        return 1;
    }

    // Thiết lập thông tin địa chỉ server
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    // Kết nối tới server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        printf("Error connecting to server.\n");
        return 1;
    }

    printf("Connected to server.\n");

    // Nhận danh sách các tệp từ server và lưu vào thư mục đích
    receiveFilesFromServer(clientSocket, folderPath);

    // Đóng kết nối
    close(clientSocket);

    printf("Disconnected from server.\n");

    return 0;
}