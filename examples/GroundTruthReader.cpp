#include "GroundTruthReader.h"
#include "ofUtils.h"

void GroundTruthReader::setup()
{
	//ofSetFrameRate(100);
	ofSetVerticalSync(true);
}

void GroundTruthReader::dragEvent( ofDragInfo dragInfo )
{
	vector<string>& files = dragInfo.files;
	for (int i = 0; i < files.size(); i++)
	{
		const string& filename = files[i];
		cout << "loading:" << filename << endl;
		ofFile file(filename);
		if (file.getExtension() == "oni")
		{
			if(depthStream.isValid()) depthStream.exit();
			if(colorStream.isValid()) colorStream.exit();
			if(oniDevice.isValid()) oniDevice.exit();

			oniDevice.setup(file.getAbsolutePath().c_str());
			depthStream.setup(oniDevice.getDevice());
			colorStream.setup(oniDevice.getDevice());


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

						truth.selections[timestamp] =  position;
					}
				}
		
			}
		}



	}
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
