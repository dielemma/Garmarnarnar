#ifndef AudioControl_h
#define AudioControl_h

#include "ofMain.h"
#include "ofxDatGui.h"

class AudioControl: public ofBaseApp
{

public:
	AudioControl()
	{		
	};

	~AudioControl();

	void setup(const ofRectangle &r);
	void update();
	void draw();
	void audioIn(float * input, int bufferSize, int nChannels);

	ofRectangle box;

	ofSoundStream sstream;

	vector<string> deviceNameList;
	vector<ofSoundDevice> deviceList;
	ofxDatGuiDropdown* deviceMenu;

	int bufferSize;
	int bufferCounter;
	int drawCounter;
	int nSamp;
	
	float smoothedVol;
	float scaledVol;
	float curLeftAmp;
	float curRightAmp;
	float curLeftVol;
	float curRightVol;

	vector <float> leftChan;
	vector <float> leftAmp;
	vector <float> rightChan;
	vector <float> rightAmp;
	vector <float> volHist;

	void onDropdownEvent(ofxDatGuiDropdownEvent e);

};

#endif