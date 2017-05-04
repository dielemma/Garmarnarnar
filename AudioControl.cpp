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

	// ofLog(OF_LOG_NOTICE,"BOX_X="+ofToString(box.x));

	this->nSamp = 200;
	this->bufferSize = 512;
    sampleRate = 44100;
	volHist.assign(nSamp,0.0);
    
    graphMaxSize = 200; // approx 7.5sec at 60fps
    
    graphLow.assign(graphMaxSize, 0.0);
    graphMid.assign(graphMaxSize, 0.0);
    graphHigh.assign(graphMaxSize, 0.0);
    graphRMS.assign(graphMaxSize, 0.0);
    
    gainLow = 1;
    gainMid = 1;
    gainHigh = 1;
    gainRMS = 1;
    
    vertOffsetLow = 0;
    vertOffsetMid = 0;
    vertOffsetHigh = 0;
    vertOffsetRMS = 0;
    
    rangeLow.setup(10, 21, 256);
    rangeMid.setup(125, 136, 256);
    rangeHigh.setup(235, 246, 256);
    
    smoothing = 0.5;
    
    //mesh_rms.setMode(OF_PRIMITIVE_LINE_STRIP);

    try
    {
    	audioAnalyzer.setup(sampleRate, bufferSize, 1);
	}	
	catch (const std::exception& ex)
	{
		ofLog(OF_LOG_ERROR,"AC setup Error:" + ofToString(ex.what()));
	}

    }

void AudioControl::draw()
{
    
    // ~~~ Spectrum ~~~ //
    
    // Not sure where this should go...
    
    ofPushMatrix();
    
    int graphH = 75;
    int yoffset = graphH + 50;
    int xpos = box.x - 280;
    int ypos = box.y + 230;
    int mw = 250;
    
    ofSetColor(255);
    ofDrawBitmapString("Spectrum: ", xpos, ypos);
    ofPushMatrix();
    ofTranslate(xpos, ypos);
    ofSetColor(255, 255, 100,200);
    float bin_w = (float) mw / spectrum.size();
    for (int i = 0; i < spectrum.size(); i++){
        if(i >= rangeHigh.lowerBound && i <= rangeHigh.upperBound)
            ofSetColor(255, 0, 100,200);
        else if(i >= rangeMid.lowerBound && i <= rangeMid.upperBound)
            ofSetColor(0, 150, 100,200);
        else if(i >= rangeLow.lowerBound && i <= rangeLow.upperBound)
            ofSetColor(0, 100, 255,200);
        else
            ofSetColor(255, 255, 100,200);
        float scaledValue = ofMap(spectrum[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
        float bin_h = -1 * (scaledValue * graphH);
        ofDrawRectangle(i*bin_w, graphH, bin_w, bin_h);
    }
    
    ofSetLineWidth(5);
    
    ofSetColor(255, 100, 0,200);
    ofDrawLine(rangeHigh.lowerBound*bin_w, graphH + 5 , rangeHigh.upperBound*bin_w, graphH + 5);
    ofSetColor(0, 255, 100,200);
    ofDrawLine(rangeMid.lowerBound*bin_w, graphH + 10, rangeMid.upperBound*bin_w, graphH + 10);
    ofSetColor(100, 100, 255, 255);
    ofDrawLine(rangeLow.lowerBound*bin_w, graphH + 15, rangeLow.upperBound*bin_w, graphH + 15);
    
    ofPopMatrix();
    
    // ~~~ End of Spectrum ~~~ //
    
    ofSetLineWidth(1);

	const int dy = 15;
	ofSetColor(255);
	ofDrawBitmapString("Audio Device:", box.x + 2, box.y + dy);
	ofSetLogLevel(OF_LOG_ERROR);
	deviceMenu->draw();
	ofSetLogLevel(OF_LOG_NOTICE);

	ofDrawBitmapString("Volume="+ofToString(round(100*rms)/100 + vertOffsetRMS), box.x + box.width - 120, box.y + dy);

	ofNoFill();
	ofSetColor(200,200,200);
	ofDrawRectangle(box);
    
    // try{
		ofDrawBitmapString("LOW - g: "+ofToString(gainLow)+" vo: "+ofToString(vertOffsetLow)+" th: "+ofToString(lowTrigger.threshold)+" tf: "+ofToString(lowTrigger.timeFrame),box.x+5,box.y+50);
	    drawAvgGraph(box.x + 20 , box.y + 60 , graphLow, ofColor(0, 100, 255,200), gainLow, vertOffsetLow, lowTrigger.threshold);
	    
	    ofDrawBitmapString("MID - g: "+ofToString(gainMid)+" vo: "+ofToString(vertOffsetMid)+" th: "+ofToString(midTrigger.threshold)+" tf: "+ofToString(midTrigger.timeFrame),box.x+275,box.y+50);
	    drawAvgGraph(box.x + 250, box.y + 60, graphMid, ofColor(0, 255, 100,200), gainMid, vertOffsetMid, midTrigger.threshold);
	    
	    ofDrawBitmapString("HIGH - g: "+ofToString(gainHigh)+" vo: "+ofToString(vertOffsetHigh)+" th: "+ofToString(highTrigger.threshold)+" tf: "+ofToString(highTrigger.timeFrame),box.x+5,box.y+200);
	    drawAvgGraph(box.x + 20, box.y + 210, graphHigh, ofColor(255, 0, 100,200), gainHigh, vertOffsetHigh, highTrigger.threshold);

        ofDrawBitmapString("RMS - g: "+ofToString(gainRMS)+" vo: "+ofToString(vertOffsetRMS)+" th: "+ofToString(rmsTrigger.threshold)+" tf: "+ofToString(rmsTrigger.timeFrame),box.x+275,box.y+200);
    
        drawAvgGraph(box.x + 250, box.y + 210, graphRMS, ofColor(255, 255, 100,200), gainHigh, vertOffsetHigh, highTrigger.threshold);
    
    /*
	    ofPushMatrix();
	    ofTranslate(box.x+250, box.y+210);
    
	    //mesh_rms.drawWireframe();
        //ofSetColor(255, 0, 0);
        //ofDrawLine(0,ofMap(waveformTrigger.threshold, 0, 1, graphMaxSize, 0,true) , graphMaxSize, ofMap(waveformTrigger.threshold, 0, 1, graphMaxSize, 0,true));
	    ofPopMatrix();
     */
	    
	   // ofDrawBitmapString("Low "+ofToString(avgLow), box.x + box.width - 120, box.y + dy + 20);
	   // ofDrawBitmapString("Mid "+ofToString(avgMid), box.x + box.width - 120, box.y + dy + 40);
	   // ofDrawBitmapString("High "+ofToString(avgHigh), box.x + box.width - 120, box.y + dy + 60);
	// }
	// catch (const std::exception& ex)
	// {
	// 	ofLog(OF_LOG_ERROR,"AC draw Error:" + ofToString(ex.what()));
	// }

	
}

void AudioControl::update()
{
	deviceMenu->update();
    

    try{


	    if (graphHigh.size()>nSamp-1) {
	        graphHigh.erase(graphHigh.begin(), graphHigh.begin()+1);
	        graphMid.erase(graphMid.begin(), graphMid.begin()+1);
	        graphLow.erase(graphLow.begin(), graphLow.begin()+1);
	        graphRMS.erase(graphRMS.begin(),graphRMS.begin()+1);
	    }
	    
	    //rms     = audioAnalyzer.getValue(RMS, 0, smoothing);
	    
	    spectrum = audioAnalyzer.getValues(SPECTRUM, 0, smoothing);
        
	    avgLow = 0;
	    avgMid = 0;
	    avgHigh = 0;
	    
        /*
	    for(int i=1; i<spectrum.size(); i++){
	        if (i<spectrum.size()*.333) {
	        	// avgLow = ofMap(spectrum[1], DB_MIN, DB_MAX/2, 0.0, 1.0, true);
	            avgLow = avgLow + ofMap(spectrum[i], DB_MIN, DB_MAX, 0.0, 2.0, true);
	        }
	        if (i>spectrum.size()*.33 && i<spectrum.size()*.666) {
	            avgMid = avgMid + ofMap(spectrum[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
	        }
	        if (i>spectrum.size()*.666 && i<spectrum.size()) {
	            avgHigh = avgHigh + ofMap(spectrum[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
	        }
	    }
	    
	    avgLow = avgLow/(spectrum.size()*.333);
	    avgMid = avgMid/(spectrum.size()*.333);
	    avgHigh = avgHigh/(spectrum.size()*.333);
        */
        
        for(int i=rangeLow.lowerBound; i<= rangeLow.upperBound; i++){
            avgLow = avgLow + ofMap(spectrum[i], DB_MIN, DB_MAX, 0.0, 2.0, true);
        }
        for(int i=rangeMid.lowerBound; i<= rangeMid.upperBound; i++){
            avgMid = avgMid + ofMap(spectrum[i], DB_MIN, DB_MAX, 0.0, 2.0, true);
        }
        for(int i=rangeHigh.lowerBound; i<= rangeHigh.upperBound; i++){
            avgHigh = avgHigh + ofMap(spectrum[i], DB_MIN, DB_MAX, 0.0, 2.0, true);
        }
        
        avgLow = avgLow/(rangeLow.upperBound-rangeLow.lowerBound+1);
        avgMid = avgMid/(rangeMid.upperBound-rangeMid.lowerBound+1);
        avgHigh = avgHigh/(rangeHigh.upperBound-rangeHigh.lowerBound+1);
        
        lowTrigger.test(gainLow*avgLow + vertOffsetLow);
        midTrigger.test(gainMid*avgMid + vertOffsetMid);
        highTrigger.test(gainHigh*avgHigh + vertOffsetHigh);
        rmsTrigger.test(gainRMS*rms + vertOffsetRMS);
	    
	    graphLow.push_back(avgLow);
	    graphMid.push_back(avgMid);
	    graphHigh.push_back(avgHigh);
	    graphRMS.push_back(rms);

        /*
	    mesh_rms.clear();
	    for (int i=0; i<nSamp; i++)
	    {
	    	mesh_rms.addVertex(ofVec2f(i,50-gainWaveform*volHist[i]-vertOffsetWaveform));
	    }
        */
	}
	catch (const std::exception& ex)
	{
		ofLog(OF_LOG_ERROR,"AC update Error:" + ofToString(ex.what()));
	}
}

void AudioControl::drawAvgGraph(int x, int y, vector<float> values, ofColor _color, float gain, float offset, float threshold){
    // ofEnableAlphaBlending();
    // ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    ofPushMatrix();
    ofFill();
    ofSetColor(_color);
    ofTranslate(x, y);
    ofBeginShape();
    
    for (int i = 0; i < (int)ofMap(values.size(), 0 , values.size(), 0,graphMaxSize); i++){ //scale it to be 150px wide
        if( i == 0 ) ofVertex(i, 100);
        
        ofVertex(i,ofMap(values[(int)ofMap(i, 0 , graphMaxSize, 0,values.size())], 0, 1, 100, 0,true));
        
        if( i == graphMaxSize -1 ) ofVertex(i, 100);
    }
    
    ofEndShape(false);
    ofSetColor(255, 255, 255);
    ofDrawLine(0,ofMap(threshold, 0, 1, 100, 0, true) , graphMaxSize, ofMap(threshold, 0, 1, 100, 0, true));
    ofPopMatrix();

    // ofDisableAlphaBlending();
    // ofDisableBlendMode();
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

void AudioControl::audioIn(ofSoundBuffer &inBuffer)
{
    audioAnalyzer.analyze(inBuffer);
    
	float curVol = 0.0;
    
    int numCounted = 0;

	for (int i = 0; i < bufferSize; i++){
        float curSample = inBuffer.getSample(i, 0);
		curVol += curSample * curSample;
        numCounted++;
	}
    
    curVol /= (float)numCounted;
    
    curVol = sqrt( curVol );

    
    rms = curVol;
    
    //ofLog(OF_LOG_NOTICE,ofToString(rms));
    volHist.erase(volHist.begin(),volHist.begin()+1);
    volHist.push_back(100*curVol);
	
    
	bufferCounter++;
	
}

AudioControl::~AudioControl()
{
	ofSoundStreamStop();
    ofSoundStreamClose();

}
