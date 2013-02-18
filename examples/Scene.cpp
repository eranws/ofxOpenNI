#include "Scene.h"
#include "ofAppRunner.h"


Scene::Scene(void)
{
//	screen.setGlobalPosition(0, -50-163, 0); //sensor on screen
	screen.setGlobalPosition(0, 65 + 163, 0);//sensor below screen
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
ofPoint Screen::getIntersectionPointWithLine( ofPoint p1, ofPoint p2, ofPoint p3, ofPoint p4, ofPoint p5)
{

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

ofPoint Screen::getIntersectionPointWithLine( ofPoint p4, ofPoint p5 )
{
	ofPoint hs = getScale() / 2; //= halfScale 
	ofPoint p1 = getPosition() + ofPoint(+hs.x, +hs.y, 0);
	ofPoint p2 = getPosition() + ofPoint(-hs.x, +hs.y, 0);
	ofPoint p3 = getPosition() + ofPoint(-hs.x, -hs.y, 0);
	return getIntersectionPointWithLine(p1, p2, p3, p4, p5);
}

ofVec2f Screen::getScreenPointFromWorld(ofPoint p)
{
	ofPoint hs = getScale() / 2; //= halfScale 
	ofPoint dp = getPosition() + ofPoint(+hs.x, +hs.y, 0);
	
	ofVec2f res((dp.x + p.x) * ofGetScreenWidth()  / getScale().x,
		(dp.y - p.y) * ofGetScreenHeight() / getScale().y);

	return res;
}

ofVec2f Screen::getScreenPointFromWorld(ofPoint tl, ofPoint tr, ofPoint bl, ofPoint q)
{
	ofVec3f vx = (tr-tl).normalize();
	ofVec3f vy = (bl-tl).normalize();
	ofVec3f v = (q-tl);

	float qMapX = v.dot(vx);
	float qMapY = v.dot(vy);

	ofVec2f res(qMapX * ofGetScreenWidth()  / getScale().x,  qMapY * ofGetScreenHeight() / getScale().y);


	return res;
}
