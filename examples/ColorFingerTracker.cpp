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


	cv::Mat redMask = getHueMask(hue, redSlider->getScaledValue(), hueRange->getScaledValue()) & basicMask;
	cv::Mat yellowMask = getHueMask(hue, yellowSlider->getScaledValue(), hueRange->getScaledValue()) & basicMask;
	cv::Mat greenMask = getHueMask(hue, greenSlider->getScaledValue(), hueRange->getScaledValue()) & basicMask;


	vector<cv::Mat> mv2;
	cv::split(colorMatHsv, mv2);
	mv2[0] = mv[0];
	mv2[1] = basicMask;
	mv2[2] = basicMask;
	showMat(basicMask);

	cv::Mat hsv2;
	cv::merge(mv2, hsv2);
	cvtColor(hsv2,hsv2, CV_HSV2BGR);
	showMat(hsv2);


	Contour biggestRedContour = getBiggestContour(redMask);

	if (!biggestRedContour.contour.empty())
	{

		cv::Mat fingerDepth;
		depthMat.copyTo(fingerDepth, biggestRedContour.mask);

		vector<int> zValues;
		for (int i = 0; i < biggestRedContour.contour.size(); i++)
		{
			const cv::Point& pt = biggestRedContour.contour[i];
			int z = depthMat.at<ushort>(pt.y, pt.x);
			if (z > 0) zValues.push_back(z);
		}

		int medianZ = median(zValues);

		cv::Mat maskDepth = depthMat < medianZ + 50 & depthMat > medianZ - 50;
		cv::Mat mask2 = biggestRedContour.mask & maskDepth;
		showMat(mask2);


		vector<ofPoint> fingerPoints;
		uchar* data = mask2.data;
		for (int i = 0; i < mask2.cols * mask2.rows; i++)
		{
			if (data[i] == 0) continue;

			int x = i % mask2.cols;
			int y = i / mask2.cols;
			int z = depthMat.at<ushort>(y, x);
			if (z > 0)
			{
				ofPoint proj(x, y, z);
				ofPoint real;
				openni::CoordinateConverter::convertDepthToWorld(*depthStream->getStream(), proj.x, proj.y, proj.z, &real.x, &real.y, &real.z);
				fingerPoints.push_back(real);
			}

		}

		if (fingerPoints.size() > 0)
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

			//TODO: stabilize point
			cv::Mat baseTip = cv::Mat::zeros(2, 3, CV_32FC1);
			
			baseTip.at<float>(0, 0) = maxVal;
			baseTip.at<float>(1, 0) = minVal;

			pca.backProject(baseTip, baseTip);

			fingerTip.pos = ofPoint(baseTip.at<float>(0, 0), baseTip.at<float>(0, 1), baseTip.at<float>(0, 2));
			fingerBase.pos = ofPoint(baseTip.at<float>(1, 0), baseTip.at<float>(1, 1), baseTip.at<float>(1, 2));

			fingerTip.valid = true;
			fingerBase.valid = true;
		}
	}


}



void ColorFingerTracker::draw()
{
	if (fingerTip.valid && fingerBase.valid)
	{
		ofSetColor(ofColor::yellow / 2 + ofColor::red / 2); //orange
		ofLine(fingerTip, fingerBase);

		ofSetColor(ofColor::red / 2); //orange
		ofSphere(fingerTip, 15);

		ofSetColor(ofColor::yellow); //orange
		ofSphere(fingerBase, 25);


	}
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

