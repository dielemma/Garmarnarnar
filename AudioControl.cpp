#include "AudioControl.h"

void AudioControl::setup(const ofRectangle &r)
{

	this-> box = r;

	deviceList = sstream.getDeviceList();
	deviceNameList.resize(deviceList.size());
	for (uint i=0; i<deviceList.size(); i++)
	{
		deviceNameList[i] = deviceList[i].name;
	}

	deviceMenu = new ofxDatGuiDropdown("Audio Device", deviceNameList);
	deviceMenu -> setPosition(box.x + 110, box.y);
	deviceMenu -> onDropdownEvent(this, &AudioControl::onDropdownEvent);

	ofLog(OF_LOG_NOTICE,"BOX_X="+ofToString(box.x));

	this->nSamp = 400;
	this->bufferSize = 256;
    sampleRate = 44100;
	leftChan.assign(bufferSize, 0.0);
	rightChan.assign(bufferSize, 0.0);
	leftAmp.assign(nSamp, 0.0);
	rightAmp.assign(nSamp, 0.0);
	volHist.assign(nSamp,0.0);
    
    graphMaxSize = 150; // approx 7.5sec at 60fps
    
    graphLow.assign(graphMaxSize, 0.0);
    graphMid.assign(graphMaxSize, 0.0);
    graphHigh.assign(graphMaxSize, 0.0);
    graphRMS.assign(graphMaxSize, 0.0);
    
    smoothing = 0;
    
    audioAnalyzer.setup(sampleRate, bufferSize, 1);

    }

void AudioControl::draw()
{

	const int dy = 15;
	ofDrawBitmapString("Audio Device:", box.x + 2, box.y + dy);
	ofSetLogLevel(OF_LOG_ERROR);
	deviceMenu->draw();
	ofSetLogLevel(OF_LOG_NOTICE);

	ofDrawBitmapString("Volume="+ofToString(round(100*smoothedVol)/100), box.x + box.width - 120, box.y + dy);

	ofNoFill();
	ofSetColor(200,200,200);
	ofDrawRectangle(box);
    
    drawAvgGraph(box.x + 60 , box.y + 40 , graphLow, ofColor(0, 100, 255,200));
    drawAvgGraph(box.x + 220, box.y + 40, graphMid, ofColor(0, 255, 100,200));
    drawAvgGraph(box.x + 60, box.y + 150, graphHigh, ofColor(255, 0, 100,200));
    drawAvgGraph(box.x + 220, box.y + 150, graphRMS, ofColor(255, 100, 255,200));
    
    ofDrawBitmapString("Low "+ofToString(avgLow), box.x + box.width - 120, box.y + dy + 20);
    ofDrawBitmapString("Mid "+ofToString(avgMid), box.x + box.width - 120, box.y + dy + 40);
    ofDrawBitmapString("High "+ofToString(avgHigh), box.x + box.width - 120, box.y + dy + 60);

}

void AudioControl::update()
{
	deviceMenu->update();
    
    if (graphHigh.size()>graphMaxSize) {
        graphHigh.erase(graphHigh.begin(), graphHigh.begin()+1);
        graphMid.erase(graphMid.begin(), graphMid.begin()+1);
        graphLow.erase(graphLow.begin(), graphLow.begin()+1);
        graphRMS.erase(graphRMS.begin(),graphRMS.begin()+1);
    }
    
    rms     = audioAnalyzer.getValue(RMS, 0, smoothing, true);
    melBands = audioAnalyzer.getValues(MEL_BANDS, 0, smoothing);
    
    smoothedVol = rms;
    
    avgLow = 0;
    avgMid = 0;
    avgHigh = 0;
    
    for(int i=0; i<melBands.size(); i++){
        if (i<melBands.size()*.333) {
            avgLow = avgLow + ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
        }
        if (i>melBands.size()*.33 && i<melBands.size()*.666) {
            avgMid = avgMid + ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
        }
        if (i>melBands.size()*.666 && i<melBands.size()) {
            avgHigh = avgHigh + ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
        }
    }
    
    avgLow = avgLow/(melBands.size()*.333);
    avgMid = avgMid/(melBands.size()*.333);
    avgHigh = avgHigh/(melBands.size()*.333);
    
    graphLow.push_back(avgLow);
    graphMid.push_back(avgMid);
    graphHigh.push_back(avgHigh);
    graphRMS.push_back(rms);
}

void AudioControl::drawAvgGraph(int x, int y, vector<float> values, ofColor _color){
    ofEnableAlphaBlending();
    ofPushMatrix();
    ofFill();
    ofSetColor(_color);
    ofTranslate(x, y);
    ofBeginShape();
    
    for (int i = 0; i < (int)ofMap(values.size(), 0 , values.size(), 0,150); i++){ //scale it to be 150px wide
        if( i == 0 ) ofVertex(i, 100);
        
        ofVertex(i,ofMap(values[(int)ofMap(i, 0 , 150, 0,values.size())], 0, 1, 100, 0,true));
        
        if( i == 150 -1 ) ofVertex(i, 100);
    }
    
    
    ofEndShape(false);
    ofSetColor(255);
    ofPopMatrix();
    ofDisableAlphaBlending();
}

void AudioControl::onDropdownEvent(ofxDatGuiDropdownEvent e)
{
	try
	{
		ofLog(OF_LOG_NOTICE,"ECHILD="+ofToString(e.child));
		sstream.setDeviceID(e.child);
		sstream.setup(this, 0, 1, 44100, bufferSize, 4);

	}
	catch (const std::exception& ex)
	{
		string errMsg = "Error initializing audio device.\n" + ofToString(ex.what());
		ofLog(OF_LOG_ERROR) << errMsg;

		//ofSystemAlertDialog(errMsg);
	}
}

//void AudioControl::audioIn(float * input, int bufferSize, int nChannels)
void AudioControl::audioIn(ofSoundBuffer &inBuffer)
{
    audioAnalyzer.analyze(inBuffer);
    /*
	float curVol = 0.0;
	
	// samples are "interleaved"
	int numCounted = 0;	

	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume	
	for (int i = 0; i < bufferSize; i++){
		leftChan[i]		= input[i*2]*0.5;
		rightChan[i]	= input[i*2+1]*0.5;

		curVol += leftChan[i] * leftChan[i];
		curVol += rightChan[i] * rightChan[i];
		numCounted+=2;
	}
	
	//this is how we get the mean of rms :) 
	curVol /= (float)numCounted;
	
	// this is how we get the root of rms :) 
	curVol = sqrt( curVol );
	
	smoothedVol *= 0.93;
	smoothedVol += 0.07 * curVol;
	*/
    
	bufferCounter++;
	
}

AudioControl::~AudioControl()
{
	ofSoundStreamStop();
    ofSoundStreamClose();

}
