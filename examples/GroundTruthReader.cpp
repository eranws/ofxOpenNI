#include "GroundTruthReader.h"
#include "ofUtils.h"

#include "OpenNI.h"

openni::PlaybackControl* pbc;

bool extFilter(string s)
{
	ofFile file(s);
	bool b = file.getExtension() == "oni"; 
	return b;
}

void GroundTruthReader::setup()
{
	//ofSetFrameRate(100);
	ofSetVerticalSync(true);
	setupLeftGui();
}

void GroundTruthReader::dragEvent( ofDragInfo dragInfo )
{
	vector<string>& files = dragInfo.files;
	loadFile(files[0]);
}

void GroundTruthReader::draw()
{
	if (colorStream.isValid())
	{
		ofTexture colorTexture;
		ofPixels colorPixels = colorStream.getPixels(); 
		colorTexture.allocate(colorPixels);
		colorTexture.loadData(colorPixels);

		float ratio = colorTexture.getWidth() / colorTexture.getHeight(); 
		colorTexture.draw(0,0, ofGetWindowHeight() * ratio, ofGetWindowHeight());
	}
}


void GroundTruthReader::setupLeftGui()
{

	float dim = 16; 
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING; 
	float length = 255-xInit; 

	gui = ofPtr<ofxUICanvas>(new ofxUICanvas(0, 0, length+xInit, ofGetHeight())); 

	vector<string> names;
	vector<string>namesFiltered;

	Poco::File dir(ofToDataPath("."));
	dir.list(names);

	for (int i=0; i < names.size(); i++)
	{
		if (extFilter(names[i]))
			namesFiltered.push_back(names[i]);
	}

	gui->addSpacer(length-xInit, 2);
	gui->addRadio("recordings", namesFiltered, OFX_UI_ORIENTATION_VERTICAL, dim, dim); 

	gui->setColorBack(ofColor::gray);
	gui->setColorFill(ofColor::gray);
	gui->setDrawFill(true);

	ofAddListener(gui->newGUIEvent,this,&GroundTruthReader::guiEvent);
}


void GroundTruthReader::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName(); 
	int kind = e.widget->getKind(); 
	cout << "got event from: " << name << endl; 	

	ofxUIWidget* parent = e.widget->getParent();

	if (parent != NULL)
	{
		if (parent->getName() == "recordings")
		{
			loadFile(name);
		}
		else if (parent->getName() == "clicks")
		{
			//ofxUIRadio* clicks = (ofxUIRadio*)parent;
			ofxUIToggle* toggle = (ofxUIToggle*)e.widget;
			
			ClickMap::const_iterator first = truth.clickMap.cbegin();
			ClickMap::const_iterator last = truth.clickMap.cend();
			ClickMap::const_iterator it;
			iterator_traits<ClickMap::const_iterator>::difference_type count, step;

			it = first;
			advance(it, ofToInt(name));
			unsigned long value = it->first;

			count = distance(first,last);
			while (count>0)
			{
				it = first; step=count/2; advance (it,step);
				if (it->first < value)                   // or: if (comp(*it,value)), for the comp version
				{ first=++it; count-=step+1;  }
				else count=step;
			}
			
			cout << it->first << " " << it->second << endl;
			
			//cout << it->second << endl;
			//seekTimestamp(name);
		}
	}


	else if(name == "SEEKBAR")
	{
		ofxUISlider* slider = (ofxUISlider*) e.widget; 


		int frame = (int) slider->getScaledValue();
		cout << "slider->getScaledValue()" << frame << endl;

		pbc->seek(*depthStream.getStream().get(), frame);
		depthStream.readFrame();
		colorStream.readFrame();

		//bdrawGrid = button->getValue(); 
	}


}


void GroundTruthReader::mouseMoved( int x, int y )
{
	const int egdeSize = 30;
	if (x < egdeSize && !gui->isVisible())	
		gui->setVisible(true);

	if (x > gui->getRect()->width)
		gui->setVisible(false);

	if (bottomPanel != NULL)
	{
		if (y > ofGetWindowHeight() - egdeSize && !bottomPanel->isVisible())	
			bottomPanel->setVisible(true);

		else if (!bottomPanel->isHit(x,y))
			bottomPanel->setVisible(false);
	}

	if (rightPanel != NULL)
	{
		if (x > ofGetWindowWidth() - egdeSize && !rightPanel->isVisible())	
			rightPanel->setVisible(true);

		else if (!bottomPanel->isHit(x,y))
			rightPanel->setVisible(false);
	}



}

void GroundTruthReader::keyPressed( int key )
{
	switch (key)
	{
	case 'f': ofToggleFullscreen(); break;
	}
}

void GroundTruthReader::loadFile( string filename )
{
	cout << "loading file:" << filename << endl;

	ofFile file(filename);
	if (file.getExtension() == "oni")
	{
		if(depthStream.isValid()) depthStream.exit();
		if(colorStream.isValid()) colorStream.exit();
		if(oniDevice.isValid()) oniDevice.exit();
		oniDevice.setup(file.getAbsolutePath().c_str());		depthStream.setup(oniDevice.getDevice());		colorStream.setup(oniDevice.getDevice());

		depthStream.readFrame();
		colorStream.readFrame();

		// parse ground truth text file
		ofFile fileTruth(file.getBaseName() + ".txt");
		if (fileTruth.exists())
		{
			/*
			//reading file in this format:
			testApp: verbose: 261212382 recording started 
			testApp: verbose: 261212382 rows: 5 cols:3 
			Keypad: verbose: 261215152 Preselect:	 5 
			Keypad: verbose: 261216023 Pressed:	 5 
			Keypad: verbose: 261224268 Preselect:	 4 
			Keypad: verbose: 261225192 Pressed:	 4 
			Keypad: verbose: 261226082 Preselect:	 11 
			Keypad: verbose: 261226743 Pressed:	 11 
			Keypad: verbose: 261228184 Preselect:	 11 
			...
			*/

			truth.clear();

			ofBuffer buf = fileTruth.readToBuffer();
			vector<string> words = ofSplitString(buf.getText(), " ", true, true);

			string recWord = "recording";
			unsigned int startTimeIndex = ofFind(words, recWord);

			truth.startTime = ofToInt(words[startTimeIndex-1]);

			string colsWord = "rows:"; //bug, already fixed in code
			unsigned int colsIndex = ofFind(words, colsWord);

			truth.gridWidth = ofToInt(words[colsIndex + 1]);
			truth.gridHeight = 3;

			buf.resetLineReader();
			while (!buf.isLastLine())
			{
				string line = buf.getNextLine();
				printf("%s\n",line.c_str());

				string pressedWord = "Pressed:";

				vector<string> lineWords = ofSplitString(line, " ", true, true);

				unsigned int pressedIndex = ofFind(lineWords, pressedWord);
				if (pressedIndex != lineWords.size())
				{
					int timestamp = ofToInt(lineWords[pressedIndex - 1]);
					int position = ofToInt(lineWords[pressedIndex + 1]);

					truth.clickMap[timestamp] =  position;

				}
			}

		}


		// TODO: encapsulate in a ofxPlaybackControl object
		pbc = oniDevice.getDevice()->getPlaybackControl();
		nFrames = pbc->getNumberOfFrames(*depthStream.getStream().get());
		openni::VideoFrameRef pFrame;
		pbc->setSpeed(0);
		pbc->seek(*depthStream.getStream().get(), 0);

		timestamps.clear();
		for (int i = 0; i < nFrames; i+=10)
		{
			cout << "read frame " << i << endl;
			pbc->seek(*depthStream.getStream().get(), i);
			depthStream.getStream()->readFrame(&pFrame);
			timestamps[pFrame.getTimestamp()] = pFrame.getFrameIndex();
		}



		//GUI
		setupBottomGui();
		setupRightGui();

	}
}

void GroundTruthReader::setupBottomGui()
{
	float dim = 16; 
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING; 
	float length = ofGetWindowWidth(); 
	float height = 100-xInit; 

	bottomPanel = ofPtr<ofxUICanvas>(new ofxUICanvas(0, ofGetWindowHeight() - height, length, height)); 
	bottomPanel->addWidgetDown(new ofxUILabel("SeekBar", OFX_UI_FONT_MEDIUM)); 				

	ofxUISlider* seekBar = bottomPanel->addSlider("SEEKBAR", 0, nFrames, 0.0, length-xInit, dim);
	seekBar->setLabelPrecision(0);

	bottomPanel->setColorBack(ofColor::gray);
	bottomPanel->setColorFill(ofColor::gray);
	bottomPanel->setDrawFill(true);

	ofAddListener(bottomPanel->newGUIEvent, this, &GroundTruthReader::guiEvent);

}

void GroundTruthReader::setupRightGui()
{

	float dim = 16; 
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING; 
	float width = 255-xInit; 

	leftPanel = ofPtr<ofxUICanvas>(new ofxUICanvas(ofGetWindowWidth() - width, 0, width+xInit, ofGetHeight())); 

	vector <string> truthSelectionsStr;
	for (int i=0; i < truth.clickMap.size(); i++)
	{
		string s;
		s += ofToString(i);
		//s += ofToString(truth.selections.at(i));

		truthSelectionsStr.push_back(s);
	}
	//selections = gui->addRadio("clicks", truthSelectionsStr, OFX_UI_ORIENTATION_VERTICAL, dim, dim); 
	leftPanel->addRadio("clicks", truthSelectionsStr, OFX_UI_ORIENTATION_VERTICAL, dim, dim); 

	leftPanel->setColorBack(ofColor::gray);
	leftPanel->setColorFill(ofColor::gray);
	leftPanel->setDrawFill(true);

	ofAddListener(leftPanel->newGUIEvent,this,&GroundTruthReader::guiEvent);
}

