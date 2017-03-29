#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);
	screenW = ofGetScreenWidth();
	screenH = ofGetScreenHeight()-40;	
	ofSetWindowPosition(0,0);
	ofSetWindowShape(screenW, screenH);
	
	ofSetBackgroundColor(0);
	
	ofSetLogLevel(OF_LOG_NOTICE);


	// SETUP BOXES ////////////////////////////////////////////
	const int bb = 8;
	const int ww = screenW;
	LEDbox = ofRectangle(ww/3.0 + bb, bb, ww/3.0 - 2*bb, (ww/3.0 - 2*bb)*0.5625);

	soundBox = ofRectangle(ww/3.0 + bb, 2*bb + LEDbox.height, ww/3.0 - 2*bb, 320);

	paletteBox = ofRectangle(bb, 3*bb + LEDbox.height + soundBox.height, screenW - 2 *bb, 320);


	// FADECANDY /////////////////////////////////////////////
	fc.setup("127.0.0.1", 7890, true, true);

	ofLog(OF_LOG_NOTICE, "ww="+ ofToString(ww));
	fc.setupStage(LEDbox);

	
	// MANUALLY CONFIGURE LED STRIPS
	Nstrips = 16;
	NperStrip = 30;	
	
	int spacing = 4;//round((LEDbox.width-4)/(Nstrips/2-1)/(NperStrip-1)/cos(angle));
	float angle = acos((LEDbox.width-4) / (Nstrips/2*(NperStrip)*spacing) );
	ofLog(OF_LOG_NOTICE,"angle = " + ofToString(angle));
	float L = (NperStrip-1)*spacing;
	float dx = L*cos(angle);
	
	float x0 = LEDbox.x+2;
	float y0 = LEDbox.y + LEDbox.height/4.0;

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

	y0 = LEDbox.y + 3.0*LEDbox.height/4.0;
	for (int i=0; i<8; i++)
	{
		float x = x0 + 0.5*dx + i*(dx + spacing * cos(angle));
		strips.push_back(make_shared<ofxLEDstrip>(id, NperStrip, x, y0, spacing, pow(-1,i)*angle, false));
		id += 30;
	}


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


	// Audio Control
	ac.setup(soundBox);

}

//--------------------------------------------------------------
void ofApp::update(){

	
	winTitle = "fps=" + ofToString((int)ofGetFrameRate());
	ofSetWindowTitle(winTitle);

	ac.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	// draw LED window
	ofNoFill();
	ofSetColor(200,200,200);
	ofSetLineWidth(0.5);
	ofDrawRectangle(LEDbox);
	ofDrawRectangle(paletteBox);

	ac.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == OF_KEY_F11)
	{
		ofToggleFullscreen();
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
