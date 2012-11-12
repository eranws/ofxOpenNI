#pragma once
#include "ofNode.h"


class SceneItem : public ofNode
{
public:
	virtual void customDraw();

	ofColor color;

};

class Sensor : public SceneItem
{
public:
	Sensor(void)
	{
		setScale(200, 15, 30);
	}
	//virtual ~Sensor(void){}

};

class Screen: public SceneItem
{
public:
	Screen(void)
	{
		setScale(559.7, 493.0, 201.5);
	}
	//virtual ~Screen(void){}

};





class Scene : public ofNode
{
public:
	Scene(void);
	virtual ~Scene(void);

	virtual void customDraw();

	Screen screen;
	Sensor sensor;


};

