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
        threshold = 1;
        timeFrame = 200;
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

struct freqRange{
    int lowerBound, upperBound, midpoint, maxUpper;

    freqRange():lowerBound(0), upperBound(2), maxUpper(2){}
    
    void setup(int l, int u, int m) {
        lowerBound = l;
        upperBound = u;
        maxUpper = m;
        if(l > u){
            throw std::invalid_argument("lowerBound must be less than or equal to upperBound");
        }
        else if(u > m){
            throw std::invalid_argument("upperBound must be less than or equal to maxUpper");
        }
    }
    
    void shiftRight(){
        if(maxUpper - upperBound > 1){
            lowerBound+=2;
            upperBound+=2;
        }
        else if (maxUpper - upperBound == 1){
            lowerBound++;
            upperBound++;
        }
    }
    
    void shiftLeft(){
        if(lowerBound > 1){
            lowerBound-=2;
            upperBound-=2;
        }
        else if (lowerBound == 1) {
            lowerBound--;
            upperBound--;
        }
    }
    
    void grow(){
        if(upperBound==maxUpper){
            lowerBound -= 2;
        }
        else if(lowerBound==0){
            upperBound += 2;
        }
        else{
            lowerBound--;
            upperBound++;
        }
    }
    
    void shrink(){
        if(upperBound - lowerBound > 1){
            lowerBound++;
            upperBound--;
        }
        else if(upperBound - lowerBound == 1){
            upperBound--;
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
    ofxDatGuiButton* bgMeasButton;
    void onButtonEvent(ofxDatGuiButtonEvent e);

	int bufferSize;
    int sampleRate;
	int bufferCounter;
	int drawCounter;
	int nSamp;
    int graphMaxSize;

    ofMesh mesh_rms;
	
	float scaledVol;
	float curLeftAmp;
	float curRightAmp;
	float curLeftVol;
	float curRightVol;
    float smoothing;
    
    float rms;
    vector<float> spectrum;

    
    float  avgLow;
    float  avgMid;
    float  avgHigh;
    
    float gainLow;
    float gainMid;
    float gainHigh;
    float gainRMS;
    
    float vertOffsetLow;
    float vertOffsetMid;
    float vertOffsetHigh;
    float vertOffsetRMS;
    
    vector<float> graphLow;
    vector<float> graphMid;
    vector<float> graphHigh;
    vector<float> graphRMS;

	vector <float> leftChan;
	vector <float> leftAmp;
	vector <float> rightChan;
	vector <float> rightAmp;
	vector <float> volHist;
    
    thresholdTrigger lowTrigger;
    thresholdTrigger midTrigger;
    thresholdTrigger highTrigger;
    thresholdTrigger rmsTrigger;
    
    freqRange rangeLow;
    freqRange rangeMid;
    freqRange rangeHigh;

	void onDropdownEvent(ofxDatGuiDropdownEvent e);
    void drawAvgGraph(int x, int y, vector<float> values, ofColor _color, float gain, float offset, float threshold);
};

#endif
