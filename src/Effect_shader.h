#ifndef Effect_shader_hpp
#define Effect_shader_hpp

#include <stdio.h>
#include "ofMain.h"
#include "Effect_base.h"

class Effect_shader: public Effect_base
{

public:
	Effect_shader( string name, ofPoint size, vector<ofColor> colors, ofPoint position, ofPoint scale )
	{
		this-> name = name;
		shader.load(name);

		this-> colors = colors; // copy by value

		this-> previewPosition = position * scale / 100.0;
		this-> previewSize = size * scale / 100.0;

		// default brightness
		brightness = 1.0;

		// allocate fbo
		fbo.allocate(previewSize.x, previewSize.y, GL_RGBA);
		fbo.begin();
		
		this->update(1.0, previewSize);

		fbo.end();

		isActive = false;
		isSelected = false;
		speed = 0;
		showPreview = false;

	};

	void draw();
	void update(float time, ofPoint drawSize);
	void updateSelf(float time, ofPoint drawSize);
	bool mouseClick(int x, int y);


	string name;
	ofPoint previewPosition;
	ofPoint previewSize;
	bool showPreview;

	vector<ofColor> colors;

	// fbo for previewing shader
	ofFbo fbo;
	ofShader shader;
	

private:
};

#endif