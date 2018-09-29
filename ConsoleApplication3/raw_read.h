#pragma once

#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include "xiaozhi.h"


typedef unsigned char BYTE;
typedef unsigned short WORD;

namespace myMusica {

	void RawRead(const char* filename, cv::OutputArray _dst, const int& height, const int& width);

	// void RawReadWithHead(char* filename, cv::OutputArray _dst);

} // namespace myMusica