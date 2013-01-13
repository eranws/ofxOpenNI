#include "ofxOpenNi2.h"

#include "OpenNI.h"

ofxOpenNi2::ofxOpenNi2(void)
{
	openni::Status rc = openni::OpenNI::initialize();
	if (rc != ONI_STATUS_OK)
	{
		throw ("Initialize failed\n%s\n", openni::OpenNI::getExtendedError());
	}
}


ofxOpenNi2::~ofxOpenNi2(void)
{
	openni::OpenNI::shutdown();
}
