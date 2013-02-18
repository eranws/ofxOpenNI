#include "ColorFingerTracker.h"

#include "ofxCv.h"
#include "cvUtils.h"
#include "openni.h"


void ColorFingerTracker::update()
{
	//get color and Depth Mat
	ofPtr<ofPixels> colorPixels = colorStream->getPixels();
	const cv::Mat colorMat = ofxCv::toCv(*colorPixels);

	ofPtr<ofShortPixels> depthPixels = depthStream->getPixels();
	const cv::Mat depthMat = ofxCv::toCv(*depthPixels);

	cv::Mat colorMatHsv;
	cvtColor(colorMat,colorMatHsv,CV_RGB2HSV);

	vector<cv::Mat> mv;
	cv::split(colorMatHsv, mv);
	//showMat(mv[0]);
	//showMat(mv[1]);
	//showMat(mv[2]);

	cv::Mat hue = mv[0];
	cv::Mat sat = mv[1];
	cv::Mat val = mv[2];


	cv::Mat basicMask = 
		sat > satRange->getScaledValueLow() &
		sat < satRange->getScaledValueHigh() &
		val > valRange->getScaledValueLow() &
		val < valRange->getScaledValueHigh();
	//showMat(basicMask);


	cv::Mat redMask = getHueMask(hue, redSlider->getScaledValue(), hueRange->getScaledValue()) & basicMask;
	cv::Mat yellowMask = getHueMask(hue, yellowSlider->getScaledValue(), hueRange->getScaledValue()) & basicMask;
	cv::Mat greenMask = getHueMask(hue, greenSlider->getScaledValue(), hueRange->getScaledValue()) & basicMask;

	cv::morphologyEx(redMask, redMask, CV_MOP_CLOSE, cv::getStructuringElement(CV_SHAPE_ELLIPSE, cv::Size(5,5)));


	vector<cv::Mat> mv2;
	cv::split(colorMatHsv, mv2);
	mv2[0] = mv[0];
	mv2[1] = basicMask;
	mv2[2] = basicMask;

	cv::Mat hsv2;
	cv::merge(mv2, hsv2);
	cvtColor(hsv2,hsv2, CV_HSV2BGR);
	//showMat(hsv2);


	//detectKnuckle(yellowMask, depthMat);
	detectWrist(greenMask, depthMat);
	detectFinger(redMask, depthMat);


}





void ColorFingerTracker::setupGui()
{
	float dim = 16; 
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING; 
	float length = 255-xInit; 

	satRange = gui->addRangeSlider("satRange", 0.0, 255.0, 100, 255, length-xInit, dim);
	valRange = gui->addRangeSlider("valRange", 0.0, 255.0, 100, 255, length-xInit, dim);		

	redSlider = gui->addSlider("red", 0.0, 180.0, 170.0, length-xInit, dim);		
	yellowSlider = gui->addSlider("yellow", 0.0, 180.0, 30.0, length-xInit, dim);		
	greenSlider = gui->addSlider("green", 0.0, 180.0, 45.0, length-xInit, dim);
	hueRange = gui->addSlider("hueRange", 0.0, 90.0, 10.0, length-xInit, dim);

	satRange->setLabelPrecision(0);
	satRange->setIncrement(1);

	valRange->setLabelPrecision(0);
	valRange->setIncrement(1);
}

vector<ofPoint> ColorFingerTracker::getContourRealPoints( std::vector<cv::Point> contour, const cv::Mat& depthMat, int medianZ, int range )
{
	vector<ofPoint> points;
	for (int i = 0; i < contour.size(); i++)
	{
		const cv::Point& pt = contour[i];
		ushort z = depthMat.at<ushort>(pt);
		if (abs(z  - medianZ) < range)
		{			ofPoint real;			openni::CoordinateConverter::convertDepthToWorld(*depthStream->getStream(), pt.x, pt.y, z, &real.x, &real.y, &real.z);			points.push_back(real);
			//realPts.push_back(cv::Matx13f(real.x, real.y, real.z));

		}
	}
	return points;
}

void ColorFingerTracker::detectWrist( cv::Mat wristMask, const cv::Mat depthMat )
{
	std::vector<cv::Point> biggestContour = getBiggestContour(wristMask);
	if (!biggestContour.empty())
	{
		int medianZ = getContourMedianZ(biggestContour, depthMat);
		vector<ofPoint> fingerPoints = getContourRealPoints(biggestContour, depthMat, medianZ, 50);
		if (fingerPoints.size() > 0)
		{
			ofPoint wristMean;
			for (int i = 0; i < fingerPoints.size(); i++)
			{
				wristMean += fingerPoints[i];
			}
			wristMean /= fingerPoints.size();

			wrist.pos = wristMean;
			wrist.valid = true;
		}
	}
}

void ColorFingerTracker::detectKnuckle( cv::Mat knuckleMask, const cv::Mat depthMat )
{
	std::vector<cv::Point> biggestContour = getBiggestContour(knuckleMask);
	if (!biggestContour.empty())
	{
		int medianZ = getContourMedianZ(biggestContour, depthMat);
		vector<ofPoint> knucklePoints = getContourRealPoints(biggestContour, depthMat, medianZ, 50);
		if (knucklePoints.size() > 0)
		{
			ofPoint knuckleMean;
			for (int i = 0; i < knucklePoints.size(); i++)
			{
				knuckleMean += knucklePoints[i];
			}
			knuckleMean /= knucklePoints.size();

			fingerKnuckle.pos = knuckleMean;
			fingerKnuckle.valid = true;
		}
	}
}


void ColorFingerTracker::detectFinger( const cv::Mat& fingerMask, const cv::Mat& depthMat)
{

	std::vector<cv::Point> biggestContour = getBiggestContour(fingerMask);
	if (!biggestContour.empty())
	{
		int medianZ = getContourMedianZ(biggestContour, depthMat);
		vector<ofPoint> fingerPoints = getContourRealPoints(biggestContour, depthMat, medianZ, 50);

		if (fingerPoints.size() > 10)
		{
			cv::Mat pcaset(fingerPoints.size(), 3, CV_32FC1);
			for (int i = 0; i < pcaset.rows; i++)
			{
				pcaset.at<float>(i, 0) = fingerPoints[i].x;
				pcaset.at<float>(i, 1) = fingerPoints[i].y;
				pcaset.at<float>(i, 2) = fingerPoints[i].z;
			}
			cv::PCA pca(pcaset, cv::Mat(), CV_PCA_DATA_AS_ROW);

			cv::Mat pcaProj = pca.project(pcaset);

			double minVal;
			double maxVal;

			cv::minMaxIdx(pcaProj.col(0), &minVal, &maxVal);

			ofPoint ev1(pca.eigenvectors.at<float>(0, 0),
				pca.eigenvectors.at<float>(0, 1),
				pca.eigenvectors.at<float>(0, 2));

			ofPoint pcaMean(pca.mean.at<float>(0),
				pca.mean.at<float>(1),
				pca.mean.at<float>(2));

			if (ev1.x < 0) ev1 = -ev1; //force point towards screen ('x': sensor on ceiling, rotated 90deg)
			
			fingerTip.pos = pcaMean + ev1 * maxVal;
			fingerBase.pos = pcaMean + ev1 * minVal;

			fingerTip.valid = true;
			fingerBase.valid = true;
		}
	}
}

void ColorFingerTracker::draw()
{
	ofPushStyle();

	if (fingerTip.isValid())
	{
		ofSetColor(ofColor::red); //orange
		ofSphere(fingerTip, 15);
	}
	
	if (fingerBase.isValid())
	{
		ofSetColor(ofColor::blue);
		ofSphere(fingerBase, 15);
	}

	if (fingerTip.isValid() && fingerBase.isValid())
	{
		ofSetColor(ofColor::yellow / 2 + ofColor::red / 2); //orange
		ofSetLineWidth(5);
		ofLine(fingerTip, fingerBase);
	}

	if (wrist.isValid())
	{
		ofSetColor(ofColor::green);
		ofSphere(wrist, 15);
	}

	if (fingerKnuckle.isValid())
	{
		ofSetColor(ofColor::yellow);
		ofSphere(fingerKnuckle, 25);
	}

	ofPopStyle();
}