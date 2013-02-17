#include "ColorFingerTracker.h"

#include "ofxCv.h"
#include "cvUtils.h"
#include "openni.h"


void ColorFingerTracker::update()
{
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


	cv::Mat basicMask = sat > satThreshold->getScaledValue() & mv[2] > valueThreshold->getScaledValue();

	//cv::Mat mask = (mv[0] < 50 | mv[0] > 205) & basicMask;
	//showMat(mask);

	vector<cv::Mat> mv2;
	cv::split(colorMatHsv, mv2);
	mv2[0] = mv[0];
	mv2[1] = basicMask;
	mv2[2] = basicMask;

	cv::Mat hsv2;
	cv::merge(mv2, hsv2);
	cvtColor(hsv2,hsv2,CV_HSV2BGR);
	showMat(hsv2);


	cv::morphologyEx(basicMask, basicMask, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7,7)));

		std::vector<std::vector<cv::Point> > contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours( basicMask, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE ); //Find the Contour BLOBS
		if( !contours.empty() && !hierarchy.empty() )
		{
			cv::Mat biggestContourMask = cv::Mat::zeros(basicMask.size(), CV_8UC1);

			int idx = 0;
			for(int i = 0; i < hierarchy.size(); i++)
			{
				if (cv::contourArea(contours[i]) > 10) //clear salt noise
				{
					cv::Moments _mu = moments( cv::Mat(contours[i]), false );
					cv::Point2f _mc( _mu.m10/_mu.m00 , _mu.m01/_mu.m00);

					//cv::drawContours(contoursMat, contours, i, red, 2, 8, hierarchy );
				}

				//find biggest cc
				if (cv::contourArea(contours[idx]) < cv::contourArea(contours[i]))
					idx = i;
			}

			cv::drawContours(biggestContourMask, contours, idx, cv::Scalar::all(255), CV_FILLED);

			cv::Mat fingerDepth;
			depthMat.copyTo(fingerDepth, biggestContourMask);

			vector<cv::Point>& biggestContour = contours[idx];

			vector<int> zValues;
			for (int i = 0; i < biggestContour.size(); i++)
			{
				const cv::Point& pt = biggestContour[i];
				int z = depthMat.at<ushort>(pt.y, pt.x);
				if (z > 0) zValues.push_back(z);
			}

			int medianZ = median(zValues);
			cout << medianZ << endl;

			//dilate(biggestContourMask, biggestContourMask, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7,7)));

			cv::Mat maskDepth = depthMat < medianZ + 50 & depthMat > medianZ - 50;
			cv::Mat mask2 = biggestContourMask & maskDepth;
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
				fingMean = ofPoint(pca.mean.at<float>(0), pca.mean.at<float>(1), pca.mean.at<float>(2));
				fingDir = ofPoint(pca.eigenvectors.at<float>(0, 0), pca.eigenvectors.at<float>(0, 1), pca.eigenvectors.at<float>(0, 2));
				valid = true;


			}


		}
}



void ColorFingerTracker::draw()
{
	if (valid)
	{
		ofSetColor(ofColor::green);
		ofLine(fingMean, fingMean + fingDir * 100);
		ofSphere(fingMean, 20);
	}
}

void ColorFingerTracker::setupGui()
{
	float dim = 16; 
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING; 
	float length = 255-xInit; 
	
	satThreshold = gui->addSlider("satThreshold", 0.0, 255.0, 160, length-xInit, dim);
	valueThreshold = gui->addSlider("hueThreshold", 0.0, 255.0, 150, length-xInit, dim);		

}

void ColorFingerTracker::customSetup()
{
	valid = false;
}



