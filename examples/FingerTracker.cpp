#include "FingerTracker.h"


FingerTracker::FingerTracker(void)
{
	//m_device.Shutdown();
}


FingerTracker::~FingerTracker(void)
{
}

void FingerTracker::ProcessEvent( const FingerTrackingEvent &event )
{
	throw std::exception("The method or operation is not implemented.");
}
