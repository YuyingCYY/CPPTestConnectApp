#include <iostream>
#include <fstream>
#include <filesystem>
#include "SocketClient.h"

void OnProgress(int percentage) {
    // 使用 \r 來覆蓋同一行，製造進度條效果
    std::cout << "\rDownload progress: [";
    int pos = percentage / 2;
    for (int i = 0; i < 50; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << percentage << "%" << std::flush;

    if (percentage == 100) {
        std::cout << std::endl;  // 完成時換行
    }
}

void OnError(const char* error) {
    std::cerr << "Error occurred: " << error << std::endl;
}

int main()
{
    try {
        const char* stationType = "type";
        const char* stationName = "name";
        const char* stationId = "Id";
        const char* operatorId = "oId";
        const char* askId = "MainApp";
        const char* productSeries = "BMS";
        const char* applicableProjects = "Thai";

        // 設置回調函數
        TransferCallbacks callbacks = {
            .onProgress = OnProgress,
            .onError = OnError
        };

        // 初始化連接
        if (!InitializeClient(stationType, stationName, stationId, operatorId)) {
            std::cerr << "Failed to initialize client connection" << std::endl;
            return 1;
        }

        std::cout << "Connected to server successfully" << std::endl;
        std::cout << "Starting file download..." << std::endl;

        // 獲取檔案，傳入回調函數
        FileInfo* fileInfo = GetBinFileInfo(askId, productSeries, applicableProjects, &callbacks);
        if (!fileInfo) {
            std::cerr << "Failed to get file info" << std::endl;
            CloseConnection();
            return 1;
        }

        std::cout << "\nFile received successfully:" << std::endl;
        std::cout << "Name: " << fileInfo->fileName << std::endl;
        std::cout << "Size: " << fileInfo->size << " bytes" << std::endl;

        // 保存檔案
        std::filesystem::path tempPath = std::filesystem::temp_directory_path() / fileInfo->fileName;
        {
            std::ofstream tempFile(tempPath, std::ios::binary);
            if (!tempFile) {
                std::cerr << "Failed to create temporary file: " << tempPath << std::endl;
                FreeFileInfo(fileInfo);
                CloseConnection();
                return 1;
            }

            std::cout << "Saving file to: " << tempPath << std::endl;
            tempFile.write(fileInfo->data, fileInfo->size);
            tempFile.close();
        }

        // 驗證檔案完整性
        std::cout << "Verifying file integrity..." << std::endl;
        bool verificationSuccessful = true;
        {
            std::ifstream inputFile(tempPath, std::ios::binary);
            if (!inputFile) {
                std::cerr << "Failed to open file for verification" << std::endl;
                verificationSuccessful = false;
            }
            else {
                std::vector<char> fileContent(fileInfo->size);
                inputFile.read(fileContent.data(), fileInfo->size);

                if (inputFile.gcount() != fileInfo->size) {
                    std::cerr << "File size mismatch during verification" << std::endl;
                    verificationSuccessful = false;
                }
                else {
                    for (size_t i = 0; i < fileInfo->size; i++) {
                        if (fileContent[i] != fileInfo->data[i]) {
                            std::cerr << "Content mismatch at position " << i << std::endl;
                            verificationSuccessful = false;
                            break;
                        }
                    }
                }
            }
        }

        // 清理
        if (verificationSuccessful) {
            std::cout << "File verification successful" << std::endl;
        }

        std::cout << "Cleaning up..." << std::endl;
        std::filesystem::remove(tempPath);
        FreeFileInfo(fileInfo);
        CloseConnection();

        std::cout << "Operation completed successfully" << std::endl;
        return 0;
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
    return 0;
}
