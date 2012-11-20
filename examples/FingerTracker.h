#pragma once

#include "EVAFingerTracker.h"
#include "FingerTrackingEvent.h"

#include "EVACommonData.h"

class FingerTracker : public FingerTrackingEventListener // ref: FingerTrackingDummyApp
{
public:
	FingerTracker(void);
	~FingerTracker(void);

	void init();
	void shutdown();

	virtual void ProcessEvent( const FingerTrackingEvent &event );

	
private:
	EVAFingerTracker *m_fingerTracker;
	OpenNIDevice m_device;
	EVACommonData *m_data;

	//FingerTrackingDummyApp* this... //ref: m_dummyApp



};

