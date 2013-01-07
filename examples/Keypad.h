#pragma once

#include "ofMain.h"


class Keypad : public ofBaseApp
{
	static const string MODULE_NAME;

public:
	Keypad(void);
	~Keypad(void);

	bool isActive;

	virtual void draw();

	virtual void keyPressed( int key );
	
	string toString();

	void keypadPreselect(int mappedKey, int delay = 0);
	void keypadPressed(int mappedKey);
private:
	
	int rows, cols;

	bool xFlip;
	bool yFlip;

	vector<unsigned long> timestamps;
	vector<unsigned long> preSelect;

	int lastPreselect;

};

