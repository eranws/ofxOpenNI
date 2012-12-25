#pragma once
#include "ofTypes.h"

namespace openni
{
	class Device;
}

class ofxOniDevice
{

public:

	void setup();
	void exit();

	ofPtr<openni::Device> getDevice() const { return device; }

protected:
	ofPtr<openni::Device> device;

};

