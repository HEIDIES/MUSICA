#include "stdafx.h"
#include "Laplace_pyramid.h"

using namespace cv;
using namespace std;

namespace myMusica {

	LaplacePyramid::LaplacePyramid(InputArray _src, const int& depth, const int& kernel_size)
		:IP_ptr(new ImageProcessor(this)) {

		Mat src;

		IP_ptr->Clip(_src, src);

		originType = src.type();
		CV_Assert(CV_MAT_DEPTH(originType) == CV_16U || CV_MAT_DEPTH(originType) == CV_8U);
		CV_Assert(CV_MAT_CN(originType) == 1);
		currentType = originType;

		this->kernel_size = kernel_size;

		this->depth = depth;

		originHeight = src.rows;
		originWidth = src.cols;

		currentHeight = originHeight;
		currentWidth = originWidth;

		if (originType == CV_16U)
		{
			originImage = (WORD*)calloc(originHeight * originWidth, sizeof(WORD));
			*IP_ptr->statusStackTop = (WORD*)calloc(originHeight * originWidth, sizeof(WORD));
			for (int i = 0; i < originHeight; ++i) {

				WORD* _srcImg = src.ptr<WORD>(i);

				for (int j = 0; j < originWidth; ++j) {

					reinterpret_cast<WORD*>(originImage)[i * originWidth + j] = _srcImg[j];
					reinterpret_cast<WORD*>(*IP_ptr->statusStackTop)[i * currentWidth + j] = _srcImg[j];
				}
			}
		}
		else {
			originImage = (BYTE*)calloc(originHeight * originWidth, sizeof(BYTE));
			*IP_ptr->statusStackTop = (BYTE*)calloc(originHeight * originWidth, sizeof(BYTE));
			for (int i = 0; i < originHeight; ++i) {

				BYTE* _srcImg = src.ptr<BYTE>(i);

				for (int j = 0; j < originWidth; ++j) {

					reinterpret_cast<BYTE*>(originImage)[i * originWidth + j] = _srcImg[j];
					reinterpret_cast<BYTE*>(*IP_ptr->statusStackTop)[i * currentWidth + j] = _srcImg[j];
				}
			}
		}
		*IP_ptr->heightStackTop++ = originHeight;
		*IP_ptr->widthStackTop++ = originWidth;
		*IP_ptr->typeStackTop++ = originType;
		*IP_ptr->statusStackTop++;


		for (int i = 0; i < 10; ++i) {
			detailImages[i] = NULL;
			approximateImages[i] = NULL;
		}

		enhencedImage = NULL;
		tmpPImage = NULL;

		BuildLaplacePyramid();
	}

	LaplacePyramid::~LaplacePyramid() {
		for (int i = 0; i < 10; ++i) {
			if (detailImages[i] != NULL) {
				free(detailImages[i]);
				detailImages[i] = NULL;
			}

			if (approximateImages[i] != NULL) {
				free(approximateImages[i]);
				approximateImages[i] = NULL;
			}

			if (CNRmap[i] != NULL) {
				free(CNRmap[i]);
				CNRmap[i] = NULL;
			}

		}

		if (originImage != NULL) {
			free(originImage);
			originImage = NULL;
		}

		if (currentImage != NULL) {
			free(currentImage);
			currentImage = NULL;
		}

		if (enhencedImage != NULL) {
			free(enhencedImage);
			enhencedImage = NULL;
		}

		if (tmpPImage != NULL) {
			free(tmpPImage);
			tmpPImage = NULL;
		}
	}

	void LaplacePyramid::BuildLaplacePyramid() {

		for (int i = 0; i < depth; ++i) {
			SetDetailApproximateImage(i);
			if (i < 4) {
				CalcCNRMap(i);

			}

		}

	}

	void LaplacePyramid::SetDetailApproximateImage(const int& idx) {

		if (idx == 0) {
			Mat approImage;
			Mat originImage(originHeight, originWidth, originType, this->originImage);
			GaussianBlur(originImage, approImage, Size(kernel_size, kernel_size), 0, 0, BORDER_REFLECT);


			if (originHeight & 0x1)
				height[0] = (originHeight + 1) >> 1;
			else
				height[0] = originHeight >> 1;

			if (originWidth & 0x1)
				width[0] = (originWidth + 1) >> 1;
			else
				width[0] = originWidth >> 1;


			Mat approImageDown(Size(width[0], height[0]), approImage.type());
			for (int i = 0; i < height[0]; ++i) {
				WORD* _approImageDown = approImageDown.ptr<WORD>(i);
				WORD* _approImage = approImage.ptr<WORD>(2 * i);
				for (int j = 0; j < width[0]; ++j) {
					_approImageDown[j] = _approImage[2 * j];
				}
			}



			approximateImages[0] = (WORD*)calloc(height[idx] * width[idx], sizeof(WORD));
			for (int i = 0; i < height[idx]; ++i) {
				WORD* _approImageDown = approImageDown.ptr<WORD>(i);
				for (int j = 0; j < width[idx]; ++j) {
					reinterpret_cast<WORD*>(approximateImages[0])[i * width[idx] + j] = _approImageDown[j];
				}
			}
			Mat approDown(height[idx], width[idx], CV_16U, approximateImages[0]);



			for (int i = 0; i < originHeight; ++i) {
				WORD* _approImage = approImage.ptr<WORD>(i);
				if (i & 0x1) {
					memset(_approImage, 0, originWidth * sizeof(WORD));
				}
				else {
					for (int j = 0; j < originWidth; ++j) {
						if (j & 0x1)
							_approImage[j] = 0;
					}
				}
			}

			GaussianBlur(approImage, approImage, Size(kernel_size, kernel_size), 0, 0, BORDER_REFLECT);

			approImage = approImage * 4;


			Mat tmp(originHeight, originWidth, CV_32S);
			for (int i = 0; i < originHeight; ++i) {
				int* _tmp = tmp.ptr<int>(i);
				WORD* _approImage = approImage.ptr<WORD>(i);
				WORD* _originImage = originImage.ptr<WORD>(i);
				for (int j = 0; j < originWidth; ++j) {
					_tmp[j] = saturate_cast<int>(_originImage[j]) - saturate_cast<int>(_approImage[j]);
				}
			}

			detailImages[0] = (int*)calloc(originHeight * originWidth, sizeof(int));
			for (int i = 0; i < originHeight; ++i) {
				int* _tmp = tmp.ptr<int>(i);
				for (int j = 0; j < originWidth; ++j) {
					reinterpret_cast<int*>(detailImages[0])[i * originWidth + j] = _tmp[j];
				}
			}

		}
		else {

			Mat origin(height[idx - 1], width[idx - 1], CV_16U, approximateImages[idx - 1]);

			Mat approImage;
			GaussianBlur(origin, approImage, Size(kernel_size, kernel_size), 0, 0, BORDER_REFLECT);

			if (idx < 4) {
				if (height[idx - 1] & 0x1)
					height[idx] = (height[idx - 1] + 1) >> 1;
				else
					height[idx] = height[idx - 1] >> 1;

				if (width[idx - 1] & 0x1)
					width[idx] = (width[idx - 1] + 1) >> 1;
				else
					width[idx] = width[idx - 1] >> 1;


				Mat approImageDown(Size(width[idx], height[idx]), approImage.type());
				for (int i = 0; i < height[idx]; ++i) {
					WORD* _approImageDown = approImageDown.ptr<WORD>(i);
					WORD* _approImage = approImage.ptr<WORD>(2 * i);
					for (int j = 0; j < width[idx]; ++j) {
						_approImageDown[j] = _approImage[2 * j];
					}
				}

				approximateImages[idx] = (WORD*)calloc(height[idx] * width[idx], sizeof(WORD));
				for (int i = 0; i < height[idx]; ++i) {
					WORD* _approImageDown = approImageDown.ptr<WORD>(i);
					for (int j = 0; j < width[idx]; ++j) {
						reinterpret_cast<WORD*>(approximateImages[idx])[i * width[idx] + j] = _approImageDown[j];
					}
				}

				for (int i = 0; i < height[idx - 1]; ++i) {
					WORD* _approImage = approImage.ptr<WORD>(i);
					if (i & 0x1) {
						memset(_approImage, 0, width[idx - 1] * sizeof(WORD));
					}
					else {
						for (int j = 0; j < width[idx - 1]; ++j) {
							if (j & 0x1)
								_approImage[j] = 0;
						}
					}
				}

				GaussianBlur(approImage, approImage, Size(kernel_size, kernel_size), 0, 0, BORDER_REFLECT);
				approImage = approImage * 4;

				if (idx == depth - 1) {
					tmpPImage = (WORD*)calloc(height[idx - 1] * width[idx - 1], sizeof(WORD));
					for (int i = 0; i < height[idx - 1]; ++i) {

						WORD* _approImage = approImage.ptr<WORD>(i);

						for (int j = 0; j < width[idx - 1]; ++j) {
							reinterpret_cast<WORD*>(tmpPImage)[i * width[idx - 1] + j] = _approImage[j];
						}

					}
				}

				Mat tmp(height[idx - 1], width[idx - 1], CV_32S);
				for (int i = 0; i < height[idx - 1]; ++i) {
					int* _tmp = tmp.ptr<int>(i);
					WORD* _approImage = approImage.ptr<WORD>(i);
					WORD* _origin = origin.ptr<WORD>(i);
					for (int j = 0; j < width[idx - 1]; ++j) {
						_tmp[j] = saturate_cast<int>(_origin[j]) - saturate_cast<int>(_approImage[j]);
					}
				}

				detailImages[idx] = (int*)calloc(height[idx - 1] * width[idx - 1], sizeof(int));
				for (int i = 0; i < height[idx - 1]; ++i) {
					int* _tmp = tmp.ptr<int>(i);
					for (int j = 0; j < width[idx - 1]; ++j) {
						reinterpret_cast<int*>(detailImages[idx])[i * width[idx - 1] + j] = _tmp[j];
					}
				}
			}
			else {
				GaussianBlur(approImage, approImage, Size(kernel_size, kernel_size), 0, 0, BORDER_REFLECT);

				height[idx] = height[idx - 1];
				width[idx] = width[idx - 1];

				detailImages[idx] = (int*)calloc(height[idx - 1] * width[idx - 1], sizeof(int));
				approximateImages[idx] = (WORD*)calloc(height[idx] * width[idx], sizeof(WORD));

				for (size_t i = 0; i < height[idx]; ++i) {
					WORD* _approImage = approImage.ptr<WORD>(i);
					WORD* _origin = origin.ptr<WORD>(i);

					for (size_t j = 0; j < width[idx]; ++j) {
						reinterpret_cast<WORD*>(approximateImages[idx])[i * width[idx] + j] = _approImage[j];

						reinterpret_cast<int*>(detailImages[idx])[i * width[idx - 1] + j] = saturate_cast<int>(_origin[j]) -
							saturate_cast<int>(_approImage[j]);
					}
				}
			}
		}
	}

	void LaplacePyramid::GetApproximateImage(const int& idx, OutputArray _dst) {

		Mat(height[idx - 1], width[idx - 1], CV_16U, approximateImages[idx - 1]).copyTo(_dst);

	}

	void LaplacePyramid::GetDetailImage(const int& idx, OutputArray _dst) {

		if (idx == 1) {
			Mat(originHeight, originWidth, CV_16S, detailImages[idx - 1]).copyTo(_dst);
		}
		else {
			Mat(height[idx - 2], width[idx - 2], CV_16S, detailImages[idx - 1]).copyTo(_dst);
		}

	}

	void LaplacePyramid::Reconstruction() {

		for (int i = depth - 1; i >= 0; --i) {
			NonLinearConversion(i);
			ReconstructOneLayer(i);

		}

	}

	void LaplacePyramid::ReconstructOneLayer(const int& idx) {
		if (idx > 1) {
			Mat currentReconstructLayer(height[idx - 1], width[idx - 1], CV_16U, tmpPImage);


			Mat currentDetailImage(height[idx - 1], width[idx - 1], CV_32S, detailImages[idx]);

			CV_Assert(currentDetailImage.rows == currentReconstructLayer.rows);
			CV_Assert(currentDetailImage.cols == currentReconstructLayer.cols);

			for (int i = 0; i < height[idx - 1]; ++i) {

				WORD* _currentReconstructLayer = currentReconstructLayer.ptr<WORD>(i);

				int* _currentDetailImage = currentDetailImage.ptr<int>(i);

				for (int j = 0; j < width[idx - 1]; ++j) {

					_currentReconstructLayer[j] = saturate_cast<WORD>(_currentReconstructLayer[j] + _currentDetailImage[j]);

				}

			}

			Mat currentReconstructLayerUp(height[idx - 2], width[idx - 2], CV_16U, Scalar(0));
			for (int i = 0; i < height[idx - 1]; ++i) {

				WORD* _currentReconstructLayerUp = currentReconstructLayerUp.ptr<WORD>(2 * i);
				WORD* _currentReconstructLayer = currentReconstructLayer.ptr<WORD>(i);

				for (int j = 0; j < width[idx - 1]; ++j) {
					_currentReconstructLayerUp[2 * j] = _currentReconstructLayer[j];
				}
			}

			GaussianBlur(currentReconstructLayerUp, currentReconstructLayerUp, Size(kernel_size, kernel_size), 0, 0, BORDER_REFLECT);
			currentReconstructLayerUp = 4 * currentReconstructLayerUp;

			free(tmpPImage);
			tmpPImage = (WORD*)calloc(height[idx - 2] * width[idx - 2], sizeof(WORD));

			for (int i = 0; i < height[idx - 2]; ++i) {
				WORD* _currentReconstructLayerUp = currentReconstructLayerUp.ptr<WORD>(i);
				for (int j = 0; j < width[idx - 2]; ++j) {
					reinterpret_cast<WORD*>(tmpPImage)[i * width[idx - 2] + j] = _currentReconstructLayerUp[j];
				}
			}
		}
		else if (idx == 1) {
			Mat currentReconstructLayer(height[idx - 1], width[idx - 1], CV_16U, tmpPImage);

			Mat currentDetailImage(height[idx - 1], width[idx - 1], CV_32S, detailImages[idx]);

			CV_Assert(currentDetailImage.rows == currentReconstructLayer.rows);
			CV_Assert(currentDetailImage.cols == currentReconstructLayer.cols);


			for (int i = 0; i < height[idx - 1]; ++i) {

				WORD* _currentReconstructLayer = currentReconstructLayer.ptr<WORD>(i);

				int* _currentDetailImage = currentDetailImage.ptr<int>(i);

				for (int j = 0; j < width[idx - 1]; ++j) {

					_currentReconstructLayer[j] = saturate_cast<WORD>(_currentReconstructLayer[j] + _currentDetailImage[j]);

				}

			}

			Mat currentReconstructLayerUp(originHeight, originWidth, CV_16U, Scalar(0));
			for (int i = 0; i < height[idx - 1]; ++i) {

				WORD* _currentReconstructLayerUp = currentReconstructLayerUp.ptr<WORD>(2 * i);
				WORD* _currentReconstructLayer = currentReconstructLayer.ptr<WORD>(i);

				for (int j = 0; j < width[idx - 1]; ++j) {
					_currentReconstructLayerUp[2 * j] = _currentReconstructLayer[j];
				}
			}

			GaussianBlur(currentReconstructLayerUp, currentReconstructLayerUp, Size(kernel_size, kernel_size), 0, 0, BORDER_REFLECT);
			currentReconstructLayerUp = 4 * currentReconstructLayerUp;

			free(tmpPImage);
			tmpPImage = (WORD*)calloc(originHeight * originWidth, sizeof(WORD));

			for (int i = 0; i < originHeight; ++i) {
				WORD* _currentReconstructLayerUp = currentReconstructLayerUp.ptr<WORD>(i);
				for (int j = 0; j < originWidth; ++j) {
					reinterpret_cast<WORD*>(tmpPImage)[i * originWidth + j] = _currentReconstructLayerUp[j];
				}
			}

		}
		else {
			Mat currentReconstructLayer(originHeight, originWidth, CV_16U, tmpPImage);
			Mat currentDetailImage(originHeight, originWidth, CV_32S, detailImages[idx]);

			CV_Assert(currentDetailImage.rows == currentReconstructLayer.rows);
			CV_Assert(currentDetailImage.cols == currentReconstructLayer.cols);

			for (int i = 0; i < originHeight; ++i) {

				WORD* _currentReconstructLayer = currentReconstructLayer.ptr<WORD>(i);

				int* _currentDetailImage = currentDetailImage.ptr<int>(i);

				for (int j = 0; j < originWidth; ++j) {

					_currentReconstructLayer[j] = saturate_cast<WORD>(_currentReconstructLayer[j] + _currentDetailImage[j]);

				}

			}

			enhencedImage = (WORD*)calloc(originHeight * originWidth, sizeof(WORD));
			for (int i = 0; i < originHeight; ++i) {
				WORD* _currentReconstructLayer = currentReconstructLayer.ptr<WORD>(i);

				for (int j = 0; j < originWidth; ++j) {
					reinterpret_cast<WORD*>(enhencedImage)[i * originWidth + j] = _currentReconstructLayer[j];
				}
			}

			*IP_ptr->statusStackTop = (WORD*)calloc(originHeight * originWidth, sizeof(WORD));
			if (memcpy_s(*IP_ptr->statusStackTop++, originHeight * originWidth * sizeof(WORD), enhencedImage, originHeight * originWidth * sizeof(WORD))) {
				throw ErrorCode::ERROR_MEMERORY_COPY_FAILED;
			}

			*IP_ptr->heightStackTop++ = originHeight;
			*IP_ptr->widthStackTop++ = originWidth;
			*IP_ptr->typeStackTop++ = originType;

		}
	}

	void LaplacePyramid::GetEnhencedImage(OutputArray _dst) {

		Reconstruction();

		GetCurrentImage(_dst);

	}

	void LaplacePyramid::GetOriginHeight(int& height) {

		height = originHeight;

	}

	void LaplacePyramid::GetOriginWidth(int& width) {

		width = originWidth;

	}

	void LaplacePyramid::GetOriginSize(Size& size) {

		size = Size(originWidth, originHeight);

	}

	void LaplacePyramid::GetOriginType(int& type) {

		type = originType;

	}

	void LaplacePyramid::GetCurrentImage(OutputArray _dst) {

		int szStatus = static_cast<int>(IP_ptr->statusStackTop - IP_ptr->statusStackBottom);
		int szHieght = static_cast<int>(IP_ptr->heightStackTop - IP_ptr->heightStackBottom);
		int szWidth = static_cast<int>(IP_ptr->widthStackTop - IP_ptr->widthStackBottom);
		int szType = static_cast<int>(IP_ptr->typeStackTop - IP_ptr->typeStackBottom);

		assert(szStatus > 0);
		assert(szHieght > 0);
		assert(szWidth > 0);
		assert(szType > 0);

		assert(szStatus == szHieght);
		assert(szHieght == szWidth);
		assert(szWidth == szType);

		Mat(IP_ptr->heightStackTop[-1], IP_ptr->widthStackTop[-1], IP_ptr->typeStackTop[-1], IP_ptr->statusStackTop[-1]).copyTo(_dst);

	}

	void LaplacePyramid::GetOriginImage(OutputArray _dst) {

		Mat(originHeight, originWidth, originType, originImage).copyTo(_dst);

	}

	void LaplacePyramid::GetMirroredImage(OutputArray _dst) {

		Mat currentSrc;
		GetCurrentImage(currentSrc);

		IP_ptr->Mirror(currentSrc);

		GetCurrentImage(_dst);

	}

	void LaplacePyramid::GetFlipedImage(OutputArray _dst) {

		Mat currentSrc;
		GetCurrentImage(currentSrc);

		IP_ptr->Flip(currentSrc);

		GetCurrentImage(_dst);

	}

	void LaplacePyramid::Get8BitImage(OutputArray _dst) {

		IP_ptr->SetMinMaxWindow();

		GetCurrentImage(_dst);

	}

	void LaplacePyramid::Revoke(OutputArray _dst) {

		IP_ptr->Revoke();

		GetCurrentImage(_dst);

	}

	void LaplacePyramid::Reset(OutputArray _dst) {

		IP_ptr->Reset();

		GetCurrentImage(_dst);

	}

	void LaplacePyramid::GetComplementaryImage(OutputArray _dst) {

		IP_ptr->CalcComplementaryImage();

		GetCurrentImage(_dst);

	}

	void LaplacePyramid::RotateImage(RotateDirection direction) {

		IP_ptr->CalcRotate(direction);

	}

	void LaplacePyramid::GetRotateClockwiseImage(OutputArray _dst) {

		RotateImage(CLOCKWISE);

		GetCurrentImage(_dst);

	}

	void LaplacePyramid::GetRotateCounterclockwiseImage(OutputArray _dst) {

		RotateImage(COUNTERCLOCKWISE);

		GetCurrentImage(_dst);

	}

	void LaplacePyramid::GetCannySharpness(OutputArray _dst) {

		IP_ptr->CannySharpness();

		GetCurrentImage(_dst);

	}

	void LaplacePyramid::NonLinearConversion(const int& idx) {

		if (idx >= 4) {
			for (size_t i = 0; i < height[idx - 1] * width[idx - 1]; ++i) {
				int tmpVal = reinterpret_cast<int*>(detailImages[idx])[i];
				int bias = 3 * static_cast<int>(noise_level[idx]);
				if (tmpVal > bias) {
					reinterpret_cast<int*>(detailImages[idx])[i] = static_cast<int>((tmpVal + bias) / 2.0);

				}
				else if (tmpVal < -bias) {
					reinterpret_cast<int*>(detailImages[idx])[i] = static_cast<int>((tmpVal - bias) / 2.0);

				}

			}

		}

	}

	void LaplacePyramid::CalcNoiseLevel(const int& idx) {

		if (idx > 10) {

			cout << "Only calculate the noise level for layer 1-10. " << endl;
			return;

		}

		int* cnr_counting;

		if (idx == 0) {
			float noise_count = -1.0;

			
			int maxVal = 0;

			for (int i = 0; i < originHeight * originWidth; ++i) {
				int tmpVal = cvRound(reinterpret_cast<float*>(CNRmap[idx])[i]);

				maxVal = tmpVal > maxVal ? tmpVal : maxVal;

			}
			cnr_counting = (int*)calloc(maxVal + 1, sizeof(int));
			

			//map<int, int> cnr_counting;
			for (int i = 0; i < originHeight * originWidth; ++i) {
				int tmpVal = cvRound(reinterpret_cast<float*>(CNRmap[idx])[i]);
					
				cnr_counting[tmpVal]++;

				
				int tmpCount = cnr_counting[tmpVal];
				noise_level[idx] = tmpCount > noise_count ? tmpVal : noise_level[idx];
				noise_count = tmpCount > noise_count ? tmpCount : noise_count;

			}
			free(cnr_counting);
			cnr_counting = NULL;
			

		}
		else {
			int noise_count = -1;

			
			int maxVal = 0;

			for (int i = 0; i < height[idx - 1] * width[idx - 1]; ++i) {
				int tmpVal = cvRound(reinterpret_cast<float*>(CNRmap[idx])[i]);

				maxVal = tmpVal > maxVal ? tmpVal : maxVal;

			}
			cnr_counting = (int*)calloc(maxVal + 1, sizeof(int));
			

			//map<int, int> cnr_counting;
			for (int i = 0; i < height[idx - 1] * width[idx - 1]; ++i) {
				int tmpVal = cvRound(reinterpret_cast<float*>(CNRmap[idx])[i]);

				cnr_counting[tmpVal]++;

				int tmpCount = cnr_counting[tmpVal];
				noise_level[idx] = tmpCount > noise_count ? tmpVal : noise_level[idx];
				noise_count = tmpCount > noise_count ? tmpCount : noise_count;

			}
			free(cnr_counting);
			cnr_counting = NULL;

		}

	}

	void LaplacePyramid::CalcCNRMap(const int& idx) {

		if (idx > 10) {

			cout << "Only calculate the CNR map for layer 1-10. " << endl;
			return;

		}

		if (idx == 0) {

			ptrdiff_t mapstep = originWidth + 4;
			AutoBuffer<int> detailBuffer(5 * mapstep * sizeof(int));

			int* detailCol[5];
			detailCol[0] = (int*)detailBuffer + 2;
			detailCol[1] = detailCol[0] + mapstep;
			detailCol[2] = detailCol[1] + mapstep;
			detailCol[3] = detailCol[2] + mapstep;
			detailCol[4] = detailCol[3] + mapstep;

			memset(detailCol[0] - 2, 0, mapstep * sizeof(int));
			memset(detailCol[1] - 2, 0, mapstep * sizeof(int));

			detailCol[2][-1] = detailCol[3][-1] = detailCol[4][-1] = 0;
			detailCol[2][-2] = detailCol[3][-2] = detailCol[4][-2] = 0;

			detailCol[2][originWidth] = detailCol[3][originWidth] = detailCol[4][originWidth] = 0;
			detailCol[2][originWidth + 1] = detailCol[3][originWidth + 1] = detailCol[4][originWidth + 1] = 0;

			if (memcpy_s(detailCol[2], originWidth * sizeof(int), reinterpret_cast<int*>(detailImages[idx]), originWidth * sizeof(int))
				|| memcpy_s(detailCol[3], originWidth * sizeof(int), reinterpret_cast<int*>(detailImages[idx]) + originWidth, originWidth * sizeof(int))) {

				throw ErrorCode::ERROR_MEMERORY_COPY_FAILED;

			}
			if (memcpy_s(detailCol[4], originWidth * sizeof(int), reinterpret_cast<int*>(detailImages[idx]) + 2 * originWidth, originWidth * sizeof(int))) {
				throw ErrorCode::ERROR_MEMERORY_COPY_FAILED;

			}


			CNRmap[idx] = (float*)calloc(originHeight * originWidth, sizeof(float));

			for (int i = 0; i < originHeight; ++i) {
				float* _CNRmap = reinterpret_cast<float*>(CNRmap[idx]) + i * originWidth;
				int tmpVal = 0;
				for (int m = 0; m < 5; ++m) {
					for (int n = -2; n < 3; ++n) {
						tmpVal += detailCol[m][n] * detailCol[m][n];
					}
				}

				for (int j = 0; j < originWidth; ++j) {
					_CNRmap[j] = fast_sqrt(static_cast<float>(tmpVal) / 25.0);
					for (int m = 0; m < 5; ++m) {
						tmpVal += (detailCol[m][j + 3] * detailCol[m][j + 3] - detailCol[m][j - 2] * detailCol[m][j - 2]);
					}

				}
				detailCol[0] = detailCol[1];
				detailCol[1] = detailCol[2];
				detailCol[2] = detailCol[3];
				detailCol[3] = detailCol[4];
				if (i + 3 < originHeight) {
					
					if (memcpy_s(detailCol[4], originWidth * sizeof(int), reinterpret_cast<int*>(detailImages[idx]) + (i + 3) * originWidth, originWidth * sizeof(int))) {
						throw ErrorCode::ERROR_MEMERORY_COPY_FAILED;

					}
				}
				else {
					memset(detailCol[4], 0, originWidth * sizeof(int));

				}

			}
			CalcNoiseLevel(idx);

			for (int i = 0; i < originHeight * originWidth; ++i) {
				reinterpret_cast<float*>(CNRmap[idx])[i] /= noise_level[idx];

			}

		}
		else {
			ptrdiff_t mapstep = width[idx - 1] + 2;
			AutoBuffer<int> detailBuffer(3 * mapstep * sizeof(int));

			int* detailCol[3];
			detailCol[0] = (int*)detailBuffer + 1;
			detailCol[1] = detailCol[0] + mapstep;
			detailCol[2] = detailCol[1] + mapstep;

			memset(detailCol[0] - 1, 0, mapstep * sizeof(int));

			detailCol[1][-1] = detailCol[2][-1] = 0;
			detailCol[1][width[idx - 1]] = detailCol[1][width[idx - 1]] = 0;

			if (memcpy_s(detailCol[1], width[idx - 1] * sizeof(int), reinterpret_cast<int*>(detailImages[idx]), width[idx - 1] * sizeof(int))
				|| memcpy_s(detailCol[2], width[idx - 1] * sizeof(int), reinterpret_cast<int*>(detailImages[idx]) + width[idx - 1], width[idx - 1] * sizeof(int))) {
				throw ErrorCode::ERROR_MEMERORY_COPY_FAILED;

			}

			CNRmap[idx] = (float*)calloc(height[idx - 1] * width[idx - 1], sizeof(float));

			for (int i = 0; i < height[idx - 1]; ++i) {
				float* _CNRmap = reinterpret_cast<float*>(CNRmap[idx]) + i * width[idx - 1];
				int tmpVal = 0;
				for (int m = 0; m < 3; ++m) {
					for (int n = -1; n < 2; ++n) {
						tmpVal += detailCol[m][n] * detailCol[m][n];

					}

				}

				for (int j = 0; j < width[idx - 1]; ++j) {
					_CNRmap[j] = fast_sqrt(static_cast<float>(tmpVal) / 9.0);
					for (int m = 0; m < 3; ++m) {
						tmpVal += (detailCol[m][j + 2] - detailCol[m][j - 1]);

					}

				}

				detailCol[0] = detailCol[1];
				detailCol[1] = detailCol[2];
				if (i + 2 < height[idx - 1]) {
					if (memcpy_s(detailCol[2], width[idx - 1] * sizeof(int), reinterpret_cast<int*>(detailImages[idx]) + (i + 2) * width[idx - 1], width[idx - 1] * sizeof(int))) {
						throw ErrorCode::ERROR_MEMERORY_COPY_FAILED;

					}

				}
				else {
					memset(detailCol[2], 0, width[idx - 1] * sizeof(int));

				}

			}
			CalcNoiseLevel(idx);
			for (int i = 0; i < height[idx - 1] * width[idx - 1]; ++i) {
				reinterpret_cast<float*>(CNRmap[idx])[i] /= noise_level[idx];

			}
		}
		
	}

} // namespace myMusica