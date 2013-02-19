#pragma once

#include "opencv2/opencv.hpp"

#include <vector>
#include "ofMain.h"
#include "openni.h"

#define showMat(x) showMatR(x, 1)
#define showMatR(x, i)	\
{						\
	cv::Mat y;			\
	cv::resize(x, y, cv::Size(), i, i, cv::INTER_LINEAR);\
	cv::imshow(#x, y);	\
}


template <class T>
T median(const std::vector<T> &v1)
{
	std::vector<T> v = v1;
	if (v.empty()) return 0;
	size_t n = v.size() / 2;
	nth_element(v.begin(), v.begin()+n, v.end());
	return v[n];
}


cv::Rect getHandFrameFromFG(cv::Mat& img, const ofPoint& handPosition, const openni::VideoStream& depthStream);


cv::Mat getHueMask( const cv::Mat& hueMat, int hue, int range );

std::vector<cv::Point> getBiggestContour(const cv::Mat& mask);

int getContourMedianZ( std::vector<cv::Point> contour, const cv::Mat& depthMat );



ofPoint toProj(const openni::VideoStream& stream, ofPoint real);
ofPoint toReal(const openni::VideoStream& stream, const cv::Mat depthMat, int i, int j);

