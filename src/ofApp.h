#pragma once

#include "ofMain.h"
#include "ofxFadecandy.h"
#include "ofxLEDstrip.h"
#include "AudioControl.h"
#include "Effect_shader.h"
#include "Effect_base.h"
#include "vizFBO.h"
#include "ofxDatGui.h"
#include "ofxMidi.h"
#include "DDJSB2Components.h"

// #include "MyTestObject.h"

class ofApp : public ofBaseApp, public ofxMidiListener{

	public:

		// MyTestObject obj;

		void setup();
		void update();
		void draw();
		void exit();

		ofxFadecandy fc;
		vector <shared_ptr<ofxLEDstrip>> strips;
		int Nstrips, NperStrip;
		
		ofRectangle ledBox;
		ofRectangle ledPreviewBox;
		ofRectangle soundBox;
		ofRectangle paletteBox;
		ofRectangle midiControlBox;
		ofRectangle effectInfoBox;

		double masterBrightness;
		vector<double> effectLevels;

		AudioControl ac;

		vector<shared_ptr<Effect_shader>> effect_shaders;
		vector<shared_ptr<Effect_base>> active_effects;
		bool activateEffect(shared_ptr <Effect_base> effect);
		void deactivateEffect(shared_ptr <Effect_base> effect);
		const uint maxEffects = 3;

		vizFBO shaderFrame;
		vizFBO ledFrame;
		vizFBO ledPreviewFrame;
		vector<vizFBO> effectRenderFrames;
		vizFBO midiControlFrame;
		vizFBO effectInfoFrame;

		double timeMult;

		vector<string> midiDeviceNameList;
		ofxMidiIn midiIn;
		ofxMidiMessage midiMessage;
		ofxDatGuiDropdown* midiDeviceMenu;		
		bool midiConnected;
		void onMidiMenuDropdownEvent(ofxDatGuiDropdownEvent e);
		void newMidiMessage(ofxMidiMessage& eventArgs);
		void updateMidiControls();
		DDJSB2Components* components;
		DDJSB2SliderKnob* mastBrightKnob;
		vector<DDJSB2SliderKnob*> effectKnobs;
		


		

		// EVENTS //////////////////////////////////////////////
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
	private:
		int screenW, screenH;
		string winTitle;

};
