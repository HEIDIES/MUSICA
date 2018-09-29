#pragma once
#include <opencv2/opencv.hpp>
#include <memory>
#include "xiaozhi.h"


typedef unsigned short WORD;
typedef unsigned char BYTE;

namespace myMusica {

	class LaplacePyramid;

	class ImageProcessor {

	public:
		ImageProcessor(LaplacePyramid* l);

		~ImageProcessor();

	private:
		void Revoke();

		void Reset();

		std::pair<cv::Point, cv::Point> GetRationalRange(cv::InputArray);

		void Clip(cv::InputArray _src, cv::OutputArray _dst);

		void Mirror(cv::InputArray _src);

		void Flip(cv::InputArray _src);

		void HistEqualization();

		void CannySharpness();

		void UnsharpnessMask();

		void SetMinMaxWindow();

		template<typename ImageType> std::pair<ImageType, ImageType> FindMinMaxGrayScale();

		void CalcComplementaryImage();

		template<typename ImageType>void ComplementaryImage();

		template<typename ImageType> void Rotate(RotateDirection direction);

		void CalcRotate(RotateDirection direction);





		std::unique_ptr<LaplacePyramid> l_ptr;

		friend class LaplacePyramid;

		int maxSize;

		std::vector<void*> imageStatusStack;

		std::vector<int> imageHeightStack;

		std::vector<int> imageWidthStack;

		std::vector<int> imageTypeStack;

		void** statusStackBottom;

		void** statusStackTop;

		int* heightStackBottom;

		int* heightStackTop;

		int* widthStackBottom;

		int* widthStackTop;

		int* typeStackBottom;

		int* typeStackTop;
	};

} // namespace myMusica