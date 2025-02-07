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
		const char* customizeId = "10000";

		if (!InitializeClient(stationType, stationName, stationId, operatorId)) {
			std::cerr << "Failed to initialize client connection" << std::endl;
			return 1;
		}

		std::cout << "Connected to server successfully" << std::endl;

		/*SendData(askId, productSeries, applicableProjects, false);

		char buffer[4096];
		int bytesReceived = ReceiveData(buffer, sizeof(buffer));
		std::cout << "Received " << bytesReceived << " bytes: " << buffer << std::endl;*/
		
		MainAppInfo* mainAppInfo = GetMainAppInfo(productSeries, applicableProjects, customizeId);
		if (!mainAppInfo) {
			std::cerr << "Failed to get main app info" << std::endl;
			CloseConnection();
			return 1;
		}
		std::cout << "Main app info:" << std::endl;
		std::cout << "Version: " << mainAppInfo->version << std::endl;
		std::cout << "BL Version: " << mainAppInfo->blVersion << std::endl;
		std::cout << "Calibration Offset: " << mainAppInfo->calibrationOffset << "\n\n" << std::endl;

		DefaultParametersInfo* defaultParaInfo = GetDefaultParametersInfo(productSeries, applicableProjects, customizeId);
		if (!defaultParaInfo) {
			std::cerr << "Failed to get default parameters info" << std::endl;
			CloseConnection();
			return 1;
		}
		std::cout << "Default parameters info:" << std::endl;
		std::cout << "Version: " << defaultParaInfo->version << std::endl;
		std::cout << "BL Version: " << defaultParaInfo->blVersion << std::endl;
		std::cout << "Calibration Offset: " << defaultParaInfo->calibrationOffset << std::endl;
		std::cout << defaultParaInfo->shieldedZone->end << "\n\n" << std::endl;

		std::cout << "Starting file download..." << std::endl;

		// 獲取 .bin 檔案function，
		FileInfo* fileInfo = GetBinFileInfo(askId, productSeries, applicableProjects, customizeId);
		if (!fileInfo) {
			std::cerr << "Failed to get file info" << std::endl;
			CloseConnection();
			return 1;
		}

		std::cout << "\nFile received successfully:" << std::endl;
		std::cout << "Name: " << fileInfo->fileName << std::endl;
		std::cout << "Size: " << fileInfo->size << " bytes" << std::endl;

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
		bool isClose = CloseConnection();

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
}
