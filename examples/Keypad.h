#pragma once

#include "ofMain.h"

class Keypad : public ofBaseApp
{
public:
	Keypad(void);
	~Keypad(void);

	bool isActive;

	virtual void draw();

	virtual void keyPressed( int key );

private:
	
	int nX, nY;

	bool xFlip;
	bool yFlip;

	vector<unsigned long> timestamps;


};

