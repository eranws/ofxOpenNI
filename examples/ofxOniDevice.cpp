#include "ofxOniDevice.h"
#include "OpenNI.h"

void ofxOniDevice::setup()
{
	using namespace openni;

	openni::Status rc = openni::OpenNI::initialize();
	if (rc != ONI_STATUS_OK)
	{
		throw ("Initialize failed\n%s\n", openni::OpenNI::getExtendedError());
	}

	device = ofPtr<openni::Device>(new openni::Device);
	rc = device->open(ANY_DEVICE);
	if (rc != ONI_STATUS_OK)
	{
		throw ("Couldn't open device\n%s\n", OpenNI::getExtendedError());
	}

}

void ofxOniDevice::exit()
{
	device->close();
	openni::OpenNI::shutdown();
}
