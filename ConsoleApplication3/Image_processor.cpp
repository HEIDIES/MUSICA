#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <memory>
#include "Image_processor.h"
#include "Laplace_pyramid.h"
#include "useful_tool.h"

using namespace std;
using namespace cv;

namespace myMusica {

	ImageProcessor::ImageProcessor(LaplacePyramid* l) :l_ptr(l) {

		maxSize = 1 << 10;
		
		imageStatusStack.resize(maxSize);
		imageHeightStack.resize(maxSize);
		imageWidthStack.resize(maxSize);
		imageTypeStack.resize(maxSize);

		statusStackBottom = &imageStatusStack[0];
		statusStackTop = &imageStatusStack[0];

		heightStackBottom = &imageHeightStack[0];
		heightStackTop = &imageHeightStack[0];

		widthStackBottom = &imageWidthStack[0];
		widthStackTop = &imageWidthStack[0];

		typeStackBottom = &imageTypeStack[0];
		typeStackTop = &imageTypeStack[0];

	}

	ImageProcessor::~ImageProcessor() {

		statusStackTop = NULL;
		statusStackBottom = NULL;

		heightStackTop = NULL;
		heightStackBottom = NULL;

		widthStackTop = NULL;
		widthStackBottom = NULL;

		typeStackTop = NULL;
		typeStackBottom = NULL;

	}

	void ImageProcessor::Revoke() {

		int szStatus = static_cast<int>(statusStackTop - statusStackBottom);
		int szHieght = static_cast<int>(heightStackTop - heightStackBottom);
		int szWidth = static_cast<int>(widthStackTop - widthStackBottom);
		int szType = static_cast<int>(typeStackTop - typeStackBottom);

		if (szStatus > 1) {

			assert(szHieght > 1);
			assert(szWidth >  1);
			assert(szType > 1);

			assert(szStatus == szHieght);
			assert(szHieght == szWidth);
			assert(szWidth == szType);

			--statusStackTop;
			free(*statusStackTop);
			heightStackTop--;
			widthStackTop--;
			typeStackTop--;
		}
		else {

			assert(szStatus > 0);
			assert(szHieght > 0);
			assert(szWidth > 0);
			assert(szType > 0);

			assert(szStatus == szHieght);
			assert(szHieght == szWidth);
			assert(szWidth == szType);

			cout << "Already catch the origin image" << endl << endl;

		}

	}

	void ImageProcessor::Reset() {

		int szStatus = static_cast<int>(statusStackTop - statusStackBottom);
		int szHieght = static_cast<int>(heightStackTop - heightStackBottom);
		int szWidth = static_cast<int>(widthStackTop - widthStackBottom);
		int szType = static_cast<int>(typeStackTop - typeStackBottom);

		assert(szStatus > 0);
		assert(szHieght > 0);
		assert(szWidth > 0);
		assert(szType > 0);

		assert(szStatus == szHieght);
		assert(szHieght == szWidth);
		assert(szWidth == szType);

		while (static_cast<int>(statusStackTop - statusStackBottom) > 1) {
			--statusStackTop;
			free(*statusStackTop);
		}

		heightStackTop = heightStackBottom + 1;
		widthStackTop = widthStackBottom + 1;
		typeStackTop = typeStackBottom + 1;

	}

	pair<Point, Point> ImageProcessor::GetRationalRange(InputArray _src) {

		return pair<Point, Point>(Point(337, 316), Point(2148, 1755));

	}

	void ImageProcessor::Clip(InputArray _src, OutputArray _dst) {

		Mat src = _src.getMat();
		CV_Assert(src.type() == CV_16U || src.type() == CV_8U);

		pair<Point, Point> rationalRange = GetRationalRange(src);

		const int x1 = rationalRange.first.x, y1 = rationalRange.first.y, x2 = rationalRange.second.x, y2 = rationalRange.second.y;
		
		_dst.create(Size(x2 - x1, y2 - y1), src.type());
		Mat dst = _dst.getMat();

		for (int i = y1; i < y2; ++i) {

			if (src.type() == CV_16U) {

				WORD* _srcCol = src.ptr<WORD>(i);
				WORD* _dstCol = dst.ptr<WORD>(i - y1);

				for (int j = x1; j < x2; ++j) {

					_dstCol[j - x1] = _srcCol[j];

				}
			}
			else {

				BYTE* _srcCol = src.ptr<BYTE>(i);
				BYTE* _dstCol = dst.ptr<BYTE>(i - y1);

				for (int j = x1; j < x2; ++j) {

					_dstCol[j - x1] = _srcCol[j];

				}
			}
		}

	}

	void ImageProcessor::Mirror(InputArray _src) {

		Mat src = _src.getMat();
		CV_Assert(src.type() == CV_16U || src.type() == CV_8U);
		const int height = src.rows;
		const int width = src.cols;

		const int currentHeight = heightStackTop[-1];
		const int currentWidth = widthStackTop[-1];
		const int currentType = typeStackTop[-1];

		CV_Assert(currentHeight == height);
		CV_Assert(currentWidth == width);
		CV_Assert(currentType == src.type());
		
		if (src.type() == CV_16U) {

			*statusStackTop = (WORD*)calloc(height * width, sizeof(WORD));

			for (int i = 0; i < height; ++i) {

				WORD* _srcCol = src.ptr<WORD>(i);

				for (int j = 0; j < width; ++j) {

					reinterpret_cast<WORD*>(*statusStackTop)[i * width + j] = _srcCol[width - 1 - j];

				}
			}
		}
		else {

			*statusStackTop = (BYTE*)calloc(height * width, sizeof(BYTE));

			for (int i = 0; i < height; ++i) {

				BYTE* _srcCol = src.ptr<BYTE>(i);

				for (int j = 0; j < width; ++j) {

					reinterpret_cast<BYTE*>(*statusStackTop)[i * width + j] = _srcCol[width - 1 - j];

				}
			}
		}
		

		*statusStackTop++;
		*heightStackTop++ = currentHeight;
		*widthStackTop++ = currentWidth;
		*typeStackTop++ = currentType;

	}

	void ImageProcessor::Flip(InputArray _src) {

		Mat src = _src.getMat();
		CV_Assert(src.type() == CV_16U || src.type() == CV_8U);
		const int height = src.rows;
		const int width = src.cols;

		const int currentHeight = heightStackTop[-1];
		const int currentWidth = widthStackTop[-1];
		const int currentType = typeStackTop[-1];

		CV_Assert(currentHeight == height);
		CV_Assert(currentWidth == width);
		CV_Assert(currentType == src.type());

		if (src.type() == CV_16U) {

			*statusStackTop = (WORD*)calloc(height * width, sizeof(WORD));

			for (int i = 0; i < height; ++i) {

				WORD* _srcCol = src.ptr<WORD>(height - i - 1);

				for (int j = 0; j < width; ++j) {

					reinterpret_cast<WORD*>(*statusStackTop)[i * width + j] = _srcCol[j];

				}
			}
		}
		else {

			*statusStackTop = (BYTE*)calloc(height * width, sizeof(BYTE));

			for (int i = 0; i < height; ++i) {
				BYTE* _srcCol = src.ptr<BYTE>(height - i - 1);
				for (int j = 0; j < width; ++j) {

					reinterpret_cast<BYTE*>(*statusStackTop)[i * width + j] = _srcCol[j];

				}
			}
		}

		*statusStackTop++;
		*heightStackTop++ = currentHeight;
		*widthStackTop++ = currentWidth;
		*typeStackTop++ = currentType;
	}

	template<typename ImageType> pair<ImageType, ImageType> ImageProcessor::FindMinMaxGrayScale() {

		ImageType minGrayScale = 65535;
		ImageType maxGrayScale = 0;

		ImageType* currentImage = reinterpret_cast<ImageType*>(statusStackTop[-1]);

		int pixelCount = heightStackTop[-1] * widthStackTop[-1];

		for (size_t i = 0; i < pixelCount; ++i) {

			minGrayScale = minGrayScale < currentImage[i] ? minGrayScale : currentImage[i];
			maxGrayScale = maxGrayScale > currentImage[i] ? maxGrayScale : currentImage[i];

		}

		currentImage = NULL;

		return pair<ImageType, ImageType>(minGrayScale, maxGrayScale);

	}

	void ImageProcessor::SetMinMaxWindow() {

		const int currentType = typeStackTop[-1];
		const int height = heightStackTop[-1];
		const int width = widthStackTop[-1];

		CV_Assert(currentType == CV_16U || currentType == CV_8U);

		if (currentType == CV_16U) {

			*statusStackTop = (BYTE*)calloc(height * width, sizeof(BYTE));

			pair<WORD, WORD> minMaxScalePair = FindMinMaxGrayScale<WORD>();

			for (size_t i = 0; i < height * width; ++i) {

				reinterpret_cast<BYTE*>(*statusStackTop)[i] = saturate_cast<BYTE>((reinterpret_cast<WORD*>(statusStackTop[-1])[i] - minMaxScalePair.first) / static_cast<float>(minMaxScalePair.second) * MAX_BYTE);

			}

		}
		else {

			*statusStackTop = (BYTE*)calloc(height * width, sizeof(BYTE));

			pair<BYTE, BYTE> minMaxScalePair = FindMinMaxGrayScale<BYTE>();

			for (size_t i = 0; i < height * width; ++i) {

				reinterpret_cast<BYTE*>(*statusStackTop)[i] = saturate_cast<BYTE>((reinterpret_cast<BYTE*>(statusStackTop[-1])[i] - minMaxScalePair.first) / static_cast<float>(minMaxScalePair.second) * MAX_BYTE);

			}

		}

		statusStackTop++;
		*heightStackTop++ = height;
		*widthStackTop++ = width;
		*typeStackTop++ = CV_8U;

	}

	void ImageProcessor::CannySharpness() {

		const int currentHeight = heightStackTop[-1];
		const int currentWidth = widthStackTop[-1];
		const int currentType = typeStackTop[-1];

		CV_Assert(currentType == CV_8U);

		Mat src(currentHeight, currentWidth, currentType, statusStackTop[-1]);

		Mat dx(src.rows, src.cols, CV_16S);
		Mat dy(src.rows, src.cols, CV_16S);
		Sobel(src, dx, CV_16S, 1, 0, 3, 1, 0, BORDER_REPLICATE);
		Sobel(src, dy, CV_16S, 0, 1, 3, 1, 0, BORDER_REPLICATE);

		const Size size = src.size();

		const int height = dx.rows, width = dx.cols;

		*statusStackTop = (BYTE*)calloc(currentHeight * currentWidth, sizeof(BYTE));

		for (int i = 0; i < height; ++i) {

			short* _dx = dx.ptr<short>(i);
			short* _dy = dy.ptr<short>(i);
			BYTE* _srcCol = src.ptr<BYTE>(i);

			for (int j = 0; j < width; ++j) {

				reinterpret_cast<BYTE*>(*statusStackTop)[i * width + j] =
					saturate_cast<BYTE>(static_cast<short>(_srcCol[j]) + static_cast<short>(fast_sqrt(static_cast<float>(_dx[j] * _dx[j] + _dy[j] * _dy[j]))));

			}
		}

		statusStackTop++;
		*heightStackTop++ = currentHeight;
		*widthStackTop++ = currentWidth;
		*typeStackTop++ = currentType;

	}

	void ImageProcessor::HistEqualization() {



	}

	void ImageProcessor::UnsharpnessMask() {



	}

	void ImageProcessor::CalcComplementaryImage() {

		const int currentType = typeStackTop[-1];

		CV_Assert(currentType == CV_16U || currentType == CV_8U);

		if (currentType == CV_16U) {

			ComplementaryImage<WORD>();

		}
		else {

			ComplementaryImage<BYTE>();

		}

	}

	template<typename ImageType> void ImageProcessor::ComplementaryImage() {

		const int currentHeight = heightStackTop[-1];
		const int currentWidth = widthStackTop[-1];
		const int currentType = typeStackTop[-1];

		CV_Assert(currentType == CV_16U || currentType == CV_8U);

		ImageType maxGray;

		if (currentType == CV_16U) {

			maxGray = MAX_WORD;

		}
		else {

			maxGray = MAX_BYTE;

		}

		*statusStackTop = (ImageType*)calloc(currentHeight * currentWidth, sizeof(ImageType));

		for (size_t i = 0; i < currentHeight * currentWidth; ++i) {

			reinterpret_cast<ImageType*>(*statusStackTop)[i] = maxGray - reinterpret_cast<ImageType*>(statusStackTop[-1])[i];

		}

		statusStackTop++;
		*heightStackTop++ = currentHeight;
		*widthStackTop++ = currentWidth;
		*typeStackTop++ = currentType;

	}

	template<typename ImageType> void ImageProcessor::Rotate(RotateDirection direction) {

		const int currentHeight = heightStackTop[-1];
		const int currentWidth = widthStackTop[-1];
		const int currentType = typeStackTop[-1];

		CV_Assert(currentType == CV_16U || currentType == CV_8U);

		//Mat currentImage(currentHeight, currentWidth, currentType, statusStackTop[-1]);
		*statusStackTop = (ImageType*)calloc(currentHeight * currentWidth, sizeof(ImageType));
		if (direction == CLOCKWISE) {
			for (size_t i = 0; i < currentHeight; ++i) {

				for (size_t j = 0; j < currentWidth; ++j) {

					reinterpret_cast<ImageType*>(*statusStackTop)[j * currentHeight + currentHeight - 1 - i] =
						reinterpret_cast<ImageType*>(statusStackTop[-1])[i * currentWidth + j];

				}
			}
		}
		else if(direction == COUNTERCLOCKWISE) {

			for (size_t i = 0; i < currentHeight; ++i) {

				for (size_t j = 0; j < currentWidth; ++j) {
					
					reinterpret_cast<ImageType*>(*statusStackTop)[(currentWidth - 1 - j) * currentHeight + i] =
						reinterpret_cast<ImageType*>(statusStackTop[-1])[i * currentWidth + j];

				}
			}

		}

		statusStackTop++;
		*heightStackTop++ = currentWidth;
		*widthStackTop++ = currentHeight;
		*typeStackTop++ = currentType;

	}

	void ImageProcessor::CalcRotate(RotateDirection direction) {

		const int currentType = typeStackTop[-1];

		CV_Assert(currentType == CV_8U || currentType == CV_16U);

		if (currentType == CV_16U) {

			Rotate<WORD>(direction);

		}

		else {

			Rotate<BYTE>(direction);

		}

	}

} // namespace myMusica