#include "stdafx.h"
#include "raw_read.h"
#include <iostream>

namespace myMusica {

	using namespace cv;
	using namespace std;

	void RawRead(const char* filename, OutputArray _dst, const int& height, const int& width) {
		FILE *fp;
		if (fopen_s(&fp, filename, "rb")) {
			throw ErrorCode::ERROR_FILE_OPEN_FAILED;
		}

		if (fp == NULL) {
			cout << endl << "error on open raw file." << endl << endl;
			system("pause");
			return ;
		}

		BYTE* rawData = (BYTE*)calloc(height * width * 2, sizeof(BYTE));

		fread(rawData, sizeof(BYTE), height * width * 2, fp);

		Mat(height, width, CV_16UC1, rawData).copyTo(_dst);

		fclose(fp);
		free(rawData);
	}

} // namespace myMusica