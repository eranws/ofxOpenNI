#include "cvUtils.h"

cv::Rect getHandFrameFromFG(cv::Mat& img, const ofPoint& handPosition, const openni::VideoStream& depthStream)
{
	if (handPosition.z == 0)
	{
		return cv::Rect();
	}
	const int handPointToTopLength = 240; //[millimeters]
	const int handPointToSideLength = 120; //[millimeters]
	const int handPointToBottomLength = 240; //[millimeters]

	ofPoint projTopLeft;
	ofPoint projBottomRight;
	ofPoint real;

	// convert real to proj.
	real = ofPoint(handPosition.x + handPointToSideLength, handPosition.y + handPointToTopLength, handPosition.z);
	//oni.depthGenerator().ConvertRealWorldToProjective(1, &real, &projTopLeft);
	openni::CoordinateConverter::convertWorldToDepth(depthStream, real.x, real.y, real.z, &projTopLeft.x, &projTopLeft.y, &projTopLeft.z);
	real = ofPoint(handPosition.x - handPointToSideLength, handPosition.y - handPointToBottomLength, handPosition.z);
	//oni.depthGenerator().ConvertRealWorldToProjective(1, &real, &projBottomRight);
	openni::CoordinateConverter::convertWorldToDepth(depthStream, real.x, real.y, real.z, &projBottomRight.x, &projBottomRight.y, &projBottomRight.z);



	int startRow = (int)MAX(projTopLeft.y, 0.0f);
	int endRow = (int)MIN((int)projBottomRight.y, (int)depthStream.getVideoMode().getResolutionY() - 1);
	cv::Range rowRange(startRow, endRow);

	// Cols are inverted. (starting from bottom)
	int startCol = (int)MAX(projBottomRight.x, 0.0f);
	int endCol = (int)MIN((int)projTopLeft.x, (int)depthStream.getVideoMode().getResolutionX() - 1);

	cv::Range colRange(startCol, endCol);
	cv::Rect depthRoiRect;
	depthRoiRect.x = colRange.start;
	depthRoiRect.y = rowRange.start;
	depthRoiRect.width = colRange.size();
	depthRoiRect.height = rowRange.size();

	cv::Rect res;
	res = /*convertDepthToImageOLD*/(depthRoiRect);	
	res.width = MIN(img.cols - res.x, res.width);
	res.height = MIN(img.rows - res.y , res.height);

	return res;
}

cv::Mat getHueMask( const cv::Mat& hueMat, int hue, int range )
{
	//compares hue mask from value and range (wraps around 180)
	cv::Mat mask;
	mask = (hueMat < hue + range & hueMat > hue - range);
	if (hue - range < 0) mask |= hueMat > hue - range + 180;
	if (hue + range > 180) mask |= hueMat < hue + range - 180;
	return mask;
}


std::vector<cv::Point> getBiggestContour(const cv::Mat& mask)
{
	std::vector<cv::Point> biggest;

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours( mask.clone(), contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE ); //Find the Contour BLOBS
	if( !contours.empty() && !hierarchy.empty() )
	{
		int idx = 0;
		int maxArea = 0; 
		for(int i = 0; i < hierarchy.size(); i++)
		{
			//find biggest cc
			int area = cv::contourArea(contours[i]);
			if (maxArea < area)
			{
				maxArea = area;
				idx = i;
			}

		}
		
		biggest = contours[idx];

		cv::Mat m;
		cvtColor(mask, m, CV_GRAY2RGB);
		cv::drawContours(m, contours, idx, CV_RGB(255,0,0));
		showMat(m);
	}

	

	return biggest;
}

int getContourMedianZ( std::vector<cv::Point> contour, const cv::Mat& depthMat )
{
	vector<int> zValues;
	for (int i = 0; i < contour.size(); i++)
	{
		const cv::Point& pt = contour[i];
		int z = depthMat.at<ushort>(pt.y, pt.x);
		if (z > 0) zValues.push_back(z);
	}
	return median(zValues);
}

ofPoint toProj( const openni::VideoStream& stream, ofPoint real )
{
	ofPoint proj;
	openni::CoordinateConverter::convertWorldToDepth(stream,
		real.x, real.y, real.z,
		&proj.x, &proj.y, &proj.z);

	return proj;
}



ofPoint toReal(const openni::VideoStream& stream, const cv::Mat depthMat, int i, int j) 
{
	ofPoint real;

	openni::CoordinateConverter::convertDepthToWorld(stream,
		i, j, depthMat.at<ushort>(j, i),
		&real.x, &real.y, &real.z);

	return real;
}
