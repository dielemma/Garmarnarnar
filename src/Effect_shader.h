#ifndef Effect_shader_hpp
#define Effect_shader_hpp

#include <stdio.h>
// #include "ofMain.h"
#include "Effect_base.h"
#include "ofxMSAInteractiveObject.h"

class Effect_shader:  public ofxMSAInteractiveObject, public Effect_base  
{

public:

	Effect_shader( ofApp* parent, string name, ofPoint position, ofPoint size, vector<ofColor> colors, ofPoint scale ) : Effect_base(parent, name)
	{
		ofLog(OF_LOG_NOTICE,"ESname="+name);

		shader.load(name);

		this-> colors = colors; // copy by value

		this-> previewPosition = position * scale / 100.0;
		this-> previewSize = size * scale / 100.0;

		// set ofxMSAInteractiveObject parameters
		set(previewPosition.x, previewPosition.y, previewSize.x, previewSize.y);		
		pressedIn = false;
	
		// default brightness
		brightness = 1.0;

		// allocate fbo
		fbo.allocate(previewSize.x, previewSize.y, GL_RGBA);
		fbo.begin();
		
		this->updateEffect(1.0, previewSize);

		fbo.end();

		isSelected = false;
		isActive = false;
		speed = 1;

	};

	~Effect_shader(){};

	
	void setup()
	{
		enableMouseEvents();
	};
	void draw()
	{

	};
	// needed to avoid vtable horseshit
	void exit()
	{

	};

	void drawPreview();
	void updateEffect(float time, ofPoint drawSize) override;
	

	void updatePreview(float time, ofPoint drawSize);
	bool mouseClick(int x, int y);


	// interactivity methods
	bool pressedIn;
	void onRollOver(int x, int y) override;
	void onRollOut() override;
	void onPress(int x, int y, int button) override;
	void onRelease(int x, int y, int button) override;
	void onReleaseOutside(int x, int y, int button) override;



	ofPoint previewPosition;
	ofPoint previewSize;
	bool showPreview;

	vector<ofColor> colors;

	// fbo for previewing shader
	ofFbo fbo;
	ofShader shader;

	// bool isActive;
	// bool isSelected;
	// float speed;
	// float brightness;
	

private:
};

#endif