#include "Keypad.h"


Keypad::Keypad(void)
{
	nX = 3;
	nY = 3;

	xFlip = false;
	yFlip = false;
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

	ofFill();
	ofSetColor(255);
	
	int w = ofGetWindowWidth() / nX - 2 * xSpacing;
	int h = ofGetWindowHeight() / nY - 2 * ySpacing;

	unsigned long now = ofGetSystemTime();


	for (int j = 0; j < nY; j++)
	{
		for (int i = 0; i < nX; i++)
		{
			ofSetColor((now - timestamp[(xFlip ? (2-i) : i) + (yFlip ? (2-j) : j)* nX]) / 2, 255, 255);
			ofRectangle button(xSpacing + i * ofGetWindowWidth() / nX, ySpacing + j * ofGetWindowHeight() / nY, w, h);
			ofRect(button);
		}
	}
	
	ofPopStyle();

}

void Keypad::keyPressed( int key )
{
	switch(key)
	{
	/*
	case '+': nX < 10 && nX++; break;
	case '-': nX > 1  && nX--; break;
	case '*': nY < 10 && nY++; break;
	case '/': nY > 1 && nY--; break;
	*/
	case 'x': xFlip = !xFlip; break;
	case 'y': yFlip = !yFlip; break;

	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		int intKey = key - '1';
		timestamp[intKey] = ofGetSystemTime();
		break;

	}

}
