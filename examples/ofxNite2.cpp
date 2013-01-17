#include "ofxNite2.h"

#include "NiTE.h"

ofxNite2::ofxNite2(void)
{
	nite::Status niteRc = nite::STATUS_OK;
	niteRc = nite::NiTE::initialize();
	if (niteRc != nite::STATUS_OK)
	{
		throw ("NiTE initialization failed\n");
	}

}


ofxNite2::~ofxNite2(void)
{
	nite::NiTE::shutdown();
}
