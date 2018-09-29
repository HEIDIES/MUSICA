#pragma once
#include <opencv2/opencv.hpp>
#include <memory>
#include "Image_processor.h"
#include "useful_tool.h"


typedef unsigned char BYTE;
typedef unsigned short WORD;

namespace myMusica{

	class LaplacePyramid {
	public:

		/**
		 * @brief : constructor.
		 * @params : _src : input raw image
		 * @params : depth : # of layers of laplace pyramid, default is 10
		 * @params : kernel_size : size of gaussian kernel
		 */

		LaplacePyramid(cv::InputArray _src, const int& depth = 10, const int& kernel_size = 5);

		~LaplacePyramid();

		/**
		 * @brief : get the detail image at layer idx, only for developer testing use
		 */

		void GetDetailImage(const int& idx, cv::OutputArray _dst);

		/**
		 * @brief : get the approximate Image at layer idx, only for developer testing use
		 */

		void GetApproximateImage(const int& idx, cv::OutputArray _dst);

		/**
		 * @brief : get the enhenced image
		 */

		void GetEnhencedImage(cv::OutputArray _dst);

		/**
		 * @brief : get the height of origin image
		 */

		void GetOriginHeight(int& height);

		/**
		 * @brief : get the width of origin image
		 */

		void GetOriginWidth(int& width);

		/**
		 * @brief : get the size of origin image
		 */

		void GetOriginSize(cv::Size & size);

		/**
		 * @brief : get the type of origin image
		 */

		void GetOriginType(int& type);

		/**
		 * @brief : get current image which is at the top of the image status stack
		 */

		void GetCurrentImage(cv::OutputArray _dst);

		/**
		 * @brief : get origin image
		 */

		void GetOriginImage(cv::OutputArray _dst);

		/**
		 * @brief : get the mirror of the current image 
		 * and push it into the top of the image status stack
		 */

		void GetMirroredImage(cv::OutputArray _dst);

		/**
		 * @brief : get the flip of the current image
		 * and push it into the top of the image status stack
		 */

		void GetFlipedImage(cv::OutputArray _dst);

		/**
		 * @brief : get the 8 bit image of the current image
		 * and push it into the top of the image status stack
		 */

		void Get8BitImage(cv::OutputArray _dst);

		void Revoke(cv::OutputArray _dst);

		void Reset(cv::OutputArray _dst);

		void GetComplementaryImage(cv::OutputArray _dst);

		void RotateImage(RotateDirection direction);

		void GetRotateClockwiseImage(cv::OutputArray _dst);

		void GetRotateCounterclockwiseImage(cv::OutputArray _dst);

		void GetCannySharpness(cv::OutputArray _dst);

		ImageProcessor* IP_ptr;

	private:

		void NonLinearConversion(const int& idx);

		void CalcCNRMap(const int& idx);

		void CalcNoiseLevel(const int& idx);

		void ReconstructOneLayer(const int& idx);

		void Reconstruction();

		void BuildLaplacePyramid();

		void SetDetailApproximateImage(const int& idx);


		/**
		 * @brief : originImage : input image
		 * @brief : originType : type of input image
		 * @brief : originHeight : height of input image
		 * @brief : originWidth : width of input image
		 */
		void* originImage;
		int originType;
		int originHeight;
		int originWidth;
		

		/**
		 * @brief : currentImage : current image, image to show, to be dealed by ImageProcessor
		 * @brief : currentType : type of current image
		 * @brief : currentHeight : height of current image
		 * @brief : currentWidth : width of current image
		 */
		void* currentImage;
		int currentType;
		int currentHeight;
		int currentWidth;


		/**
		 * @brief : kernel_size : size of gaussian kernel, used to build laplace pyramid
		 * @brief : depth : the # of layers of laplace pyramid, maximal value is 10(default)
		 * @brief : detailImages : detail images of all layers, capacity is 10
		 * @brief : approximateImages : approximate Images of all layers, capacity is 10
		 * @brief : height : height of each approximate image and detail image
		 * @brief : width : width of each approxamate image and detail image
		 */
		int kernel_size;
		int depth;
		void* detailImages[10];
		void* approximateImages[10];
		int height[10];
		int width[10];


		/**
		 * @brief : tmpPImage : temp image during reconstruction
		 * @brief : enhencedImage : image after contrast enhencement
		 */
		void* tmpPImage;
		void* enhencedImage;
		
		
		/**
		 * @brief : CNRmap : contrast-noise-ratio map of detail image 1-3
		 * @brief : noise_level : estimated noise level of detail image 1-3
		 */
		void* CNRmap[10];
		float noise_level[10];
	};

} // namespace myMusica