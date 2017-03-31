#ifndef AudioControl_h
#define AudioControl_h

#include "ofMain.h"
#include "ofxDatGui.h"
#include "ofEvents.h"
#include "ofxAudioAnalyzer.h"
#include <chrono>

struct thresholdTrigger{
    float threshold;
    float timeFrame;
    float lastTimeTriggered;
    
    bool triggered;
    
    ofEvent<float> aboveThreshold;
    
    thresholdTrigger(){
        lastTimeTriggered = ofGetElapsedTimeMillis();
        triggered = false;
    }
    
    void test(float value){
        if (value >= threshold) {
            if(!triggered){
                ofNotifyEvent(aboveThreshold, value);
                triggered = true;
                lastTimeTriggered = ofGetElapsedTimeMillis();
            }
            else if((ofGetElapsedTimeMillis() - lastTimeTriggered) > timeFrame){
                ofNotifyEvent(aboveThreshold, value);
                lastTimeTriggered = ofGetElapsedTimeMillis();
            }
        }
        else {
            if(triggered){
                if((ofGetElapsedTimeMillis() - lastTimeTriggered) > timeFrame)
                    triggered = false;
            }
        }
    }
};

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
	void audioIn(ofSoundBuffer &inBuffer); //void audioIn(float * input, int bufferSize, int nChannels);
    
	ofRectangle box;

	ofSoundStream sstream;
    
    ofxAudioAnalyzer audioAnalyzer;

	vector<string> deviceNameList;
	vector<ofSoundDevice> deviceList;
	ofxDatGuiDropdown* deviceMenu;

	int bufferSize;
    int sampleRate;
	int bufferCounter;
	int drawCounter;
	int nSamp;
    int graphMaxSize;
	
	float smoothedVol;
	float scaledVol;
	float curLeftAmp;
	float curRightAmp;
	float curLeftVol;
	float curRightVol;
    float smoothing;
    
    float rms;
    vector<float> melBands;

    
    float  avgLow;
    float  avgMid;
    float  avgHigh;
    
    vector<float> graphLow;
    vector<float> graphMid;
    vector<float> graphHigh;
    vector<float> graphRMS;

	vector <float> leftChan;
	vector <float> leftAmp;
	vector <float> rightChan;
	vector <float> rightAmp;
	vector <float> volHist;

	void onDropdownEvent(ofxDatGuiDropdownEvent e);
    void drawAvgGraph(int x, int y, vector<float> values, ofColor _color);


};

#endif