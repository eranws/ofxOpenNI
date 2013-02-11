#include "testApp.h"
#include "ofxCv\Utilities.h"

#define PROFILE
#ifdef PROFILE
#include "src\ofxProfile.h"
#endif
#include <math.h>
#include "../cvUtils.h"

void  update_mhi( IplImage* img, IplImage* dst, int diff_threshold );

const string testApp::MODULE_NAME = "testApp";

cv::Scalar white(255, 255, 255);
cv::Scalar blue(255,0,0);
cv::Scalar green(0,255,0);
cv::Scalar yellow(255,255,0);
cv::Scalar red(0,0,255);
cv::Scalar purple(255,0,255);
cv::Scalar black(0,0,0);

void depthOnMouse( int event, int x, int y, int flags, void* thisApp)
{
	switch( event )
	{
	case CV_EVENT_LBUTTONDOWN:
		{
			testApp* app = (testApp*)thisApp;
			cv::Mat mat = app->depthMat.clone();

			ofPoint proj;
			proj.x = (float)x;
			proj.y = (float)y;
			proj.z = mat.at<ushort>(y, x);


			stringstream ss;
			ss << proj.z;
			cv::putText(mat, ss.str(), cv::Point(15,15),cv::FONT_HERSHEY_PLAIN,1, cv::Scalar(0,0,0));
			cv::putText(mat, ss.str(), cv::Point(14,16),cv::FONT_HERSHEY_PLAIN,1, cv::Scalar(255,255,255));

			imshow("Depth", mat * 32);

			printf("%.2f\n", proj.z);
			//oni.depthGenerator().ConvertProjectiveToRealWorld(1, &proj, &g_handPosition);


		}
		break;
	case CV_EVENT_RBUTTONDOWN: 
		{
		}
		break;
	case CV_EVENT_LBUTTONUP:
		{		
		}
		break;
	case CV_EVENT_RBUTTONUP:
		{
		}
		break;
	case CV_EVENT_MOUSEMOVE:
		{

		}	
		break;
	}	
}


//--------------------------------------------------------------
void testApp::setup() {

	//ofSetLogLevel(OF_LOG_SILENT);
	ofSetLogLevel(MODULE_NAME, OF_LOG_VERBOSE);
	ofLog::setAutoSpace(true);

	cout.precision(2);
	ofSetFrameRate(100);

	try
	{
		oniDevice.setup();
	}
	catch (exception e)
	{
		//oniDevice.setup("c:\\1.oni");
		//oniDevice.setup("E:/gridRecordings/2013-01-07-17-30-35-716.oni");
		//oniDevice.setup("E:/gridRecordings/2013-01-07-17-43-01-955.oni");
		//oniDevice.setup("E:/gridRecordings/2013-01-07-19-02-28-215.oni");
		//oniDevice.setup("E:/gridRecordings/2013-01-07-19-02-28-215.oni");
		//oniDevice.setup("130602_1058.oni");
		oniDevice.setup("130206_1633.oni");
	}


	depthStream.setup(oniDevice.getDevice());
	colorStream.setup(oniDevice.getDevice());
	//oniDevice.setRegistration(true);

	recorder.setup();
	recorder.addStream(depthStream.getStream());
	recorder.addStream(colorStream.getStream());

	handTracker.setup(depthStream.getDevice());


	if (oniDevice.getDevice()->isFile())
	{
		oniDevice.getDevice()->getPlaybackControl()->setSpeed(-1); // manual mode
	}
	else
	{
		depthStream.startThread(false);
		colorStream.startThread(false);
		handTracker.startThread(false);
	}

	handCam.setDistance(10);
	faceTracker.setup();

	sceneCam.setGlobalPosition(0,0,0);
	sceneCam.setTarget(ofPoint(0, 0, 3000));
	sceneCam.setDistance(2000);

	setupGui(); 

	depthHistorySize = 10;

	handHistorySize = 100;

	//parabola filter
	A = cv::Mat(fingerWristHistorySize, 3, CV_32FC1);
	for (int i = 0; i < fingerWristHistorySize; i++)
	{
		int t = fingerWristHistorySize/2 - i;
		A.at<float>(i, 0) = t * t;
		A.at<float>(i, 1) = t;
		A.at<float>(i, 2) = 1;
	}
	AA = (A.t() * A);
	AA = AA.inv();
	AA = AA * A.t();

	motion = NULL;

	lastClicked = 0;
	frameIndex = 0;
	toUpdate = true;
	readOnUpdate = false;

	cv::namedWindow("Depth");
	cvSetMouseCallback("Depth", depthOnMouse, this);

	showProfilerString = false;
	drawDebugString = false;
	drawOpenNiDebug = false;
	logToFile = false;

	recordingStartFrame = 0;
}




ofPoint projToReal(const openni::VideoStream& stream, const cv::Mat depthMat, int i, int j) 
{
	ofPoint real;

	openni::CoordinateConverter::convertDepthToWorld(stream,
		j, i, depthMat.at<ushort>(i,  j),
		&real.x, &real.y, &real.z);

	return real;
}



//--------------------------------------------------------------
void testApp::update(){

	ofxProfileThisFunction();
	
	if (readOnUpdate)
	{
		depthStream.readFrame();
		colorStream.readFrame();
		handTracker.readFrame();

		openni::PlaybackControl* pbc = oniDevice.getDevice()->getPlaybackControl();
		if (pbc != NULL && !playToggle->getValue())
		{
			int nFrames = pbc->getNumberOfFrames(*depthStream.getStream().get());
			if (frameIndex >= nFrames)
			{
				readOnUpdate = false;
				logToFile = false;
				pbc->seek(*depthStream.getStream().get(), 0);
			}
		}
	}

	if (!toUpdate && frameIndex == handTracker.getFrameIndex())
	{
		return;
	}
	else
	{
		frameIndex = handTracker.getFrameIndex();
		toUpdate = false;
	}

	debugString = stringstream();
	logString = stringstream();

	logString << frameIndex << " ";
	logString << handTracker.getTimestamp() << " ";

	// update colorPixels
	ofPixels colorPixels = colorStream.getPixels();
	cv::Mat colorMat = ofxCv::toCv(colorPixels);


	ofPoint handPoint = ofPoint();
	if (handTracker.hasHand())
	{

		hand.pos = handTracker.getHandPoint();
		hand.frame = frameIndex;

		handPoint = handTracker.getHandPoint();
		handHistory.push_front(handPoint);
		if (handHistory.size() > handHistorySize)
		{
			handHistory.pop_back();
		}
	}
	else
	{
		handHistory.clear();
	}

	logString << handPoint << " ";



	if (faceToggle->getValue())
	{
		ofxProfileSectionPush("faceTracker update");
		faceTracker.update(colorMat);
		ofxProfileSectionPop();
	}
	if(!faceToggle->getValue() || !faceTracker.getFound())
	{
		facePos = ofVec3f();
		screenPoint = ofVec2f();
	}


	ofPtr<ofShortPixels> depthPixels = depthStream.getPixels();

	if (cvDepthToggle->getValue())
	{
		depthMat = ofxCv::toCv(*depthPixels);
		

		int wristFoundCounter = 0;
		bool wristFound = false;
		bool fingerFound = false;
		bool handFound = false;

		handFound = handTracker.hasHand();
		if (handFound)
		{
			ofPoint handProj;
			openni::CoordinateConverter::convertWorldToDepth(*depthStream.getStream(), handPoint.x, handPoint.y, handPoint.z, &handProj.x, &handProj.y, &handProj.z);

			cv::Mat depth8;
			depthMat.convertTo(depth8, CV_8UC1, 0.1);

			cv::Mat depth8Color;
			cvtColor(depth8, depth8Color, CV_GRAY2RGB);


			//find finger
			cv::Mat grayQ = cv::Mat::zeros(depthMat.size(), CV_8UC1);

			deque<cv::Point> q;
			q.push_back(cv::Point(handProj.x, handProj.y));
			cv::Point fingerCandidate = q.front();

			while (!q.empty())
			{
				cv::Point pt = q.front();
				q.pop_front();
				short z = depthMat.at<ushort>(pt.y, pt.x);

				if (pt.y < fingerCandidate.y && z > 0)
				{
					fingerCandidate = pt;
				}

				depth8Color.row(pt.y).col(pt.x) = yellow;

				if (pt.y > 1 && pt.x > 2 && pt.y < depthMat.rows - 1 && pt.x < depthMat.cols - 2)
				{
					int maxJ = (pt.y < handProj.y + 0) ? 2 : 1; // allow search down for 0 pixels
					for (int j = -1; j < maxJ; j++)
					{
						for (int i = -2; i < 3; i++)
						{
							short cz = depthMat.at<ushort>(pt.y + j, pt.x + i);
							if(fabs(z - handPoint.z) < 200 && cz < z + 50)
							{
								if (grayQ.at<uchar>(pt.y + j, pt.x + i) == 0)
								{
									grayQ.at<uchar>(pt.y + j, pt.x + i) = 1;
									q.push_back(cv::Point(pt.x + i, pt.y + j));
								}
							}
						}
					}
				}
			}

			cv::circle(depth8Color, fingerCandidate, 3, green, CV_FILLED);
			fingerFound = (handProj.y - fingerCandidate.y) > 3;

			if (fingerFound)
			{
				// spatially averaged finger point
				cv::circle(depth8Color, fingerCandidate, 1, red, CV_FILLED); //closest point

				ofPoint fingerCandidateProj(fingerCandidate.x, fingerCandidate.y, depthMat.at<ushort>(fingerCandidate.y, fingerCandidate.x));
				ofPoint fingerCandidateReal;
				openni::CoordinateConverter::convertDepthToWorld(*depthStream.getStream(), fingerCandidateProj.x, fingerCandidateProj.y, fingerCandidateProj.z, &fingerCandidateReal.x, &fingerCandidateReal.y, &fingerCandidateReal.z);

				ofPoint fingerOffsetRealTotal = ofPoint();
				int fingerOffsetRealCount = 0;

				for (int j = 0; j < depthMat.rows; j++)
				{
					for (int i = 0; i < depthMat.cols; i++)
					{
						if (grayQ.at<uchar>(j, i) == 0) continue;
						ofPoint fingReal;
						ofPoint fingProj;
						fingProj.x = i;
						fingProj.y = j;
						fingProj.z = depthMat.at<ushort>(j,  i);

						openni::CoordinateConverter::convertDepthToWorld(*depthStream.getStream(),
							fingProj.x, fingProj.y, fingProj.z,
							&fingReal.x, &fingReal.y, &fingReal.z);

						ofPoint offset = fingReal - fingerCandidateReal;

						if (fabs(offset.y) < 30 && fabs(offset.x) < 70 && fabs(offset.z) < 30)
						{
							depth8Color.row(j).col(i) = green;
							//float p = offset.length() / 30;
							fingerOffsetRealTotal += offset;// * (1-p);
							fingerOffsetRealCount++;
						}
					}
				}

				assert(fingerOffsetRealCount && "Should be at least the finger itself!");
				ofPoint fingerOffsetAvgReal = fingerOffsetRealTotal / fingerOffsetRealCount;
				ofPoint fingerReal = fingerCandidateReal + fingerOffsetAvgReal;
				ofPoint fingerProj;
				openni::CoordinateConverter::convertWorldToDepth(*depthStream.getStream(), fingerReal.x, fingerReal.y, fingerReal.z, &fingerProj.x, &fingerProj.y, &fingerProj.z);
				cv::circle(depth8Color, cv::Point2i(fingerProj.x, fingerProj.y), 3, red, 2);


				fingerHistory.push_front(fingerReal);
				if (fingerHistory.size() > fingerHistorySize)
				{
					fingerHistory.pop_back();
				}



				cv::Mat wristGrayQ = cv::Mat::zeros(depthMat.size(), CV_8UC1);
				deque<cv::Point> wristQ;
				wristQ.push_back(cv::Point(handProj.x, handProj.y));

				ofPoint wristCandidateReal;
				cv::Point wristCandidateXY;
				while (!wristQ.empty() && !wristFound)
				{
					cv::Point pt = wristQ.front();

					depth8Color.row(pt.y).col(pt.x) = blue;
					short z = depthMat.at<ushort>(pt.y, pt.x);

					if (z > 0)
					{

						ofPoint d(pt.x, pt.y, z);

						ofPoint tempReal;
						openni::CoordinateConverter::convertDepthToWorld(*depthStream.getStream(), d.x, d.y, d.z, &tempReal.x, &tempReal.y, &tempReal.z);

						ofPoint diff = handPoint - tempReal;
						if (diff.length() > 150)
						{

							wristFoundCounter++;
							if (wristFoundCounter > 20)
							{
								wristFound = true;
								wristCandidateReal = tempReal;
								wristCandidateXY = pt;
							}
						}
					}

						if (pt.y > 1 && pt.x > 2 && pt.y < depthMat.rows - 3 && pt.x < depthMat.cols - 2)
						{
							for (int j = 0; j < 4; j++)
							{
								for (int i = -2; i < 3; i++)
								{
									if (wristGrayQ.at<uchar>(pt.y + j, pt.x + i) == 0)
									{
										short cz = depthMat.at<ushort>(pt.y + j, pt.x + i);
										int zDiff = (i==0) ? 120 : 30;
										if(fabs(z - handPoint.z) < 300 && abs(cz - z) < zDiff)
											
										{
										
											wristGrayQ.at<uchar>(pt.y + j, pt.x + i) = 1;
											wristQ.push_back(cv::Point(pt.x + i, pt.y + j));
										}

									}
								}
							}
						}
					
					wristQ.pop_front();
				}


				if (wristFound)
				{

					ofPoint wristReal;
					ofPoint wristRealAverage;
					ofPoint wristOffsetRealTotal = ofPoint();
					int wristOffsetRealCount = 0;

					wristGrayQ = cv::Mat::zeros(depthMat.size(), CV_8UC1);
					wristQ.clear();
					wristQ.push_back(wristCandidateXY);


					while (!wristQ.empty())
					{
						cv::Point pt = wristQ.front();
						short z = depthMat.at<ushort>(pt.y, pt.x);

						if (z > 0)
						{
							ofPoint d(pt.x, pt.y, z);

							ofPoint tempReal;
							openni::CoordinateConverter::convertDepthToWorld(*depthStream.getStream(), d.x, d.y, d.z, &tempReal.x, &tempReal.y, &tempReal.z);


							ofPoint wristOffset = tempReal - wristCandidateReal;
							if (fabs(wristOffset.y) < 30 && fabs(wristOffset.x) < 70 && fabs(wristOffset.z) < 50)
							{
								depth8Color.row(pt.y).col(pt.x) = red;
								//float p = offset.length() / 30;
								wristOffsetRealTotal += wristOffset;// * (1-p);
								wristRealAverage += tempReal;

								wristOffsetRealCount++;
							}







							if (pt.y > 1 && pt.x > 2 && pt.y < depthMat.rows - 3 && pt.x < depthMat.cols - 2)
							{
								for (int j = 0; j < 4; j++)
								{
									for (int i = -2; i < 3; i++)
									{
										if (wristGrayQ.at<uchar>(pt.y + j, pt.x + i) == 0)
										{
											short cz = depthMat.at<ushort>(pt.y + j, pt.x + i);
											int zDiff = (i==0) ? 120 : 30;
											if(fabs(z - handPoint.z) < 300 && abs(cz - z) < zDiff && pt.y < wristCandidateXY.y + 20)
											{
												wristGrayQ.at<uchar>(pt.y + j, pt.x + i) = 1;
												wristQ.push_back(cv::Point(pt.x + i, pt.y + j));
											}
										}
									}
								}
							}
						}
						wristQ.pop_front();
					}



					assert(wristOffsetRealCount && "Should be at least the wrist itself!");
					ofPoint wristOffsetAvgReal = wristOffsetRealTotal / wristOffsetRealCount;
					//wristReal = wristCandidateReal + wristOffsetAvgReal;
					wristReal = wristRealAverage / wristOffsetRealCount;


					ofPoint wristProj;
					openni::CoordinateConverter::convertWorldToDepth(*depthStream.getStream(), wristReal.x, wristReal.y, wristReal.z, &wristProj.x, &wristProj.y, &wristProj.z);
					cv::circle(depth8Color, cv::Point2i(wristProj.x, wristProj.y), 3, blue, 3); //closest point

					wrist.pos = wristReal;
					wrist.frame = frameIndex;

					finger.pos = fingerReal;
					finger.frame = frameIndex;



					ofPoint fingerWrist = wristReal - fingerReal;

					

					fingerWristHistory.push_front(fingerWrist);
					if (fingerWristHistory.size() > fingerWristHistorySize)
					{
						fingerWristHistory.pop_back();
					}

				} // if (wristFound)
			}
			showMat(depth8Color);
			imshow("Depth", depthMat * 32);

		}


		if (fingerFound)
		{
			logString << fingerHistory.front() << " ";

			if (fingerHistory.size() == fingerHistorySize)
			{
			}
		}
		else
		{
			logString << ofPoint();
			fingerHistory.clear();
		}



		if (handFound && wristFound && fingerFound)
		{
			if (fingerWristHistory.size() == fingerWristHistorySize)
			{



				cv::Mat pcaset(fingerWristHistorySize, 2, CV_32FC1);
				for (int i = 0; i < fingerWristHistorySize; i++)
				{
					pcaset.at<float>(i, 0) = fingerWristHistory[i].x;
					pcaset.at<float>(i, 1) = fingerWristHistory[i].z;
				}

				//! PCA compressPCA(InputArray pcaset, int maxComponents, const Mat& testset, OutputArray compressed)
				cv::PCA pca(pcaset, // pass the data
					cv::Mat(), // there is no pre-computed mean vector,
					// so let the PCA engine to compute it
					CV_PCA_DATA_AS_ROW // indicate that the vectors
					// are stored as matrix rows
					// (use CV_PCA_DATA_AS_COL if the vectors are
					// the matrix columns)					
					//! maxComponents // specify how many principal components to retain
					);

				/*			
				cout << pcaset << endl;
				cout << pca.eigenvalues << endl;
				cout << pca.eigenvectors << endl;
				cout << pca.mean << endl;
				*/

				cv::Mat B(fingerWristHistorySize, 1, CV_32FC1);
				cv::Mat Bpca(fingerWristHistorySize, 1, CV_32FC1);


				Bpca = pca.project(pcaset).col(0);


				for (int i = 0; i < fingerWristHistorySize; i++)
				{
					B.at<float>(i) = fingerWristHistory[i].length();
				}



				cv::Mat pcaProj = pca.project(pcaset);
				cv::Mat pcaShow;

				const int S = 500;
				cv::Point center(S/2, S/2);
				pcaShow.create(S, S, CV_8UC3);

				for (int i = 0; i < fingerWristHistorySize; i++)
				{
					cv::circle(pcaShow, center + cv::Point(pcaProj.at<float>(i, 0), pcaProj.at<float>(i, 1)), 5 + i, blue, 2);
				}

				cv::line(pcaShow, center, center - cv::Point(S * pca.eigenvectors.at<float>(0, 0), S * pca.eigenvectors.at<float>(0, 1)), red, 3);
				//cv::line(pcaShow, center, center + cv::Point(S * pca.eigenvectors.at<float>(1, 0), S * pca.eigenvectors.at<float>(1, 1)), green, 2);
				//showMat(pcaShow);

				cv::Mat X = AA * B;
				cv::Mat Xpca = AA * Bpca;

				float a = X.at<float>(0);
				float b = X.at<float>(1);
				float c = X.at<float>(2);


				cv::Mat diff = A * X - B;
				cv::Mat sd;
				cv::pow(diff, 2, sd);

				float ssd = cv::sum(sd)[0];


				float apca = Xpca.at<float>(0);
				cv::Mat sdpca;
				cv::pow(A * Xpca - Bpca, 2, sdpca);
				float ssdpca = cv::sum(sdpca)[0];

				//ofLog() << ofGetSystemTime() << a << ssd << apca << ssdpca;


				for (int i = 0; i < 3; i++)
				{
					mgZ->addPoint(B.at<float>(0));//(wrist.pos - finger.pos).z);
					mgA->addPoint(a);
					mgApca->addPoint(apca);

					mgB->addPoint(b);
					mgC->addPoint(c);
					mgErr->addPoint(ssd);
					mgErrPca->addPoint(pca.eigenvalues.at<float>(1));
				}

				//cout << X;
				//cout << ssd;

				/*
				ofPoint fw = fingerWristHistory.front();
				float ang = atan2f(fw.z, fw.x) / PI; // -1 < a < 1
				*/
				float ang = atan2f(pca.eigenvectors.at<float>(0, 1), pca.eigenvectors.at<float>(0, 0)) / PI;

				//cout << "atan: " << ang;
				if (apca < apcaThreshold->getScaledValue())
				{
					if (ssdpca < errpcaThreshold->getScaledValue())
					{
						if (ofGetFrameNum() > lastClicked + 5)
						{
							lastClicked = ofGetFrameNum();
							int k = floor(5 * ofMap(ang, 0.2, 0.8, 0, 1, true));
							cout << " key" << k;

							keypad.keypadPressed(k + 5);
						}
					}


				}
				//cout << endl;


			}
		}
		else
		{
			fingerWristHistory.clear();
		}

		if (computeHistory->getValue())
		{

			depthHistory.push_front(depthMat.clone());
			if (depthHistory.size() > depthHistorySize)
			{
				depthHistory.pop_back();
			}
			if (depthHistory.size() == depthHistorySize)
			{

				cv::Mat d = depthHistory[0].clone();
				cv::Mat d8 = depthHistory[0].clone();
				d8.convertTo(d8, CV_8UC1, 0.1);

				if (velocityMasking->getValue())
				{
					cv::Mat d0 = depthHistory[1] - depthHistory[0];
					ofxUISlider* s0 = (ofxUISlider*)gui1->getWidget("0");		
					ofxUISlider* s1 = (ofxUISlider*)gui1->getWidget("1");		
					ofxUISlider* s2 = (ofxUISlider*)gui1->getWidget("2");		

					cv::Mat mask;
					//mask.create(depthHistory[0].size(), CV_8UC1);
					mask = (depthHistory[0] == 0);

					const int s = 5; //make slider (caution!)

					//mask = (depthHistory[s] == 0);
					//mask += (d > s1->getScaledValue());
					for (int i = 1; i < s; i++)
					{
						cv::Mat diff = depthHistory[i] - depthHistory[i-1];
						mask |= (diff < 1);
					}
					d.setTo(0, mask);

					cv::imshow("mask", mask);

					int morph_size = 2;
					cv::Mat element = getStructuringElement( 2, cv::Size( 2*morph_size + 1, 2*morph_size+1 ), cv::Point( morph_size, morph_size ) );

					cv::Mat morphMask;
					morphologyEx(mask, morphMask, CV_MOP_OPEN, element, cv::Point(-1, -1), 5);
					cv::imshow("morphMask", morphMask);


					double maxVal;
					cv::Point maxLoc;
					cv::minMaxLoc(d, 0, &maxVal, 0, &maxLoc);

					//cv::Mat drgb;

#define addMat(x) cv::Mat& x = matMap[#x] = cv::Mat();
					addMat(drgb);
					d.convertTo(drgb, CV_8UC1);
					cv::cvtColor(drgb, drgb, CV_GRAY2RGB);

					cv::circle(drgb, maxLoc, 1 + maxVal * s2->getScaledValue(), CV_RGB(255, 0, 0), CV_FILLED);
					cv::putText(drgb, ofToString(maxVal), cv::Point(11, 11), CV_FONT_HERSHEY_PLAIN, 1, CV_RGB(255, 255, 255));
					cv::putText(drgb, ofToString(maxVal), cv::Point(10, 10), CV_FONT_HERSHEY_PLAIN, 1, CV_RGB(0, 255, 0));
					//cv::circle(drgb, cv::Point(100,100), 30, CV_RGB(255, 0, 0), CV_FILLED);
					cv::imshow("d", d * s0->getScaledValue());
					cv::imshow("drgb", drgb);// * s2->getScaledValue());


					//
					cv::Mat invMask;
					cv::bitwise_not(morphMask, invMask);
					cv::Mat d0mask = d0 > 0;
					morphologyEx(d0mask, d0mask, CV_MOP_OPEN, element, cv::Point(-1, -1), 2);
				} //end if(velocityMasking)


				if (depthThresholding->getValue())
				{
					cv::Mat d8thrMean;
					adaptiveThreshold(d8, d8thrMean, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 3, 0);
					cv::Mat d8clean = d8thrMean.clone();
					for (int i = 2; i < 6; i++)
					{
						cv::Mat d8thrMeanBlurred;
						blur(d8thrMean, d8thrMeanBlurred, cv::Size(i, i));
						d8clean &= d8thrMeanBlurred >= (255 / i);
					}
					cv::imshow("d8clean", d8clean);

					//contours
					std::vector<std::vector<cv::Point> > contours;
					std::vector<cv::Vec4i> hierarchy;
					cv::Mat contoursMat = cv::Mat::zeros(depthMat.size(), CV_8UC3);
					cv::findContours( d8clean, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE ); //Find the Contour BLOBS
					if( !contours.empty() && !hierarchy.empty() )
					{
						int idx = 0;
						for(int i = 0; i < hierarchy.size(); i++)
						{
							if (cv::contourArea(contours[i]) > 10) //clear salt noise
							{
								cv::Moments _mu = moments( cv::Mat(contours[i]), false );
								cv::Point2f _mc( _mu.m10/_mu.m00 , _mu.m01/_mu.m00);

								cv::Scalar color(_mc.x * 255 / d8thrMean.cols, 255,  _mc.y * 255 / d8thrMean.rows);
								cv::drawContours(contoursMat, contours, i, color, 2, 8, hierarchy );
							}

							//find biggest cc
							if (cv::contourArea(contours[idx]) < cv::contourArea(contours[i]))
								idx = i;
						}

						cv::drawContours(contoursMat, contours, idx, white, CV_FILLED);//, 8, hierarchy );

					}

					cv::imshow("invMask", d8thrMean);
					cv::imshow("contoursMat", contoursMat);


					//Color
					cv::Mat colorMat = ofxCv::toCv(colorPixels);
					cv::Mat grayMat;
					cv::cvtColor(colorMat, grayMat, CV_BGR2GRAY);
					showMat(grayMat);

					cv::Mat edges;

					ofxUISlider* can1 = (ofxUISlider*)gui1->getWidget("can1");		
					double threshold1 = can1->getScaledValue();

					ofxUISlider* can2 = (ofxUISlider*)gui1->getWidget("can2");		
					double threshold2 = can2->getScaledValue();

					cv::Canny(grayMat, edges, threshold1, threshold2);//, int apertureSize=3, bool L2gradient=false )¶
					showMat(edges);

					/*
					cv::Mat ba;
					depthHistory[0].convertTo(ba, CV_8UC1, 0.25);
					ba.setTo(0, mask);

					cv::cvtColor(ba, ba, CV_GRAY2RGB);

					IplImage image = ba;

					if (motion == NULL)
					{
					motion = cvCreateImage( cvSize(image.width, image.height), 8, 3);
					cvZero( motion );
					//motion->origin = image->origin;
					}

					update_mhi(&image, motion, 30 );
					cvShowImage( "Motion", motion );
					*/


					/*
					ofTexture depthTex;
					depthTex.allocate(m8.cols, m8.rows, GL_LUMINANCE);
					depthTex.loadData(m8.ptr(), m8.cols, m8.rows, GL_LUMINANCE);
					*/


					/*
					cv::Mat d1 = depthHistory[2] - depthHistory[1]; 
					d1.setTo(0, depthHistory[2] == 0 | depthHistory[1] == 0);

					cv::Mat a = d1 - d;
					a.setTo(0, d1 == 0 | d == 0);


					cv::Mat a8;	a.convertTo(a8, CV_8UC1);




					cv::Mat a8edges;
					cv::Canny(a8, a8edges, 10, 100);
					cv::imshow("a8edges", a8edges);

					cv::Mat a8eq;
					cv::equalizeHist(a8, a8eq);
					cv::imshow("a8eq", a8eq);
					*/

				}//end if (depthThresholding)
			}//end if (depthHistory.size() == depthHistorySize)
		}//end if (computeHistory)
	}//end if (cvDepthToggle->getValue())

	if (logToFile)
	{
		ofLogVerbose(MODULE_NAME) << logString.str();
	}

	ofxProfileSectionPop();

}

//--------------------------------------------------------------
void testApp::draw(){
	ofxProfileThisFunction();
	ofBackground(0);

	ofSetColor(255);

	ofTexture colorTexture;
	ofPixels colorPixels = colorStream.getPixels(); 
	colorTexture.allocate(colorPixels);
	colorTexture.loadData(colorPixels);
	colorTexture.draw(0,0);

	ofTexture depthTexture;
	ofPtr<ofShortPixels> depthRawPixels = depthStream.getPixels();

	ofPixels depthPixels;
	depthPixels.allocate(depthRawPixels->getWidth(), depthRawPixels->getHeight(), OF_PIXELS_RGBA);

	const unsigned short* prd = depthRawPixels->getPixels();
	unsigned char* pd = depthPixels.getPixels();
	for (int i = 0; i < depthRawPixels->size(); i++)
	{
		const short minDepth = 450;
		short s = prd[i];
		char x = (s < minDepth) ? 0 : powf(s - minDepth, 0.7f);
		pd[4 * i + 0] = 255 - x;
		pd[4 * i + 1] = 255 - x;
		pd[4 * i + 2] = 255 - x;
		pd[4 * i + 3] = x;

	}


	depthTexture.allocate(depthPixels);
	depthTexture.loadData(depthPixels);
	depthTexture.draw(640,0);

	colorTexture.draw(320,0);
	depthTexture.draw(320,0);




	sceneCam.begin();

	scene.draw();

	ofSetColor(255);



	if (finger.frame == frameIndex)
	{
		ofSetColor(ofColor::green);
		ofSphere(finger.pos, 10);
	}
	if (wrist.frame == frameIndex)
	{
		ofSetColor(ofColor::red);
		ofSphere(wrist.pos, 10);
	}
	if (wrist.frame == frameIndex && finger.frame == frameIndex)
	{
		ofSetColor(ofColor::white);
		ofSetLineWidth(10);
		ofLine(wrist.pos, finger.pos);

		float p = wrist.pos.z / (wrist.pos.z - finger.pos.z);
		ofSetLineWidth(3);
		//ofLine(wrist.pos, wrist.pos.getInterpolated(finger.pos, p));
	}





	if (drawHand->getValue() && !handHistory.empty())
	{
		ofSetColor(ofColor::blue);
		ofSphere(handHistory.front(), 10);
	}

	if (drawHandHistory->getValue())
	{

		glEnable(GL_DEPTH_TEST);       	
		if (handHistory.size() == handHistorySize)
		{
			for (int i = 1; i < handHistorySize; i++)
			{
				float p = 1 - (float(i) / (handHistorySize));

				ofColor c = ofColor::fromHsb(255 * p, 255, 255);
				ofSetColor(c);
				ofSphere(handHistory[i], 10 * p + 3);
				//ofSphere(handHistory[i], 20);

				ofSetLineWidth(10 * p + 3);
				ofLine(handHistory[i-1], handHistory[i]);
			}
		}
	}


	if (drawFingerHistory->getValue())
	{
//		glEnable(GL_DEPTH_TEST);       	
		if (fingerHistory.size() == fingerHistorySize)
		{


			cv::Mat pcaset(fingerHistorySize, 3, CV_32FC1);
				for (int i = 0; i < fingerHistorySize; i++)
				{
					pcaset.at<float>(i, 0) = fingerHistory[i].x;
					pcaset.at<float>(i, 1) = fingerHistory[i].y;
					pcaset.at<float>(i, 2) = fingerHistory[i].z;
				}

				
				cv::PCA pca(pcaset, cv::Mat(), CV_PCA_DATA_AS_ROW);
				
				/*			
				cout << pcaset << endl;
				*/

				/*
				cout << pca.eigenvalues << endl;
				cout << pca.eigenvectors << endl;
				cout << pca.mean << endl;
				*/


			stringstream fingerDebugString;

			cv::Mat pcaproj = pca.project(pcaset).col(0);
			fingerDebugString << pcaproj << endl;
			ofSetColor(ofColor::green);
			ofDrawBitmapString(fingerDebugString.str(), 100, ofGetScreenHeight() - 200);


			for (int i = 1; i < fingerHistorySize; i++)
			{
				float p = 1 - (float(i) / (fingerHistorySize));

				ofColor c = ofColor::fromHsb(255 * p, 255, 255);
				ofSetColor(c);
				ofSphere(fingerHistory[i], 10 * p + 3);
				//ofSphere(handHistory[i], 20);

				ofSetLineWidth(10 * p + 3);
				ofLine(fingerHistory[i-1], fingerHistory[i]);
				
			}

			ofPoint ev1(pca.eigenvectors.at<float>(0, 0),
				pca.eigenvectors.at<float>(0, 1),
				pca.eigenvectors.at<float>(0, 2));

			ofPoint pcaMean(pca.mean.at<float>(0),
				pca.mean.at<float>(1),
				pca.mean.at<float>(2));

			if (ev1.z > 0) ev1 = -ev1; //force point towards screen


			if (pca.eigenvalues.at<float>(0) / pca.eigenvalues.at<float>(1) > 50 && ev1.z < 0.5)
			{
				fingerHistoryScreenIntersectionPoint = scene.screen.getIntersectionPointWithLine(pcaMean, fingerHistory[0] + ev1);
				ofSetColor(255);
				ofLine(pcaMean, fingerHistoryScreenIntersectionPoint);
				ofSphere(fingerHistoryScreenIntersectionPoint, 20);
			}
			else
			{
				fingerHistoryScreenIntersectionPoint = ofPoint();
			}
				


		}
	}






#define camlog(func) {debugString << #func << " : " << sceneCam.func() << endl;}
	camlog(getDistance);
	camlog(getPosition);
	camlog(getOrientationEuler);
	camlog(getFarClip);
#undef camlog



	ofPushMatrix();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);


	if(faceToggle->getValue() && faceTracker.getFound())
	{

		ofPushMatrix();

		const float b = (facePos==ofVec3f()) ? 0 : 0.5;
		facePos = (b*facePos) + (1-b) * depthStream.cameraToWorld(faceTracker.getPosition());

		ofSetColor(ofColor::green);
		ofSphere(facePos, 5);

		ofTranslate(facePos);
		debugString << "facePos" << facePos;

		ofxCv::applyMatrix(faceTracker.getRotationMatrix());
		ofRotateY(180.0);
		faceTracker.getObjectMesh().drawWireframe();

		ofPopMatrix();
	}

#ifdef OPENNI1

	// iterate through users
	int oldestHandIndex = 0;
	for (int h = 0; h < openNIDevice.getNumTrackedHands(); h++)
	{
		if (openNIDevice.getTrackedHand(h).getBirthTime() < openNIDevice.getTrackedHand(oldestHandIndex).getBirthTime())
			oldestHandIndex = h;
	}

	int i=oldestHandIndex;
	if(openNIDevice.getNumTrackedHands() > 0)
	{
		// get a reference to this user
		ofxOpenNIHand & hand = openNIDevice.getTrackedHand(i);

		// get hand position
		ofPoint & handPosition = hand.getPosition();

		// draw a rect at the position
		ofSetColor(255,0,0);
		//ofRect(handPosition.x, handPosition.y, 2, 2);

		// set depthThresholds based on handPosition
		ofxOpenNIDepthThreshold & depthThreshold = openNIDevice.getDepthThreshold(i); // we just use hand index for the depth threshold index
		//depthThreshold.drawROI();


		if (fingers[i].isTracked)
		{
			// draw ROI over the depth image
			ofSetColor(255,255,255);
			//handCam.setGlobalPosition(0,0,handPosition.z + 400);
			//handCam.begin();
			//cam.lookAt(handPosition);//, ofVec3f(0, -1, 0));
			depthThreshold.getPointCloud().disableColors();
			depthThreshold.drawPointCloud();

			ofNoFill();


			ofSetColor(ofColor::blue, 128);
			ofSphere(fingers[i].getFilteredPosition(0.5), 5);

			//handCam.end();
		}

	}

#endif

	if(faceToggle->getValue() && faceTracker.getFound())
	{
		ofPoint tip = finger.frame == frameIndex ? finger.pos : handTracker.getHandPoint();
		ofPushStyle();
		ofSetLineWidth(3);
		ofSetColor(ofColor::green);
		ofDrawArrow(facePos, tip);
		ofSetLineWidth(1);
		ofSetColor(ofColor::yellow);
		ofLine(facePos, tip.interpolated(facePos, -3));
		ofPopStyle();

		ofSetColor(ofColor::magenta);

		ofxProfileSectionPush("getIntersectionPointWithLine");
		ofPoint screenIntersectionPoint = scene.screen.getIntersectionPointWithLine(facePos, tip);
		ofxProfileSectionPop();

		ofSphere(screenIntersectionPoint, 10);

		const float b = (screenPoint==ofVec2f()) ? 0 : 0.5;
		screenPoint = (b*screenPoint) + (1-b) * scene.screen.getScreenPointFromWorld(screenIntersectionPoint);

		screenPointHistory.push_front(screenPoint);
		if (screenPointHistory.size() > 10)
		{
			screenPointHistory.pop_back();
		}

		int score = 0;
		for (int i = 0; i < screenPointHistory.size(); i++)
		{
			if (screenPointHistory[i].y > 0 && screenPointHistory[i].y < ofGetScreenHeight())
			{
				if (i < screenPointHistory.size() / 2)
				{
					if (screenPointHistory[i].x < 0)// || abs(screenPointHistory[i].x - ofGetScreenWidth()) < 40)
						score++;
				}
				else
				{
					if (screenPointHistory[i].x > 0)
						score++;
				}
			}
		}

		if (score > screenPointHistory.size() - 1)
		{
			ofSetColor(255, 0, 0);
			//ofCircle(screenPointHistory[0], 30);
		}



	}



	ofDisableBlendMode();
	ofPopMatrix();

	sceneCam.end();

	//2D (gui) drawing
	
	if (drawFingerHistory->getValue() && fingerHistory.size() == fingerHistorySize && fingerHistoryScreenIntersectionPoint != ofPoint())
	{
		ofSetColor(ofColor::yellow);
		ofVec2f hwscreenPoint = scene.screen.getScreenPointFromWorld(fingerHistoryScreenIntersectionPoint);
		ofCircle(hwscreenPoint, 10);
		ofLine(ofGetScreenWidth()/2, ofGetScreenHeight()/2, hwscreenPoint.x, hwscreenPoint.y);
		ofCircle(0, hwscreenPoint.y, 10);
		ofCircle(ofGetScreenWidth(), hwscreenPoint.y, 10);
		ofCircle(hwscreenPoint.x, 0, 10);
		ofCircle(hwscreenPoint.x, ofGetScreenHeight(), 10);
	}
	

	if(faceToggle->getValue() && faceTracker.getFound())
	{
		ofFill();
		ofSetColor(255);
		ofCircle(screenPoint, 10);
		ofNoFill();
		for (int i=0; i<screenPointHistory.size();i++)
		{
			int ri = screenPointHistory.size() - i - 1;
			ofSetColor(255,255,255,1 - 0.1*i);
			ofCircle(screenPointHistory[ri], 21 - ri);
		}
		debugString << "screenPoint: " << screenPoint << endl;
	}


	static float timeThen = 0;
	static float timeNow = 0;
	static float frameRate = 0;

	timeNow = ofGetElapsedTimef();
	double diff = timeNow-timeThen;	
	if( diff  > 0.00001 ){
		float fps = 1.0 / diff;
		frameRate	*= 0.9f;
		frameRate	+= 0.1f*fps;
	}
	timeThen = timeNow;

	// draw some info regarding frame counts etc
	ofSetColor(0, 255, 0);
	debugString << "Run Time: " << ofToString(ofGetElapsedTimeMillis() / 1000) << "." << ofToString(ofGetElapsedTimeMillis() % 1000) << endl;
	debugString << "frameRate: " << frameRate << endl;
	debugString << "fps: " << ofGetFrameRate() << endl;
	debugString << "Is Recording: " << (recorder.IsRecording() ? "On" : "Off") << endl;
	debugString << endl;
	debugString << "frame #" << frameIndex << endl;

	

	//	camString << "Device FPS: " << openNIDevice.getFrameRate()<< endl;



#ifdef OPENNI1
	debugString << ofGetFrameRate() << endl << "Device FPS: " << openNIDevice.getFrameRate()<< endl;
#endif

	if (showProfilerString)
	{
		debugString << ofxProfile::describe();
	}

	if (drawDebugString)
	{
		ofSetColor(ofColor::green);
		ofDrawBitmapString(debugString.str(), 10, 240 + 20);
	}

	keypad.draw();

}

#ifdef OPENNI1
//--------------------------------------------------------------
void testApp::handEvent(ofxOpenNIHandEvent & event){
	// show hand event messages in the console
	ofLogNotice() << getHandStatusAsString(event.handStatus) << "for hand" << event.id << "from device" << event.deviceID;
}
#endif

//--------------------------------------------------------------
void testApp::exit(){

	if (recorder.IsRecording())
	{
		recorder.stop();
	}
	dumpGroundTruth();

	faceTracker.stopThread();
	faceTracker.waitForThread();

	handTracker.exit();
	colorStream.exit();
	depthStream.exit();
	oniDevice.exit();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	switch (key)
	{
	case OF_KEY_RIGHT:
	case OF_KEY_LEFT:
	case 'l':
	case 'k':
	case 'L':
	case 'K':
		{		
			openni::PlaybackControl* pbc = oniDevice.getDevice()->getPlaybackControl();
			if (pbc != NULL && !playToggle->getValue())
			{
				int inc = (key == OF_KEY_RIGHT || key == 'l' || key == 'L') ? 1 : -1;
				inc *= (key == 'L' || key == 'K') ? 10 : 1;
				frameIndex += inc;

				// check bounds
				if (frameIndex < 0)
				{
					frameIndex = 0;
				}
				int nFrames = pbc->getNumberOfFrames(*depthStream.getStream().get());
				if (frameIndex > nFrames)
				{
					frameIndex = nFrames;
				}

				pbc->seek(*depthStream.getStream().get(), frameIndex);
				depthStream.readFrame();
				colorStream.readFrame();
				handTracker.readFrame();

				toUpdate = true;

			}
		}
		break;


	case 'r':
		{
			openni::PlaybackControl* pbc = oniDevice.getDevice()->getPlaybackControl();
			if (pbc != NULL && !playToggle->getValue())
			{
				readOnUpdate = !readOnUpdate;
			}
		}
		break;


	case 'R':
		{
			openni::PlaybackControl* pbc = oniDevice.getDevice()->getPlaybackControl();
			if (pbc != NULL && !playToggle->getValue())
			{	
				pbc->seek(*depthStream.getStream().get(), 0);
			}
		}
		break;


	case 'O':
		{
			string filename = ofGetTimestampString();
			ofLogToFile(filename + ".txt", true);

			openni::PlaybackControl* pbc = oniDevice.getDevice()->getPlaybackControl();
			if (pbc != NULL && !playToggle->getValue())
			{
				pbc->seek(*depthStream.getStream().get(), 0);
			}

			logToFile = true;
			readOnUpdate = true;

		}
		break;


	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '0':
		{
		stringstream ss;
		ss << handTracker.getFrameIndex() - recordingStartFrame;
		ss << " ";
		ss << key - '0';
		clicks.push_back(ss.str());
		}
		break;

	case '-':
		{
			int lastps = keypad.getLastPreselect();
			if (lastps != -1)
			{
				stringstream ss;
				ss << handTracker.getFrameIndex() - recordingStartFrame;
				ss << " ";
				ss << lastps - keypad.getKeypadPressOffset() + 1;
				clicks.push_back(ss.str());
			}
		}
		break;

	case 'P':
		dumpGroundTruth();
		break;


	case OF_KEY_F8:
		if (!recorder.IsRecording())
		{
			recordingStartFrame = handTracker.getFrameIndex();
			recordingFilename = ofGetTimestampString();
			recorder.start(recordingFilename);
			toast.addText("Start Recording");
		}

		return;

	case OF_KEY_F7:
		if (recorder.IsRecording())
		{
			recorder.stop();
			toast.addText("Stop Recording");
		}
		return;


	case 'z': keypad.visible = !keypad.visible; return;

	}

	if (keypad.visible)
	{
		keypad.keyPressed(key);
	}
	else
	{
		switch (key)
		{
		//case '1': drawDebugString = !drawDebugString; break;
		//case '2': showProfilerString = !showProfilerString; break;
		case 'C': ofxProfile::clear(); break;

		case 'f': fullScreenToggle->toggleValue(); fullScreenToggle->triggerSelf(); break;
		//case 'g': guiAutoHide->toggleValue(); guiAutoHide->triggerSelf(); break;
		case ' ': playToggle->toggleValue(); playToggle->triggerSelf(); break;



		default:
			break;
		}
	}

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y )
{
	if (guiAutoHide->getValue())
	{
		if (x < 10 && !gui1->isVisible())	
			gui1->setVisible(true);

		if (x > gui1->getRect()->width)
			gui1->setVisible(false);
	}
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

#ifdef OPENNI1

void testApp::setupOpenNiDevice()
{
	openNIDevice.setup();
	//openNIDevice.setupFromONI("C:/f/q.oni");

	openNIDevice.addImageGenerator();
	openNIDevice.addDepthGenerator();
	openNIDevice.setUseDepthRawPixels(true);
	openNIDevice.setUseBackBuffer(false);

	openNIDevice.setRegister(true);
	openNIDevice.setMirror(true);

	// setup the hand generator
	openNIDevice.addHandsGenerator();

	// add all focus gestures (ie., wave, click, raise arm)
	depthStream.addAllHandFocusGestures();

	// or you can add them one at a time
	//vector<string> gestureNames = openNIDevice.getAvailableGestures(); // you can use this to get a list of gestures
	// prints to console and/or you can use the returned vector
	//openNIDevice.addHandFocusGesture("Wave");

	openNIDevice.setMaxNumHands(MAX_HANDS);

	for(int i = 0; i < openNIDevice.getMaxNumHands(); i++){
		ofxOpenNIDepthThreshold depthThreshold = ofxOpenNIDepthThreshold(0, 0, false, true, true, true, true); 
		// ofxOpenNIDepthThreshold is overloaded, has defaults and can take a lot of different parameters, eg:
		// (ofxOpenNIROI OR) int _nearThreshold, int _farThreshold, bool _bUsePointCloud = false, bool _bUseMaskPixels = true, 
		// bool _bUseMaskTexture = true, bool _bUseDepthPixels = false, bool _bUseDepthTexture = false, 
		// int _pointCloudDrawSize = 2, int _pointCloudResolution = 2
		depthThreshold.setUsePointCloud(true);
		depthThreshold.setPointCloudDrawSize(2);

		depthStream.addDepthThreshold(depthThreshold);

	}

	depthStream.start();
}
#endif


ofVec3f testApp::Finger::getFilteredPosition(float a)
{
	ofVec3f res;

	for (int i = 0; i < position.size(); i++)
	{
		res += pow(1-a,i) * position[i];
	}
	res *= a;
	return res;
}



void testApp::setupGui()
{

	float dim = 16; 
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING; 
	float length = 255-xInit; 


	gui1 = ofPtr<ofxUICanvas>(new ofxUICanvas(0, 0, length+xInit, ofGetHeight())); 
	gui1->addWidgetDown(new ofxUILabel("GUI", OFX_UI_FONT_LARGE)); 

	guiAutoHide = gui1->addToggle("guiAutoHide", false, dim, dim);
	fullScreenToggle = gui1->addToggle("fullScreen", false, dim, dim);
	
	playToggle = gui1->addToggle("play/pause", false, dim, dim);

	cvDepthToggle = gui1->addToggle("cvDepth", false, dim, dim);

	computeHistory = gui1->addToggle("computeHistory", false, dim, dim);
	velocityMasking = gui1->addToggle("velocityMasking", false, dim, dim);
	depthThresholding =	gui1->addToggle("depthThresholding", false, dim, dim);


	gui1->addSpacer(length-xInit, 2);
	gui1->addSlider("can1", 0.0, 255.0, 10, length-xInit, dim);
	gui1->addSlider("can2", 0.0, 255.0, 100, length-xInit, dim);


	int sliderHeight = 30;
	gui1->addSpacer(length-xInit, 2); 
	gui1->addWidgetDown(new ofxUILabel("V SLIDERS", OFX_UI_FONT_MEDIUM)); 
	gui1->addSlider("0", 0.0, 10255.0, 150, dim, sliderHeight);
	gui1->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
	gui1->addSlider("1", 0.0, 255.0, 150, dim, sliderHeight);
	gui1->addSlider("2", 0.0, 1.0, 1, dim, sliderHeight);
	gui1->addSlider("3", 0.0, 255.0, 150, dim, sliderHeight);
	gui1->addSlider("4", 0.0, 255.0, 150, dim, sliderHeight);
	gui1->addSlider("5", 0.0, 255.0, 150, dim, sliderHeight);
	gui1->addSlider("6", 0.0, 255.0, 150, dim, sliderHeight);
	gui1->addSlider("7", 0.0, 255.0, 150, dim, sliderHeight);
	gui1->addSlider("8", 0.0, 255.0, 150, dim, sliderHeight);
	gui1->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);


	gui1->addSpacer(length-xInit, 2);
	gui1->addWidgetDown(new ofxUILabel("TOGGLES", OFX_UI_FONT_MEDIUM));

	faceToggle = gui1->addToggle("FaceTracker", false, dim, dim);

	detectFingerToggle = gui1->addToggle("detectFinger", false, dim, dim);
	drawHand = gui1->addToggle("drawHand", false, dim, dim);
	drawHandHistory = gui1->addToggle("drawHandHistory", false, dim, dim);
	drawFingerHistory = gui1->addToggle("drawFingerHistory", false, dim, dim);

	vector<float> buffer; 
	for(int i = 0; i < 256; i++)
	{
		buffer.push_back(0.0);
	}
	mgZ = new ofxUIMovingGraph(length-xInit, 100, buffer, 256, -150, 150, "MOVING GRAPH Z");

	mgA = new ofxUIMovingGraph(length-xInit, 100, buffer, 256, -3, 1, "MOVING GRAPH A");
	mgApca = new ofxUIMovingGraph(length-xInit, 100, buffer, 256, -3, 1, "MOVING GRAPH Apca");
	mgB = new ofxUIMovingGraph(length-xInit, 100, buffer, 256, -5, 5, "MOVING GRAPH B");
	mgC = new ofxUIMovingGraph(length-xInit, 100, buffer, 256, 150, 250, "MOVING GRAPH C");

	mgErr = new ofxUIMovingGraph(length-xInit, 100, buffer, 256, 0, 200, "MOVING GRAPH Err");
	mgErrPca = new ofxUIMovingGraph(length-xInit, 100, buffer, 256, 0, 200, "MOVING GRAPH Errpca");

	gui1->addSpacer(length-xInit, 2);
	aThreshold = gui1->addSlider("aThreshold", -2.0, 0, -0.2, length-xInit, dim);
	errThreshold = gui1->addSlider("errThreshold", 0.0, 1000, 200, length-xInit, dim);

	apcaThreshold = gui1->addSlider("apcaThreshold", -2.0, 0, -1, length-xInit, dim);
	errpcaThreshold = gui1->addSlider("errpcaThreshold", 0.0, 1000, 500, length-xInit, dim);


	//gui1->addWidgetDown(mgZ);
	gui1->addWidgetDown(mgA);
	gui1->addWidgetDown(mgErr);

	gui1->addWidgetDown(mgApca);
	gui1->addWidgetDown(mgErrPca);
	//gui1->addWidgetDown(mgB);
	//gui1->addWidgetDown(mgC);

	//gui->addWidgetDown(new ofxUIWaveform(length-xInit, 64, buffer, 256, 0.0, 1.0, "WAVEFORM")); 


	gui1->setColorBack(ofColor::gray);
	//gui1->setDrawBack(true);
	gui1->setColorFill(ofColor::gray);
	gui1->setDrawFill(true);

	ofAddListener(gui1->newGUIEvent,this,&testApp::guiEvent);
}


void testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName(); 
	int kind = e.widget->getKind(); 
	cout << "got event from: " << name << endl; 	

	if(name == "DRAW GRID")
	{
		ofxUIButton *button = (ofxUIButton *) e.widget; 
		//bdrawGrid = button->getValue(); 
	}
	else if(name == "FaceTracker")
	{
		ofxUIToggle *toggle = (ofxUIToggle *) e.widget; 
		//bdrawGrid = toggle->getValue(); 
	}
	else if(name == "TEXT INPUT")
	{
		ofxUITextInput *textinput = (ofxUITextInput *) e.widget; 
		if(textinput->getTriggerType() == OFX_UI_TEXTINPUT_ON_ENTER)
		{
			cout << "ON ENTER: "; 
			//            ofUnregisterKeyEvents((ofxUItestApp*)this); 
		}
		else if(textinput->getTriggerType() == OFX_UI_TEXTINPUT_ON_FOCUS)
		{
			cout << "ON FOCUS: "; 
		}
		else if(textinput->getTriggerType() == OFX_UI_TEXTINPUT_ON_UNFOCUS)
		{
			cout << "ON BLUR: "; 
			//            ofRegisterKeyEvents(this);             
		}        
		string output = textinput->getTextString(); 
		cout << output << endl; 
	}

	else if(name == "fullScreen")
	{
		ofToggleFullscreen();
	}

	else if(name == "play/pause")
	{
		if(playToggle->getValue())
		{
			depthStream.startThread(false);
			colorStream.startThread(false);
			handTracker.startThread(false);
		}
		else
		{
			depthStream.stopThread();
			colorStream.stopThread();
			handTracker.stopThread();
		}
		
	}




}

void testApp::dumpGroundTruth()
{
	if (clicks.size() > 0)
	{
		ofstream of;
		of.open(recordingFilename + "_clicks" + ".txt");
		for (int i = 0; i < clicks.size(); i++)
		{
			of << clicks[i] << endl;
		}
		clicks.clear();
		of.close();
	}
}
















#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <time.h>
#include <stdio.h>
#include <ctype.h>

// various tracking parameters (in seconds)
const double MHI_DURATION = 1;
const double MAX_TIME_DELTA = 0.5;
const double MIN_TIME_DELTA = 0.05;
// number of cyclic frame buffer used for motion detection
// (should, probably, depend on FPS)
const int N = 4;

// ring image buffer
IplImage **buf = 0;
int last = 0;

// temporary images
IplImage *mhi = 0; // MHI
IplImage *orient = 0; // orientation
IplImage *mask = 0; // valid orientation mask
IplImage *segmask = 0; // motion segmentation map
CvMemStorage* storage = 0; // temporary storage

// parameters:
//  img - input video frame
//  dst - resultant motion picture
//  args - optional parameters
void  update_mhi( IplImage* img, IplImage* dst, int diff_threshold )
{
	double timestamp = (double)clock()/CLOCKS_PER_SEC; // get current time in seconds
	CvSize size = cvSize(img->width,img->height); // get current frame size
	int i, idx1 = last, idx2;
	IplImage* silh;
	CvSeq* seq;
	CvRect comp_rect;
	double count;
	double angle;
	CvPoint center;
	double magnitude;
	CvScalar color;

	// allocate images at the beginning or
	// reallocate them if the frame size is changed
	if( !mhi || mhi->width != size.width || mhi->height != size.height ) {
		if( buf == 0 ) {
			buf = (IplImage**)malloc(N*sizeof(buf[0]));
			memset( buf, 0, N*sizeof(buf[0]));
		}

		for( i = 0; i < N; i++ ) {
			cvReleaseImage( &buf[i] );
			buf[i] = cvCreateImage( size, IPL_DEPTH_8U, 1 );
			cvZero( buf[i] );
		}
		cvReleaseImage( &mhi );
		cvReleaseImage( &orient );
		cvReleaseImage( &segmask );
		cvReleaseImage( &mask );

		mhi = cvCreateImage( size, IPL_DEPTH_32F, 1 );
		cvZero( mhi ); // clear MHI at the beginning
		orient = cvCreateImage( size, IPL_DEPTH_32F, 1 );
		segmask = cvCreateImage( size, IPL_DEPTH_32F, 1 );
		mask = cvCreateImage( size, IPL_DEPTH_8U, 1 );
	}

	cvCvtColor( img, buf[last], CV_BGR2GRAY ); // convert frame to grayscale


	idx2 = (last + 1) % N; // index of (last - (N-1))th frame
	last = idx2;

	silh = buf[idx2];
	cvAbsDiff( buf[idx1], buf[idx2], silh ); // get difference between frames

	cvThreshold( silh, silh, diff_threshold, 1, CV_THRESH_BINARY ); // and threshold it
	cvUpdateMotionHistory( silh, mhi, timestamp, MHI_DURATION ); // update MHI

	// convert MHI to blue 8u image
	cvCvtScale( mhi, mask, 255./MHI_DURATION,
		(MHI_DURATION - timestamp)*255./MHI_DURATION );
	cvZero( dst );
	cvMerge( mask, 0, 0, 0, dst );

	// calculate motion gradient orientation and valid orientation mask
	cvCalcMotionGradient( mhi, mask, orient, MAX_TIME_DELTA, MIN_TIME_DELTA, 3 );

	if( !storage )
		storage = cvCreateMemStorage(0);
	else
		cvClearMemStorage(storage);

	// segment motion: get sequence of motion components
	// segmask is marked motion components map. It is not used further
	seq = cvSegmentMotion( mhi, segmask, storage, timestamp, MAX_TIME_DELTA );

	// iterate through the motion components,
	// One more iteration (i == -1) corresponds to the whole image (global motion)
	for( i = -1; i < seq->total; i++ ) {

		if( i < 0 ) { // case of the whole image
			comp_rect = cvRect( 0, 0, size.width, size.height );
			color = CV_RGB(255,255,255);
			magnitude = 100;
		}
		else { // i-th motion component
			comp_rect = ((CvConnectedComp*)cvGetSeqElem( seq, i ))->rect;
			if( comp_rect.width + comp_rect.height < 100 ) // reject very small components
				continue;
			color = CV_RGB(255,0,0);
			magnitude = 30;
		}

		// select component ROI
		cvSetImageROI( silh, comp_rect );
		cvSetImageROI( mhi, comp_rect );
		cvSetImageROI( orient, comp_rect );
		cvSetImageROI( mask, comp_rect );

		// calculate orientation
		angle = cvCalcGlobalOrientation( orient, mask, mhi, timestamp, MHI_DURATION);
		angle = 360.0 - angle;  // adjust for images with top-left origin

		count = cvNorm( silh, 0, CV_L1, 0 ); // calculate number of points within silhouette ROI

		cvResetImageROI( mhi );
		cvResetImageROI( orient );
		cvResetImageROI( mask );
		cvResetImageROI( silh );

		// check for the case of little motion
		if( count < comp_rect.width*comp_rect.height * 0.05 )
			continue;

		// draw a clock with arrow indicating the direction
		center = cvPoint( (comp_rect.x + comp_rect.width/2),
			(comp_rect.y + comp_rect.height/2) );

		cvCircle( dst, center, cvRound(magnitude*1.2), color, 3, CV_AA, 0 );
		cvLine( dst, center, cvPoint( cvRound( center.x + magnitude*cos(angle*CV_PI/180)),
			cvRound( center.y - magnitude*sin(angle*CV_PI/180))), color, 3, CV_AA, 0 );
	}
}