#include "Keypad.h"
#include "Poco\Ascii.h"


//const char keyMap[] = "qwerasdfzxcv";
const char keyMap[] = "zxcvasdfqwer";
const int keyMapSize = sizeof(keyMap) - 1;


Keypad::Keypad(void)
{
	nX = 4;
	nY = 3;

	xFlip = false;
	yFlip = true;

	timestamps.assign(nX * nY, 0);
	preSelect.assign(nX * nY, false);
}


Keypad::~Keypad(void)
{
}

void Keypad::draw()
{
	if (!isActive) return;

	const int xSpacing = 30;
	const int ySpacing = 30;

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
			int index = i + j * nX;

			ofColor c;
			if (preSelect[index])
			{
				c = ofColor(255, 0, 0);
			}
			else
			{
				//fade
				float p = ofMap(now - timestamps[index], 0, 200, 0, 1, true);
				
				if (p > 0.99f)
				{
					c = 255;
				}
				else
				{
					c = ofColor(0, (1-p) * 255, (1-p) * 255);
				}
			}

			ofSetColor(c);

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
		if (Poco::Ascii::toLower(key) == keyMap[k])
		{
			int i = k % nX;
			int j = k / nX;
			int mappedKey = (xFlip ? (nX-i-1) : i) + (yFlip ? (nY-1-j) : j) * nX;
			
			if (Poco::Ascii::isUpper(key))
			{
				preSelect[mappedKey] = !preSelect[mappedKey];
			}
			else 
			{
				timestamps[mappedKey] = ofGetSystemTime();
				preSelect[mappedKey] = false;
			}

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
	case 'M': yFlip = !yFlip; break;
	case 'N': xFlip = !xFlip; break;

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
		preSelect[mappedKey] = false;
		break;
	}

}
