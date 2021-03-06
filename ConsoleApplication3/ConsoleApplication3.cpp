// ConsoleApplication3.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include "raw_read.h"
#include "Laplace_pyramid.h"
#include <opencv2/highgui.hpp>

typedef unsigned char BYTE;
typedef unsigned short WORD;

using namespace std;
using namespace cv;

namespace MUSICA {
	
} // namespace MUSICA

int method = 0;

void ImgProcess(int state, void* userdata) {
	const char* filename = "1.raw";

	Mat rawImage;

	myMusica::RawRead(filename, rawImage, 2292, 2804);

	myMusica::LaplacePyramid laplacePyramid(rawImage);

	Mat currentImage;
	laplacePyramid.GetCurrentImage(currentImage);

	

	switch (method)
	{
	case(0):
		cv::resizeWindow("Image Show", currentImage.rows / 4, currentImage.cols / 4);
		cv::imshow("Image Show", currentImage);
	default:
		break;
	}
	
	

}

int main()
{
	namedWindow("Image Show", 0);

	createTrackbar("Img Process", "Image Show", &method, 8, ImgProcess);

	/*
	for (;;) {

		std::cout << "Choose a method to deal with current image." << endl;
		std::cout << "1 : SetMinMaxWindow" << endl << "2 : Flip" << endl;
		std::cout << "3 : Mirror" << endl << "4 : Revoke" << endl << "5 : Reset" << endl;
		std::cout << "6 : Complementary" << endl << "7 : Rotate clockwise" << endl;
		std::cout << "8 : Rotate counterclockwise" << endl;
		std::cout << "9 : Canny sharpness" << endl;
		std::cout << "Press q/Q/ESC : Quit" << endl << endl;

		
		char key = getchar();
		char enter = getchar();
		if (key == 27 || key == 'q' || key == 'Q') {

			break;

		}

		double t = (double)getTickCount();
		

		switch (key) {
		case('1'):

			std::cout << "SetMinMaxWIndow" << endl << endl;
			laplacePyramid.Get8BitImage(currentImage);
			break;

		case('2'):

			std::cout << "Flip" << endl << endl;
			laplacePyramid.GetFlipedImage(currentImage);
			break;

		case('3'):

			std::cout << "Mirror" << endl << endl;
			laplacePyramid.GetMirroredImage(currentImage);
			break;

		case('4'):

			std::cout << "Revoke" << endl << endl;
			laplacePyramid.Revoke(currentImage);
			break;

		case('5'):

			std::cout << "Reset" << endl << endl;
			laplacePyramid.Reset(currentImage);
			break;

		case('6'):

			std::cout << "Complemantary" << endl << endl;
			laplacePyramid.GetComplementaryImage(currentImage);
			break;

		case('7'):

			std::cout << "Rotate clockwise" << endl << endl;
			laplacePyramid.GetRotateClockwiseImage(currentImage);
			break;

		case('8'):

			std::cout << "Rotate counterclockwise" << endl << endl;
			laplacePyramid.GetRotateCounterclockwiseImage(currentImage);
			break;

		case('9'):

			std::cout << "Canny sharpness" << endl << endl;
			laplacePyramid.GetCannySharpness(currentImage);
			break;

		default:
			laplacePyramid.GetCurrentImage(currentImage);
			break;
		}

		t = (double)getTickCount() - t;

		std::cout << "Cost time : " << t / getTickFrequency() * 1000 << "ms" <<endl << endl << endl;

		cv::imshow("Image Show", currentImage);
		cv::waitKey(10);
	}
	*/
	cv::waitKey(0);
	destroyAllWindows();
	return 0;
}

