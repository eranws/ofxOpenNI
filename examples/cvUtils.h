
#define showMat(x) showMatR(x, 1)
#define showMatR(x, i)	\
{						\
	cv::Mat y;			\
	cv::resize(x, y, cv::Size(), i, i, cv::INTER_LINEAR);\
	cv::imshow(#x, y);	\
}


cv::Point2f g_hand_fingerPoint2f; // in handFrame coordinates
cv::Point2f g_fingerPoint2f; //in frameCoordinates

cv::Point2f g_handPoint2f, g_prevHandPoint;
bool g_hasFinger = false;



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




void detectFinger(cv::Mat& handFrame, const cv::Rect& handRect)
{

	cv::Mat handLabel;
	if (handFrame.channels() > 1)
	{
		cv::cvtColor(handFrame, handLabel, cv::COLOR_BGR2GRAY);
	}
	else
	{
		handFrame.copyTo(handLabel);
	}
	cv::inRange(handLabel, 1, 254, handLabel);
	// find peaks
	cv::Mat dst;// = cv::Mat::zeros(handLabel.size(), CV_8UC3);
	cv::cvtColor(handLabel, dst, CV_GRAY2RGB);

	//cv::Mat element = getStructuringElement(cv::MORPH_CROSS,cv::Size( 1*2 + 1 , 1*2 + 1 ),cv::Point( 1, 1 ) );
	//cv::dilate(handLabel, handLabel, element, cv::Point(-1,-1), 1);
	//cv::erode(handLabel, handLabel, element, cv::Point(-1, -1), 1);
	//	cv::medianBlur(handLabel, handLabel, 3);	

	//Extract the contours so that
	/*std::vector<std::vector<cv::Point> > contours0;
	cv::findContours(handLabel, contours0, g_hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

	g_contours.resize(contours0.size());
	for( size_t k = 0; k < contours0.size(); k++ )
	{
	cv::approxPolyDP(cv::Mat(contours0[k]), g_contours[k], 3, false);
	}

	on_contours_trackbar(0,0);*/


	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(handLabel.clone(), contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

	//approximate with lines
	for( size_t k = 0; k < contours.size(); k++ )
		approxPolyDP(cv::Mat(contours[k]), contours[k], 3, true);


	//cv::Mat handCut = cv::Mat::zeros(handLabel.size(), CV_8UC1);	
	if( !contours.empty() && !hierarchy.empty() )
	{


		// iterate through all the top-level contours,
		// draw each connected component with its own random color

		std::vector<cv::Point2f> fingerPoint; 

		int idx = 0;
		for(int i = 0; i < hierarchy.size(); i++)
		{
			//find biggest cc
			if (cv::contourArea(contours[idx]) < cv::contourArea(contours[i]))
				idx = i;
		}



		cv::Scalar white(255, 255, 255);
		cv::Scalar blue(255,0,0);
		cv::Scalar green(0,255,0);
		cv::Scalar red(0,0,255);
		cv::Scalar black(0,0,0);

		cv::drawContours( dst, contours, idx, blue, 2, 8, hierarchy );

		if(cv::contourArea(contours[idx]) > 15)
		{
			fingerPoint.push_back(contours[idx][0]);
			for (int i = 1; i < contours[idx].size();i++)
			{
				//check if peak
				cv::Point2f vec1 = contours[idx][i - 1] - contours[idx][i];
				cv::Point2f vec2 = contours[idx][(i + 1) % contours[idx].size()] - contours[idx][i];

				vec1.x = vec1.x / cv::norm(vec1); vec1.y = vec1.y / cv::norm(vec1);
				vec2.x = vec2.x / cv::norm(vec2); vec2.y = vec2.y / cv::norm(vec2);

				if ( vec1.dot(vec2) < 0.1 || vec1.y < 0 )
					continue;

				if (g_hasFinger)
				{
					float prevDist = cv::norm(fingerPoint.back() - (g_hand_fingerPoint2f - (g_handPoint2f - g_prevHandPoint)));
					float curDist = cv::norm(cv::Point2f(contours[idx][i]) - (g_hand_fingerPoint2f - (g_handPoint2f - g_prevHandPoint)));

					if (curDist > prevDist)
						continue;
				}
				else
				{
					if ( fingerPoint.back().y < contours[idx][i].y)
					{
						continue;
					}
				}

				fingerPoint.back() = contours[idx][i];

			}
		}


		int minIdx = 0;
		float minNorm = 1000;
		if (fingerPoint.size())
		{
			if (g_hasFinger)
			{
				// get closest to prev finger pos
				for (int i = 0; i < fingerPoint.size(); i++)
				{
					float curNorm = cv::norm(fingerPoint[i] - (g_hand_fingerPoint2f - (g_handPoint2f - g_prevHandPoint)));
					if ( curNorm < minNorm)
					{
						minNorm = curNorm;
						minIdx = i;
					}
				}

			}
			else
			{
				// get highest 
				for (int i = 0; i < fingerPoint.size(); i++)
				{
					float curNorm = fingerPoint[i].y;
					if ( curNorm < minNorm)
					{
						minNorm = curNorm;
						minIdx = i;
					}
				}
			}

			g_hand_fingerPoint2f = fingerPoint[minIdx];


			cv::Rect r;
			const int S = 30;
			r.x = g_hand_fingerPoint2f.x - S;
			r.y = g_hand_fingerPoint2f.y - S;
			r.width = 2 * S;
			r.height = 2 * S;

			//find center of mass
			if (r.x > 0 && r.y > 0 && r.x + r.width < handLabel.cols && r.y + r.height < handLabel.rows)
			{
				cv::Point2f g_hand_fingerComPoint2f;

				cv::rectangle(dst, r, green, 2);

				cv::Mat fing;
				handLabel(r).copyTo(fing);

				std::vector<std::vector<cv::Point> > fingContours;
				std::vector<cv::Vec4i> fingHierarchy;

				cv::findContours( fing, fingContours, fingHierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE ); //Find the Contour BLOBS
				vector<cv::Moments> _mu(fingContours.size() );
				vector<cv::Point2f> _mc( fingContours.size() );
				for( int i = 0; i < fingContours.size(); i++ )
				{ 
					_mu[i] = cv::moments( cv::Mat(fingContours[i]), false );
					_mc[i] = cv::Point2f( _mu[i].m10/_mu[i].m00 , _mu[i].m01/_mu[i].m00);
				}

				
				if (_mc.size() > 0)
				{
					cv::Point2f pt;
					float maxDist = 1e10;
					for(int i = 0; i < _mc.size(); i++)
					{
						//avoid zero elements
						if (_mc[i].x > 0 && _mc[i].y > 0)
						{
							pt = _mc[i];
							cv::Point2f d = pt - g_hand_fingerPoint2f;
							float dist = d.dot(d);
							if (dist < maxDist)
							{
								maxDist = dist;

								g_hand_fingerComPoint2f = cv::Point2f(r.x, r.y) + pt;
								
								g_fingerPoint2f = cv::Point2f(handRect.tl()) + g_hand_fingerComPoint2f;
							}
						
						}
					}
					
				}
				else
				{
					g_fingerPoint2f = cv::Point2f(handRect.tl()) + g_hand_fingerPoint2f;
				}

				g_hasFinger = true;

				cv::circle(dst, g_hand_fingerComPoint2f, 3, green, CV_FILLED);
				cv::circle(dst, g_hand_fingerPoint2f, 3, red, CV_FILLED);
				

			}
			else
			{
				g_hasFinger = false;
			}

		}
		else
		{
			g_hasFinger = false;
		}
	} 
	else
	{
		g_hasFinger = false;
	}

	showMat(dst);
	showMat(handLabel);

	//cv::Mat handEdges;
	//cv::Canny(handCut, handEdges, 60, 3*60);
	//showMat(handEdges);
}