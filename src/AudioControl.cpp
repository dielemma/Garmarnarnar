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
	leftChan.assign(bufferSize, 0.0);
	rightChan.assign(bufferSize, 0.0);
	leftAmp.assign(nSamp, 0.0);
	rightAmp.assign(nSamp, 0.0);
	volHist.assign(nSamp,0.0);


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
}

void AudioControl::update()
{
	deviceMenu->update();
}

void AudioControl::onDropdownEvent(ofxDatGuiDropdownEvent e)
{
	try
	{
		ofLog(OF_LOG_NOTICE,"ECHILD="+ofToString(e.child));
		sstream.setDeviceID(e.child);
		sstream.setup(this, 0, 2, 44100, bufferSize, 4);		

	}
	catch (const std::exception& ex)
	{
		string errMsg = "Error initializing audio device.\n" + ofToString(ex.what());
		ofLog(OF_LOG_ERROR) << errMsg;

		//ofSystemAlertDialog(errMsg);
	}
}

void AudioControl::audioIn(float * input, int bufferSize, int nChannels)
{
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
	
	bufferCounter++;
	
}

AudioControl::~AudioControl()
{
	ofSoundStreamStop();
    ofSoundStreamClose();

}