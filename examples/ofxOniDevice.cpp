#include "ofxOniDevice.h"
#include "OpenNI.h"

void ofxOniDevice::setup(const char* uri)
{
	using namespace openni;

	device = ofPtr<openni::Device>(new openni::Device);
	openni::Status rc = device->open(uri);
	if (rc != ONI_STATUS_OK)
	{
		throw std::exception(OpenNI::getExtendedError());
	}

}

void ofxOniDevice::exit()
{
	device->close();
}

void ofxOniDevice::setRegistration( bool b )
{
	device->isImageRegistrationModeSupported(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
	device->setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
	
}

bool ofxOniDevice::isValid()
{
	return (device.use_count() > 0 && device->isValid());
}

