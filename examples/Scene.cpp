#include "Scene.h"


Scene::Scene(void)
{
	screen.setGlobalPosition(0, -50-163, 0);
	screen.color = ofColor::red; 

	sensor.setGlobalPosition(0, 0, 0);
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

// http://mathworld.wolfram.com/Line-PlaneIntersection.html
ofPoint Screen::getIntersectionPointWithLine( ofPoint p4, ofPoint p5 )
{
	ofPoint hs = getScale() / 2; //= halfScale 
	ofPoint p1 = getPosition() + ofPoint(+hs.x, +hs.y, 0);
	ofPoint p2 = getPosition() + ofPoint(-hs.x, +hs.y, 0);
	ofPoint p3 = getPosition() + ofPoint(-hs.x, -hs.y, 0);
	
	float t =
		-ofMatrix4x4(
		1   ,1   ,1   ,1   ,
		p1.x,p2.x,p3.x,p4.x,
		p1.y,p2.y,p3.y,p4.y,
		p1.z,p2.z,p3.z,p4.z
		).determinant()
		/
		ofMatrix4x4(
		1   ,1   ,1   ,0   ,
		p1.x,p2.x,p3.x,p5.x-p4.x,
		p1.y,p2.y,p3.y,p5.y-p4.y,
		p1.z,p2.z,p3.z,p5.z-p4.z
		).determinant();

	ofPoint res = p4 + (p5-p4) * t;

	return res;
}
