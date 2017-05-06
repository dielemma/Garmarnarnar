#include "ofApp.h"

string sliderNames[] = { "crossfader", "channelLevel1", "channelLevel2" };
string knobNames[] = { "eqHi1", "eqMid1", "eqLow1", "eqHi2", "eqMid2", "eqLow2" };

//--------------------------------------------------------------
void ofApp::setup(){
	debugCircle = false;
	simpleSound = false;
	shapeCase=1;
	dColors.push_back(ofColor(255,0,0));
	dColors.push_back(ofColor(255));
	dColors.push_back(ofColor(46,140,90));
	dColors.push_back(ofColor(255,99,71));
	dColors.push_back(ofColor(430,144,255));
	dColors.push_back(ofColor(138,43,226));
	dColors.push_back(ofColor(169,169,169));
	dcolID = 0;


	//obj.set(100, 50, 100, 200);

	ofSetFrameRate(60);
	screenW = ofGetScreenWidth();
	screenH = ofGetScreenHeight()-40;	
	ofSetWindowPosition(0,0);
	ofSetWindowShape(screenW, screenH);
	
	ofSetBackgroundColor(0);
	
	ofSetLogLevel(OF_LOG_NOTICE);

	/////////////////////////////////////////////////////////////////////////////
	// SETUP BOXES ////////////////////////////////////////////
	const int bb = 8;
	const int ww = screenW;
	ledBox = ofRectangle(ww/3.0 + bb, bb, ww/3.0 - bb, (ww/3.0 - 2*bb)*0.5625);

	ledPreviewBox = ofRectangle(ww*2.0/3.0 + bb, bb, ww/3.0-bb, ledBox.height);

	soundBox = ofRectangle(ww/3.0 + bb, 2*bb + ledBox.height, ww/3.0 - bb, 320);

	paletteBox = ofRectangle(bb, 3*bb + ledBox.height + soundBox.height, screenW - 2 *bb, 320);

	midiControlBox = ofRectangle(ww*2.0/3.0 + bb, 2*bb + ledBox.height, ww/6.0-bb, 320 );

	effectInfoBox = ofRectangle(ww*5.0/6.0 + bb, 2*bb + ledBox.height, ww/6.0 -bb, 320);


	/////////////////////////////////////////////////////////////////////////////
	// MANUALLY CONFIGURE LED STRIPS //////////////////////////////
	Nstrips = 16;
	NperStrip = 30;	
	
	int spacing = 4;//round((ledBox.width-4)/(Nstrips/2-1)/(NperStrip-1)/cos(angle));
	float angle = acos((ledBox.width-4) / (Nstrips/2*(NperStrip)*spacing) );
	ofLog(OF_LOG_NOTICE,"angle = " + ofToString(angle));
	float L = (NperStrip-1)*spacing;
	float dx = L*cos(angle);
	
	float x0 = ledBox.x+2;
	float y0 = ledBox.y + ledBox.height/4.0;

	int id = 0;
	for (int i=0; i<8; i++)
	{
		float x = x0 + 0.5*dx + i*(dx + spacing * cos(angle));
		strips.push_back(make_shared<ofxLEDstrip>(id, NperStrip, x, y0, spacing, -pow(-1,i)*angle, false));
		if(i==1){
			ofLog(OF_LOG_NOTICE,"spacing =" + ofToString(spacing));
			ofLog(OF_LOG_NOTICE,"x=" + ofToString(x));

		}
		id += 30;
	}

	y0 = ledBox.y + 3.0*ledBox.height/4.0;
	for (int i=0; i<8; i++)
	{
		float x = x0 + 0.5*dx + i*(dx + spacing * cos(angle));
		strips.push_back(make_shared<ofxLEDstrip>(id, NperStrip, x, y0, spacing, pow(-1,i)*angle, false));
		id += 30;
	}


	/////////////////////////////////////////////////////////////////////////////
	// FADECANDY /////////////////////////////////////////////
	fc.setup("127.0.0.1", 7890, true, true);

	ofLog(OF_LOG_NOTICE, "ww="+ ofToString(ww));
	fc.setupStage(ledBox);
	// populate Fadecandy object with LED info
	int LEDcount = 0;
	for (int i=0; i<Nstrips; i++)
	{
		for (int j=0; j<strips[i]->count; j++)
		{
			fc.allLEDcoords.push_back(strips[i]->pixelCoords[j]);
			LEDcount++;
		}
	}
	fc.allLEDcolors.resize(LEDcount);

	fc.initFrameBuffer();

	/////////////////////////////////////////////////////////////////////////////
	// MIDI CONTROL /////////////////////////////////////////////
	midiDeviceNameList = midiIn.getPortList();
	midiIn.listPorts();
	midiIn.addListener(this);
	// midiIn.setVerbose(true);
	midiDeviceMenu = new ofxDatGuiDropdown("Midi Control Device", midiDeviceNameList);
	midiDeviceMenu -> setPosition(4 + midiControlBox.x, 20 + midiControlBox.y);
	midiDeviceMenu -> onDropdownEvent(this, &ofApp::onMidiMenuDropdownEvent);
	midiConnected = false;
	components = new DDJSB2Components(midiIn);
	components->populateFromXML("Pioneer_DDJ-SB2.xml");
	mastBrightKnob = components->getSlider("channelFader1");
	// efK1 = components->getKnob("eqHi1");
	effectKnobs.push_back(components->getKnob("eqHi1"));
	effectKnobs.push_back(components->getKnob("eqMid1"));
	effectKnobs.push_back(components->getKnob("eqLow1"));


	/////////////////////////////////////////////////////////////////////////////
	// vis FBO
	ledFrame = vizFBO("LED Frame", ofPoint(ledBox.x, ledBox.y), ofPoint(ledBox.width, ledBox.height));
	for (uint i=0; i< maxEffects; i++)
	{
		effectRenderFrames.push_back(vizFBO("",ofPoint(0,0),ofPoint(ledBox.width,ledBox.height)));
	}
	ledPreviewFrame = vizFBO("LED Preview Frame", ledPreviewBox);
	shaderFrame = vizFBO("Shader Effects", ofPoint(paletteBox.x, paletteBox.y), ofPoint(paletteBox.width, paletteBox.height) );
	midiControlFrame = vizFBO("Midi Controller", ofPoint(midiControlBox.x, midiControlBox.y), ofPoint(midiControlBox.width, midiControlBox.height));
	effectInfoFrame = vizFBO("Effect Info", effectInfoBox);

	/////////////////////////////////////////////////////////////////////////////
	// SHADERS /////////////////////////////
	// **TODO: these should be loaded at runtime from a config XML
	vector<ofColor> colorVect;
	colorVect.push_back(ofColor(80,12,60));
	colorVect.push_back(ofColor(19,85,189));
	effect_shaders.push_back(make_shared<Effect_shader>(this, "fire", ofPoint(0,8), ofPoint(20,20), colorVect, ledFrame.size));
	effect_shaders.push_back(make_shared<Effect_shader>(this, "clouds", ofPoint(20,8), ofPoint(20,20), colorVect, ledFrame.size));
	effect_shaders.push_back(make_shared<Effect_shader>(this, "psychNoise", ofPoint(40,8), ofPoint(20,20), colorVect, ledFrame.size));
	effect_shaders[effect_shaders.size()-1]->speed = 0.4;
	effect_shaders.push_back(make_shared<Effect_shader>(this, "lightning", ofPoint(60,8), ofPoint(20,20), colorVect, ledFrame.size));
	effect_shaders[effect_shaders.size()-1]->speed = 0.5;
	effect_shaders.push_back(make_shared<Effect_shader>(this, "vortex", ofPoint(80,8), ofPoint(20,20), colorVect, ledFrame.size));
	effect_shaders[effect_shaders.size()-1]->speed = 0.5;
	effect_shaders.push_back(make_shared<Effect_shader>(this, "interferingWaves", ofPoint(100,8), ofPoint(20,20), colorVect, ledFrame.size));
	effect_shaders[effect_shaders.size()-1]->speed = 0.5;
	effect_shaders.push_back(make_shared<Effect_shader>(this, "horizontalWave", ofPoint(120,8), ofPoint(20,20), colorVect, ledFrame.size));
	effect_shaders.push_back(make_shared<Effect_shader>(this, "SF2D", ofPoint(0,28), ofPoint(20,20), colorVect, ledFrame.size));
	effect_shaders.push_back(make_shared<Effect_shader>(this, "SF2D_2", ofPoint(20,28), ofPoint(20,20), colorVect, ledFrame.size));
	effect_shaders.push_back(make_shared<Effect_shader>(this, "SF2D_3", ofPoint(40,28), ofPoint(20,20), colorVect, ledFrame.size));
	effect_shaders.push_back(make_shared<Effect_shader>(this, "bluefire", ofPoint(60,28), ofPoint(20,20), colorVect, ledFrame.size));
	effect_shaders.push_back(make_shared<Effect_shader>(this, "purplefire", ofPoint(80,28), ofPoint(20,20), colorVect, ledFrame.size));
	// effect_shaders.push_back(make_shared<Effect_shader>(this, "lightning", ofPoint(48,8), ofPoint(20,20), colorVect, ledFrame.size));
	// effect_shaders.push_back(make_shared<Effect_shader>(this, "strobe", ofPoint(68,8), ofPoint(20,20), colorVect, ledFrame.size));
	// ofLog(OF_LOG_NOTICE,"Created: "+ effect_shaders[0]->name);
	for (uint i=0; i<effect_shaders.size(); i++)
	{
		effect_shaders[i]->enableMouseEvents();
		// offset position used for interactivity
		const ofPoint tmpPos = effect_shaders[i]->getPosition();
		effect_shaders[i]->setPosition(tmpPos + shaderFrame.pos);
	}

	/////////////////////////////////////////////////////////////////////////////
	// Audio Control
	ac.setup(soundBox);

	/////////////////////////////////////////////////////////////////////////////
	// Parameters
	timeMult = 0.2;
	masterBrightness = 1.0;
	effectLevels.push_back(1.0);
	effectLevels.push_back(1.0);
	effectLevels.push_back(1.0);
}

//--------------------------------------------------------------
void ofApp::update(){
	midiDeviceMenu->update();
	
	winTitle = "fps=" + ofToString((int)ofGetFrameRate());
	ofSetWindowTitle(winTitle);

	ac.update();

	// update shaders
	shaderFrame.fbo.begin();
	for (uint i=0; i< effect_shaders.size(); i++)
	{
		effect_shaders[i]->updatePreview( ofGetElapsedTimef() * pow(10,timeMult/2), effect_shaders[i]->previewSize);
		effect_shaders[i]->drawPreview();
	}
	shaderFrame.fbo.end();

	if (midiIn.isOpen())
	{
		updateMidiControls();
	}
	
	for (int i=0; i<active_effects.size(); i++)
	{
		active_effects[i]->brightness *= effectLevels[i];
	}

	if (simpleSound && ac.avgHigh < ac.highTrigger.threshold)
	{
		if(active_effects.size()>0)
		{
			active_effects[0]->brightness = 0;
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	

	// draw LED window
	ofNoFill();
	ofSetColor(200,200,200);
	ofSetLineWidth(0.5);
	ofDrawRectangle(ledBox);
	ofDrawRectangle(ledPreviewBox);
	ofDrawRectangle(paletteBox);
	ofDrawRectangle(midiControlBox);
	ofDrawRectangle(effectInfoBox);
	ofDrawRectangle(soundBox);

	
	/// Audio controller
	ac.draw();

	// shader palette frame
	shaderFrame.draw();

	
	// Render effects into separate frame buffers
	for (uint i=0; i< active_effects.size(); i++)
	{
		effectRenderFrames[i].fbo.begin();
		ofBackground(0);
		
		// dim the effect
		active_effects[i]->brightness = 1.0;
		active_effects[i]->updateEffect(ofGetElapsedTimef() * pow(10,timeMult), ledFrame.size);
		ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);		
		
		int tmpAlpha;
		if (i==0 && simpleSound)
		{
			tmpAlpha = 255;
			if (ac.avgHigh > ac.highTrigger.threshold )
			{
				tmpAlpha = (int)255 * (1.0 - ac.avgHigh*effectLevels[i]);
			}

		}
		else
		{
			tmpAlpha = (int)255 * (1.0 - effectLevels[i]);
		}

		ofSetColor(255,255,255,tmpAlpha);
		ofFill();
		ofDrawRectangle(0, 0, ledFrame.size.x, ledFrame.size.y);
		ofDisableBlendMode();		
		effectRenderFrames[i].fbo.end();

	}

	// LED total frame buffer
	ledFrame.fbo.begin();
	ofBackground(0);

	// add different effects together
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	for (uint i=0; i<active_effects.size(); i++)
	{
		effectRenderFrames[i].draw();
	}

	if (debugCircle && mouseY < ledBox.height)
	{		



		ofSetColor(dColors[dcolID]);
		ofFill;

		if (shapeCase ==1)
		{
			ofDrawCircle(mouseX-ledBox.x,mouseY,42);
		}
		if (shapeCase ==2)
		{
			ofDrawCircle(mouseX-ledBox.x,mouseY,84);
		}
		if (shapeCase ==3)
		{
			ofDrawRectangle(mouseX-ledBox.x,ledBox.y,40,ledBox.height);
		}
		if (shapeCase ==4)
		{
			ofDrawRectangle(mouseX-ledBox.x,ledBox.y,60,ledBox.height);
		}
		if (shapeCase ==5)
		{
			ofDrawRectangle(mouseX-ledBox.x,ledBox.y,80,ledBox.height);
		}
		if (shapeCase ==6)
		{
			ofDrawRectangle(mouseX-ledBox.x,ledBox.y,40,ledBox.height);
			ofDrawRectangle(mouseX-ledBox.x + 120,ledBox.y,40,ledBox.height);
			ofDrawRectangle(mouseX-ledBox.x + 240,ledBox.y,40,ledBox.height);
		}

	}

	// dim the total result
	ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
	masterBrightness = max(0.0,min(1.0,masterBrightness));
	int tmpAlpha = (int)255 * (1.0 - masterBrightness);
	ofSetColor(255,255,255,tmpAlpha);
	ofFill();
	ofDrawRectangle(0, 0, ledFrame.size.x, ledFrame.size.y);
	ofDisableBlendMode();



	ledFrame.fbo.end();
	
	ledFrame.draw();
	ofSetColor(dColors[dcolID]);
	ofFill();
	ofDrawRectangle(0,0,100,100);

	// LED PREVIEW ////////////////////////////////////////
	ledPreviewFrame.fbo.begin();
	ofBackground(0);
	ofFill();
	for (uint i=0; i<fc.allLEDcoords.size(); i++)
	{
		ofSetColor(fc.allLEDcolors[i]);

		ofDrawCircle(fc.allLEDcoords[i].x - ledBox.x, fc.allLEDcoords[i].y, 1);
	}
	ledPreviewFrame.fbo.end();
	ledPreviewFrame.draw();

	// MIDI CONTROL ////////////////////////////////////////
	midiControlFrame.fbo.begin();
	ofBackground(0);

	if (midiConnected){
		ofSetColor(0,255,0);
	}
	else{
		ofSetColor(255,0,0);
	}
	ofFill();
	ofDrawCircle(midiControlBox.width - 20,10,6);

	ofSetColor(255);
	//mast brightness

	ofDrawBitmapString("Master Brightness:" + ofToString(round(masterBrightness*1000)/1000), 4, 65);
	ofDrawRectangle(0,75,ofMap(masterBrightness,0,1,0,midiControlBox.width),15);

	int y0 = 130;
	ofSetColor(255,20,20);
	for (uint i=0; i<effectLevels.size(); i++)
	{
		ofDrawBitmapString("Effect "+ofToString(i+1)+" Brightness:" + ofToString(round(effectLevels[i]*1000)/1000), 4, y0);
		ofDrawRectangle(0,y0+10,ofMap(effectLevels[i],0,1,0,midiControlBox.width),15);
		y0 += 65;
	}

	midiControlFrame.fbo.end();
	midiControlFrame.draw();

	ofSetLogLevel(OF_LOG_ERROR);
	midiDeviceMenu->draw();
	ofSetLogLevel(OF_LOG_NOTICE);

	// EFFECT INFO ////////////////////////////////////
	effectInfoFrame.fbo.begin();
	ofBackground(0);

	y0 = 40;
	for (uint i= 0; i< active_effects.size(); i++)
	{
		ofDrawBitmapString("Effect1: " + active_effects[i]->name, 4, y0);
		y0+=50;
	}

	effectInfoFrame.fbo.end();
	effectInfoFrame.draw();

	if (ac.avgHigh > ac.highTrigger.threshold )
	{
		ofFill();
		ofSetColor(0,255,0);
		ofDrawRectangle(100,400,100,100);
	}

	ofDrawBitmapString("SimpleSound="+ofToString(simpleSound),10,10);

	fc.write(fc.allLEDcolors);
}// END DRAW

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

bool ofApp::activateEffect(shared_ptr<Effect_base> effect)
{
	if (active_effects.size() < maxEffects)
	{		
		active_effects.push_back( effect);
		effect -> activeID = active_effects.size() - 1;
		ofLog(OF_LOG_NOTICE,"effect name:"+effect->name);
		ofLog(OF_LOG_NOTICE,"effect ID:"+ofToString(effect->activeID));
		return true;
	}
	return false;
}

void ofApp::deactivateEffect(shared_ptr<Effect_base> effect)
{
	uint eraseID = effect->activeID;
	// ofLog(OF_LOG_NOTICE,"eraseID=" + ofToString(eraseID));

	// if this is not the last effect, decrease following effect IDs by 1
	if (eraseID<active_effects.size()-1)
	{
		for (uint i = eraseID+1; i< active_effects.size(); i++)
		{
			active_effects[i]-> activeID --;
		}
	}

	// now erase it
	// ofLog(OF_LOG_NOTICE,"erasing...");
	active_effects.erase(active_effects.begin() + effect->activeID);
	// ofLog(OF_LOG_NOTICE,"erased!");
	
}

void ofApp::updateMidiControls()
{
	// ofLog(OF_LOG_NOTICE,ofToString(mastBrightKnob->getValue()));

	masterBrightness = ofMap(mastBrightKnob->getValue(), 0, 16383, 0, 1.0);




	for (uint i=0; i<effectKnobs.size(); i++)
	{
		if (effectKnobs[i] == nullptr)
		{
			effectKnobs[i] = components->getKnob(knobNames[i]);
		}
		if (effectKnobs[i] != nullptr)
		{
			effectLevels[i] = ofMap(effectKnobs[i]->getValue(), 0, 16383, 0, 1.0);
		}
		else
		{
			effectLevels[i] = 1.0;
		}
		
	}


}

//--------------------------------------------------------------
//--------------------------------------------------------------
// 		EVENTS

//--------------------------------------------------------------
void ofApp::onMidiMenuDropdownEvent(ofxDatGuiDropdownEvent e)
{

	try
	{
		midiIn.openPort(e.child);
		midiConnected = true;
	}	
	catch (const std::exception& ex)
	{
		string errMsg = "Error initializing midi device.\n" + ofToString(ex.what());
		ofLog(OF_LOG_ERROR) << errMsg;
		midiConnected = false;
		//ofSystemAlertDialog(errMsg);
	}
}

//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage& msg) {

	// make a copy of the latest message
	midiMessage = msg;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == OF_KEY_F11)
	{
		// ofToggleFullscreen();
	}
	if (key == 'd')
	{
		debugCircle = !debugCircle;
	}
	if (key == OF_KEY_DOWN)
	{
		ac.currentTrigger->threshold -= 0.1;
	}
	if (key == OF_KEY_UP)
	{
		ac.currentTrigger->threshold += 0.1;
	}
	if (key == OF_KEY_LEFT)
	{
		*(ac.currentGain) -= 0.1;
	}
	if (key == OF_KEY_RIGHT)
	{
		*(ac.currentGain) += 0.1;
	}

	if (key == 'm')
	{
		simpleSound = !simpleSound;
	}

	if (key == '1')
	{
		shapeCase =1;
	}
	if (key == '2')
	{
		shapeCase =2;
	}
	if (key == '3')
	{
		shapeCase =3;
	}
	if (key == '4')
	{
		shapeCase =4;
	}
	if (key == '5')
	{
		shapeCase =5;
	}
	if (key == '6')
	{
		shapeCase =6;
	}
	if (key == 'c')
	{
		dcolID++;
		dcolID = dcolID%dColors.size();
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

	
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::exit(){
	delete components;
	midiIn.closePort();
	midiIn.removeListener(this);
}
