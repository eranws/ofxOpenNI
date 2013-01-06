#include "Keypad.h"


const char keyMap[] = "qwerasdfzxcv";
const int keyMapSize = sizeof(keyMap) - 1;


Keypad::Keypad(void)
{
	nX = 3;
	nY = 3;

	xFlip = false;
	yFlip = true;

	timestamps.assign(nX * nY, 0);
}


Keypad::~Keypad(void)
{
}

void Keypad::draw()
{
	if (!isActive) return;

	const int xSpacing = 10;
	const int ySpacing = 10;

	ofPushStyle();

	ofBackground(0);
	ofFill();
	ofSetColor(255);
	
	int w = ofGetWindowWidth() / nX - 2 * xSpacing;
	int h = ofGetWindowHeight() / nY - 2 * ySpacing;
	unsigned long now = ofGetSystemTime();

	for (int i = 0; i < nX; i++)
	{
		for (int j = 0; j < nY; j++)
		{
			int p = (now - timestamps[i + j * nX]) / 2;
			ofSetColor(p, 255, 255);
			ofRectangle button(xSpacing + i * ofGetWindowWidth() / nX, ySpacing + j * ofGetWindowHeight() / nY, w, h);
			ofRect(button);
		}
	}
	
	ofPopStyle();

}

void Keypad::keyPressed( int key )
{

	for (int k = 0; k < keyMapSize; k++)
	{
		if (key == keyMap[k])
		{
			int i = k % nX;
			int j = k / nX;
			int mappedKey = (xFlip ? (nX-i-1) : i) + (yFlip ? (nY-1-j) : j) * nX;
			timestamps[mappedKey] = ofGetSystemTime();
		}
	}


	switch(key)
	{
	/*
	case '+': nX < 10 && nX++; break;
	case '-': nX > 1  && nX--; break;
	case '*': nY < 10 && nY++; break;
	case '/': nY > 1 && nY--; break;
	*/
	case 'X': xFlip = !xFlip; break;
	case 'Y': yFlip = !yFlip; break;

	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		int k = key - '1';
		int i = k % nX;
		int j = k / nX;
		int mappedKey = (xFlip ? (nX-i-1) : i) + (yFlip ? (nY-1-j) : j) * nX;
		timestamps[mappedKey] = ofGetSystemTime();
		break;
	}

}
