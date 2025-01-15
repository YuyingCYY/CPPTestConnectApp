#include <iostream>
#include <fstream>
#include <filesystem>
#include "SocketClient.h"

int main()
{
	try
	{
		const char* stationType = "type";
		const char* stationName = "name";
		const char* stationId = "Id";
		const char* operatorId = "oId";

		const char* askId = "MainApp";
		const char* productSeries = "BMS";
		const char* applicableProjects = "Thai";

		// 初始化並連接Server，回傳 bool 檢查是否連接成功
		if (InitializeClient(stationType, stationName, stationId, operatorId)) {
			char buffer[4096];

			// 獲取 .bin 檔案function，
			FileInfo* fileInfo = GetBinFileInfo(askId, productSeries, applicableProjects);

			std::cout << "File name: " << fileInfo->fileName << std::endl;
			std::cout << "File size: " << fileInfo->size << std::endl;

			std::filesystem::path tempPath = std::filesystem::temp_directory_path() / fileInfo->fileName;
			std::ofstream tempFile(tempPath, std::ios::binary | std::ios::out);
			if (tempFile.is_open()) {
				tempFile.write(fileInfo->data, fileInfo->size);
				tempFile.close();
				std::cout << "Temporary file created: " << tempPath << std::endl;

				// 讀取並檢查檔案內容
				std::ifstream inputFile(tempPath, std::ios::binary | std::ios::in);
				if (inputFile.is_open()) {
					std::vector<char> fileContent(fileInfo->size);
					inputFile.read(fileContent.data(), fileInfo->size);
					inputFile.close();

					// 檢查檔案內容是否正確
					for (size_t i = 0; i < fileInfo->size; i++) {
						if (fileContent[i] != fileInfo->data[i]) {
							std::cout << "File content mismatch at index " << i << std::endl;
							break;
						}
					}
				}
				else {
					std::cout << "Failed to open temporary file." << std::endl;
				}

				// 刪除臨時檔案
				std::filesystem::remove(tempPath);
			}
			else {
				std::cout << "Failed to create temporary file." << std::endl;
			}

			FreeFileInfo(fileInfo);

			CloseConnection();
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception occurred: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown exception occurred." << std::endl;
	}

	return 0;
}
