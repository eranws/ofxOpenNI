#include "ofxOpenNi2.h"

using namespace openni;
//using namespace ofxOpenNi2;

int ofxOpenNI::init()
{
	openni::Status rc;
	rc = OpenNI::initialize();
	if (rc != openni::STATUS_OK)
	{
		printf("Initialize failed\n%s\n", OpenNI::getExtendedError());
		return 1;
	}
	Array<DeviceInfo> deviceInfoList;
	OpenNI::enumerateDevices(&deviceInfoList);

	for (int i =0; i < deviceInfoList.getSize(); i++)
	{
		printf("%s\n", deviceInfoList[i].getUri());
	}

}

int wtf::setup( bool threaded /*= true*/ )
{
	//OpenNI::addListener(this);

	rc = device.open(openni::ANY_DEVICE);
	if (rc != openni::STATUS_OK)
	{
		printf("Couldn't open device\n%s\n", OpenNI::getExtendedError());
		return 2;
	}

	VideoStream depth;
	if (device.getSensorInfo(openni::SENSOR_DEPTH) != NULL)
	{
		rc = depth.create(device, openni::SENSOR_DEPTH);
		if (rc != openni::STATUS_OK)
		{
			printf("Couldn't create depth stream\n%s\n", OpenNI::getExtendedError());
		}
	}
	rc = depth.start();
	if (rc != openni::STATUS_OK)
	{
		printf("Couldn't start the depth stream\n%s\n", OpenNI::getExtendedError());
	}
	
	// Register to new frame
	depth.addListener(&depthStream);

	VideoStream color;
	if (device.getSensorInfo(openni::SENSOR_COLOR) != NULL)
	{
		rc = color.create(device, SENSOR_COLOR);
		if (rc != ONI_STATUS_OK)
		{
			printf("Couldn't create color stream\n%s\n", OpenNI::getExtendedError());
		}
	}
	rc = color.start();
	if (rc == openni::STATUS_OK)
	{
		colorStream.setup(color.getVideoMode());
	}
	else
	{
		printf("Couldn't start the color stream\n%s\n", OpenNI::getExtendedError());
	}
	color.addListener(&colorStream);



	rc = device.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);
	if (rc != ONI_STATUS_OK)
	{
		printf("setImageRegistrationMode Error: \n%s\n", OpenNI::getExtendedError());
	}





	return 0;

}



ofPixels& ofxVideoStream::getPixels()
{
	return pixels[0];
}


void ofxColorStream::onNewFrame(openni::VideoStream& stream)
{
	openni::VideoFrameRef frame;
	stream.readFrame(&frame);

	openni::RGB888Pixel* pColor;

	int middleIndex = (frame.getHeight()+1)*frame.getWidth()/2;

	switch (frame.getVideoMode().getPixelFormat())
	{
	case openni::PIXEL_FORMAT_RGB888:
		pColor = (openni::RGB888Pixel*)frame.getData();
		printf("[%08llu] 0x%02x%02x%02x\n", (long long)frame.getTimestamp(),
			pColor[middleIndex].r&0xff,
			pColor[middleIndex].g&0xff,
			pColor[middleIndex].b&0xff);
		break;

	default:
		printf("Unknown format\n");
	}
	
	//TODO: read to pixels
}

void ofxVideoStream::update()
{
	
}

void ofxVideoStream::setup(openni::VideoMode& videoMode)
{
	int w = videoMode.getResolutionX();
	int h = videoMode.getResolutionY();
	texture.allocate(w, h, GL_RGB);
	allocatePixels(w, h);
}

void ofxColorStream::allocatePixels(int w, int h)
{
	//colorPixelsDoubleBuffer[i] = new ofPixels();
	//colorPixelsDoubleBuffer[i]->allocate(dw, dh, OF_IMAGE_COLOR);
}


void ofxDepthStream::allocatePixels(int w, int h)
{
	for (int i=0 ; i<2; i++)
	{
		pixels[i].allocate(w, h, OF_IMAGE_GRAYSCALE);
	}
}

void ofxDepthStream::onNewFrame( openni::VideoStream& )
{

}
