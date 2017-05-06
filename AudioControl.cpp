#include "AudioControl.h"

void AudioControl::setup(const ofRectangle &r)
{
	ofLog(OF_LOG_NOTICE,"AudioControlSetup");
	this-> box = r;

	// GUI DROPDOWN
	deviceList = sstream.getDeviceList();
	deviceNameList.resize(deviceList.size());
	for (uint i=0; i<deviceList.size(); i++)
	{
		deviceNameList[i] = deviceList[i].name;
	}

	deviceMenu = new ofxDatGuiDropdown("Audio Device", deviceNameList);
	deviceMenu -> setPosition(box.x + 110, box.y);
	deviceMenu -> setWidth(200);
	deviceMenu -> onDropdownEvent(this, &AudioControl::onDropdownEvent);

	// GUI BUTTONS
	bgMeasButton = new ofxDatGuiButton("Measure BG");
	bgMeasButton -> setPosition(box.x + 350, box.y );
	bgMeasButton -> setWidth(100,0.9);
	bgMeasButton -> onButtonEvent(this, &AudioControl::onButtonEvent);

	toggleMel = new ofxDatGuiToggle("MEL Breakdown");
	toggleMel -> setPosition(box.x+ 500, box.y);
	toggleMel -> setWidth(100,0.9);
	toggleMel -> onButtonEvent(this, &AudioControl::onButtonEvent);


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
    
    spectrum_bg.assign(bufferSize/2, 0.0);
    rms_bg = 0;

    gainLow = 0.5;
    gainMid = 1;
    gainHigh = 1;
    gainRMS = 1;
    
    currentGain = &gainHigh;
    currentTrigger = &highTrigger;
    
    ofAddListener(ofEvents().mousePressed, this, &AudioControl::mousePressed);

    vertOffsetLow = 0;
    vertOffsetMid = 0;
    vertOffsetHigh = 0;
    vertOffsetRMS = 0;
    
    configBreakdowns();
    
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

    ofSetLineWidth(2);
    ofFill();

    xpos = 10;
    ypos = box.y + 230;
    ofDrawBitmapString("Mel Bands ("+ofToString(melBands.size())+"):",xpos,ypos);
    ofPushMatrix();
    ofTranslate(xpos,ypos);
    ofSetColor(ofColor::cyan);
    bin_w = (float) mw / melBands.size();
    for (int i = 0; i < melBands.size(); i++){
        float scaledValue = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
        float bin_h = -1 * (scaledValue * graphH);
        ofDrawRectangle(i*bin_w, graphH, bin_w, bin_h);
    }
    ofPopMatrix();
    
    // ~~~ End of Spectrum ~~~ //
    
    ofSetLineWidth(1);

	const int dy = 15;
	ofSetColor(255);
	ofDrawBitmapString("Audio Device:", box.x + 2, box.y + dy);
	ofSetLogLevel(OF_LOG_ERROR);
	deviceMenu->draw();
	bgMeasButton->draw();
	toggleMel->draw();
	ofSetLogLevel(OF_LOG_NOTICE);

	// ofDrawBitmapString("Volume="+ofToString(round(100*rms)/100 + vertOffsetRMS), box.x + box.width - 120, box.y + dy);

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
    
        drawAvgGraph(box.x + 250, box.y + 210, graphRMS, ofColor(255, 255, 100,200), gainRMS, vertOffsetHigh, rmsTrigger.threshold);
    
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
	bgMeasButton->update();
	toggleMel->update();

    try{


	    if (graphHigh.size()>nSamp-1) {
	        graphHigh.erase(graphHigh.begin(), graphHigh.begin()+1);
	        graphMid.erase(graphMid.begin(), graphMid.begin()+1);
	        graphLow.erase(graphLow.begin(), graphLow.begin()+1);
	        graphRMS.erase(graphRMS.begin(),graphRMS.begin()+1);
	    }
	    
	    
	    
	    spectrum = audioAnalyzer.getValues(SPECTRUM, 0, smoothing);
	    melBands = audioAnalyzer.getValues(MEL_BANDS, 0, smoothing);
	    double aa_rms = audioAnalyzer.getValue(RMS, 0, smoothing);

	    if (i_bg_samp < N_bg_samp)
	    {
	    	// measure background
	    	for (uint i=0; i<spectrum.size(); i++)
	    	{
	    		spectrum_bg[i] += spectrum[i];
	    		rms_bg += aa_rms;
	    	}
	    	i_bg_samp++;
	    	// ofLog(OF_LOG_NOTICE,"i_bg_samp="+ofToString(i_bg_samp));
	    	// ofLog(OF_LOG_NOTICE,"s0="+ofToString(spectrum[0])+"; bg0="+ofToString(spectrum_bg[0]));
	    	ofLog(OF_LOG_NOTICE,"rmsBG="+ofToString(rms_bg));

	    }
	    else
	    {
	    	// subract background
	    	// double sum = std::accumulate(spectrum_bg.begin(), spectrum_bg.end(), 0.0);
	    	for (uint i=0; i<spectrum.size(); i++)
	    	{

	    		spectrum[i] -= spectrum_bg[i]/N_bg_samp-DB_MIN;
	    		// const double linspec = 10*pow(10,spectrum[i]/10);
	    		// const double linbg = 10*pow(10,spectrum_bg[i]/N_bg_samp/10);
	    		// spectrum[i] = 10*log10(abs(linspec - linbg));

	    		aa_rms -= rms_bg/N_bg_samp;
	    	}
	    }
        
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
        if (useMel)
        {
        	for(int i=rangeLow.lowerBound; i<= rangeLow.upperBound; i++){
	            avgLow = avgLow + ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 2.0, true);
	        }
	        //avgLow /= (rangeLow.upperBound - rangeLow.lowerBound);

	        for(int i=rangeMid.lowerBound; i<= rangeMid.upperBound; i++){
	            avgMid = avgMid + ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 2.0, true);
	        }
	        //avgMid /= (rangeMid.upperBound - rangeMid.lowerBound);

	        for(int i=rangeHigh.lowerBound; i<= rangeHigh.upperBound; i++){
	            avgHigh = avgHigh + ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 2.0, true);
	        }
        }
        else
        {
	        for(int i=rangeLow.lowerBound; i<= rangeLow.upperBound; i++){
	            avgLow = avgLow + ofMap(spectrum[i], DB_MIN, DB_MAX, 0.0, 2.0, true);
	        }
	        //avgLow /= (rangeLow.upperBound - rangeLow.lowerBound);

	        for(int i=rangeMid.lowerBound; i<= rangeMid.upperBound; i++){
	            avgMid = avgMid + ofMap(spectrum[i], DB_MIN, DB_MAX, 0.0, 2.0, true);
	        }
	        //avgMid /= (rangeMid.upperBound - rangeMid.lowerBound);

	        for(int i=rangeHigh.lowerBound; i<= rangeHigh.upperBound; i++){
	            avgHigh = avgHigh + ofMap(spectrum[i], DB_MIN, DB_MAX, 0.0, 2.0, true);
	        }
	    }
        //avgHigh /= (rangeHigh.upperBound - rangeHigh.lowerBound);
        
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

	    
	    mfcc = audioAnalyzer.getValues(MFCC, 0, smoothing);

	    graphRMS.push_back(rms);
	    // graphRMS.push_back(audioAnalyzer.getValue(ENERGY, 0, smoothing));

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
        
        ofVertex(i,ofMap(gain*values[(int)ofMap(i, 0 , graphMaxSize, 0,values.size())], 0, 1, 100, 0,true));
        
        if( i == graphMaxSize -1 ) ofVertex(i, 100);
    }
    
    ofEndShape(false);
    ofSetColor(255, 255, 255);
    ofDrawLine(0,ofMap(threshold, 0, 1, 100, 0, true) , graphMaxSize, ofMap(threshold, 0, 1, 100, 0, true));
    ofPopMatrix();

    // ofDisableAlphaBlending();
    // ofDisableBlendMode();
}

void AudioControl::mousePressed(ofMouseEventArgs& eventArgs){
    int x = eventArgs.x;
    int y = eventArgs.y;
    if(x >= box.x+20 && x <= box.x+220) {
        if(y >= box.y+60 && y <= box.y+160) {
            currentGain = &gainLow;
            currentTrigger = &lowTrigger;
        }
        else if(y >= box.y+210 && y <= box.y+310){
            currentGain = &gainHigh;
            currentTrigger = &highTrigger;
        }
    }
    else if(x >= box.x+250 && x <= box.x+450){
        if(y >= box.y+60 && y <= box.y+160) {
            currentGain = &gainMid;
            currentTrigger = &midTrigger;
        }
        else if(y >= box.y+210 && y <= box.y+310){
            currentGain = &gainRMS;
            currentTrigger = &rmsTrigger;
        }
    }
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

void AudioControl::onButtonEvent(ofxDatGuiButtonEvent e)
{
	if (e.target == bgMeasButton)
	{
		// measure background
		ofLog(OF_LOG_NOTICE,"Measure Background");
		i_bg_samp = 0;
		rms_bg = 0.0;
		spectrum_bg.clear();
		spectrum_bg.assign(spectrum.size(), 0.0);
	}
	if (e.target == toggleMel)
	{
		useMel = !useMel;

		configBreakdowns();
	}
}

void AudioControl::configBreakdowns()
{
	if (useMel)
    {
    	toggleMel -> setLabel("MEL Breakdown");
    	rangeLow.setup(0, 7, 24);
	    rangeMid.setup(8, 15, 24);
	    rangeHigh.setup(16, 23, 24);
    }
    else
    {
    	toggleMel -> setLabel("FFT Breakdown");
	    rangeLow.setup(0, 1, 256);
	    rangeMid.setup(86, 170, 256);
	    rangeHigh.setup(174, 246, 256);
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

    
    // rms = curVol;
    double s = 1.0;
    rms = s*curVol + (1-s)*rms;
    
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
