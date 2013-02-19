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

		cv::Mat m = cv::Mat::zeros(fingerMask.size(), CV_8UC1);
		cv::fillConvexPoly(m, &biggestContour[0], biggestContour.size(), CV_RGB(255, 255, 255));
		cv::morphologyEx(m, m, CV_MOP_CLOSE, cv::getStructuringElement(CV_SHAPE_ELLIPSE, cv::Size(5, 5)));
	
		cv::Mat depthmask;
		absdiff(depthMat, medianZ, depthmask);
		depthmask = depthmask < 50;
		m &= depthmask;

		int sz = cv::countNonZero(m);
		if (biggestContour.size() > 10 && sz > 0)
		{
			cv::Mat pcaset(sz, 2, CV_32FC1);
			int i = 0;
			for (int y = 0; y < m.rows; y++) //TODO: optimize by iterating on enclosing rect
			{
				for (int x = 0; x < m.cols; x++)
				{
					if (m.at<uchar>(y,x) > 0)
					{
						pcaset.at<float>(i, 0) = x;
						pcaset.at<float>(i, 1) = y;
						i++;
					}
				}
			}
			cv::PCA pca(pcaset, cv::Mat(), CV_PCA_DATA_AS_ROW);

			cv::Mat pcaProj = pca.project(pcaset);

			double minVal;
			double maxVal;

			cv::minMaxIdx(pcaProj.col(0), &minVal, &maxVal);

			ofPoint ev1(pca.eigenvectors.at<float>(0, 0) ,pca.eigenvectors.at<float>(0, 1));
			ofPoint pcaMean(pca.mean.at<float>(0), pca.mean.at<float>(1));
			
			if (ev1.x > 0) ev1 = -ev1; //force point towards screen ('x': sensor on ceiling, rotated 90deg)
			
			ofPoint tip = pcaMean + ev1 * maxVal * 0.8;
			ofPoint base = pcaMean + ev1 * minVal * 0.8;

			
			fingerTip.pos = toReal(*depthStream->getStream(), depthMat, tip.x, tip.y);
			fingerBase.pos = toReal(*depthStream->getStream(), depthMat, base.x, base.y);

			
			fingerTip.pos.z = medianZ;
			fingerBase.pos.z = medianZ; 
			

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