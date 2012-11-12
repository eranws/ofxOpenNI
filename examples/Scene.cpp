#include "Scene.h"


Scene::Scene(void)
{
	screen.setGlobalPosition(0,0,0);
	screen.color = ofColor::red; 

	sensor.setGlobalPosition(0, 250, 0);
	sensor.color = ofColor::yellow; 

}


Scene::~Scene(void)
{
}

void Scene::customDraw()
{
	sensor.draw();
	screen.draw();

}


void SceneItem::customDraw()
{

	ofFill();
	ofSetColor(color);
	ofBox(1);

	ofNoFill();
	ofSetLineWidth(2);
	ofSetColor(color.getInverted());
	ofBox(1);

}

