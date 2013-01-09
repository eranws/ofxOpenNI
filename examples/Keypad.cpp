#include "Keypad.h"
#include "Poco\Ascii.h"
#include "ofLog.h"
#include <assert.h>

const char keyMap[] = "zxcasdqwe";
//const char keyMap[] = "zxcvasdfqwer";
const int keyMapSize = sizeof(keyMap) - 1;

const string Keypad::MODULE_NAME = "Keypad";

Keypad::Keypad(void)
{
	ofSetLogLevel(MODULE_NAME, OF_LOG_VERBOSE);

	cols = 5;
	rows = 3;

	lastPreselect = -1;

	xFlip = false;
	yFlip = true;

	timestamps.assign(cols * rows, 0);
	preSelect.assign(cols * rows, 0);

	visible = false;
}


Keypad::~Keypad(void)
{
}

void Keypad::draw()
{
	if (!visible) return;

	const int xSpacing = 30;
	const int ySpacing = 30;

	ofPushStyle();

	ofBackground(0);
	ofFill();
	ofSetColor(255);

	int w = ofGetWindowWidth() / cols - 2 * xSpacing;
	int h = ofGetWindowHeight() / rows - 2 * ySpacing;
	unsigned long now = ofGetSystemTime();

	for (int i = 0; i < cols; i++)
	{
		for (int j = 0; j < rows; j++)
		{
			int index = i + j * cols;

			ofColor c;
			const int fadeInTime = 500;
			int diff = int(now - preSelect[index]);
			if (preSelect[index] > 0 && diff > -fadeInTime)
			{
				float p = ofMap(diff, 0, fadeInTime, 0, 1, true);
				
				c = ofColor::white.getLerped(ofColor::red, p);
			}
			
			else
			{
				//fade out
				float p = ofMap(now - timestamps[index], 0, 200, 0, 1, true);
				
				p = powf(p, 3);
				c = ofColor::black.getLerped(ofColor::white, p);
			}

			ofSetColor(c);

			ofRectangle button(xSpacing + i * ofGetWindowWidth() / cols, ySpacing + j * ofGetWindowHeight() / rows, w, h);
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
			int i = k % cols;
			int j = k / cols;
			int mappedKey = (xFlip ? (cols-i-1) : i) + (yFlip ? (rows-1-j) : j) * cols;

			if (Poco::Ascii::isUpper(key))
			{
				keypadPreselect(mappedKey);
			}
			else 
			{
				keypadPressed(mappedKey);				
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
		{
			int k = key - '1';
			int i = k % cols;
			int j = k / cols;
			int mappedKey = (xFlip ? (cols-i-1) : i) + (yFlip ? (rows-1-j) : j) * cols;
			keypadPressed(mappedKey);				
			break;
		}

	case ' ':
		if (lastPreselect == -1)
		{
			const int delay = 0;//1000;
			keypadPreselect(ofRandom(cols * rows), delay);
		}
		else
		{
			keypadPressed(lastPreselect);
		}
		break;

	case '+':
		if (lastPreselect != -1)
		{
			keypadPressed(lastPreselect);
			const int delay = 0;
			keypadPreselect(ofRandom(cols * rows), delay);
		}
		break;


	}

}

string Keypad::toString()
{
	return "rows: " + ofToString(cols) + " " + "cols:" + ofToString(rows);
}

void Keypad::keypadPreselect(int mappedKey, int delay)
{
	assert(mappedKey >= 0 && mappedKey < cols * rows);
	lastPreselect = mappedKey;

	unsigned long now = ofGetSystemTime();
	preSelect[mappedKey] = now + delay;

	ofLogVerbose(MODULE_NAME) << now << "Preselect:\t" << mappedKey;
}

void Keypad::keypadPressed( int mappedKey )
{
	assert(mappedKey >= 0 && mappedKey < cols * rows);
	lastPreselect = -1;

	unsigned long now = ofGetSystemTime();
	timestamps[mappedKey] = now;
	preSelect[mappedKey] = 0;

	ofLogVerbose(MODULE_NAME) << now << "Pressed:\t" << mappedKey;
}
