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

