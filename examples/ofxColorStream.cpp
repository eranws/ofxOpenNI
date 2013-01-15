#include "ofxColorStream.h"
#include "OpenNI.h"


void ofxColorStream::setup(ofPtr<openni::Device> device, bool isVerbose)
{
	this->device = device;

	openni::Status rc;

	if (device->getSensorInfo(openni::SENSOR_COLOR) != NULL)
	{
		stream = ofPtr<openni::VideoStream>(new openni::VideoStream);
		rc = stream->create(*device, openni::SENSOR_COLOR);
		if (rc != openni::STATUS_OK)
		{
			throw ("Couldn't create color stream\n%s\n", openni::OpenNI::getExtendedError());
		}
	}

	allocateBuffers();

	rc = stream->start();
	if (rc != openni::STATUS_OK)
	{
		throw ("Couldn't start the color stream\n%s\n", openni::OpenNI::getExtendedError());

	}

	//startThread(false, isVerbose);
}

void ofxColorStream ::exit()
{
	stopThread();
	waitForThread();

	stream->stop();
	stream->destroy();
}

void ofxColorStream ::threadedFunction()
{
	while (isThreadRunning())
	{
		readFrame();
	}
}

void ofxColorStream::allocateBuffers()
{
	int w = stream->getVideoMode().getResolutionX();
	int h = stream->getVideoMode().getResolutionY();

	for (int i = 0; i < 2; i++)
	{
		pixels[i] = ofPtr<ofPixels>(new ofPixels);
		pixels[i]->allocate(w, h, OF_IMAGE_COLOR);
	}
//	texture.allocate(w, h, GL_RGB);
}

bool ofxColorStream::isValid()
{
	return (getStream().use_count() > 0 && getStream()->isValid());
}

int ofxColorStream::readFrame()
{
	openni::VideoFrameRef frame;
	openni::Status rc = stream->readFrame(&frame);
	if (rc != openni::STATUS_OK)
	{
		printf("Wait failed\n");
		return rc;
	}

	if (frame.getVideoMode().getPixelFormat() != openni::PIXEL_FORMAT_RGB888)
	{
		printf("Unexpected frame format\n");
		return rc;
	}

	openni::RGB888Pixel* pcolor = (openni::RGB888Pixel*)frame.getData();
	int middleIndex = (frame.getHeight()+1)*frame.getWidth()/2;

	//printf("[%08llu] %8d fps:%d\n", (long long)frame.getTimestamp(), pcolor[middleIndex].r, stream->getVideoMode().getFps());

	pixels[1]->setFromPixels((const unsigned char*)frame.getData(), pixels[1]->getWidth(), pixels[1]->getHeight(), OF_IMAGE_COLOR);
	swap(pixels[0], pixels[1]);

	return openni::STATUS_OK;
}



