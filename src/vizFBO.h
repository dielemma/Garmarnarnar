#ifndef vizFBO_hpp
#define vizFBO_hpp

#include <stdio.h>
#include "ofMain.h"

class vizFBO
{

public:
	
	vizFBO()
	{

	};

	vizFBO(string name, ofPoint pos, ofPoint size)
	{
		this->setup(name, pos, size);
	};

	vizFBO(string name, ofRectangle rect)
	{
		ofPoint tmpPos = ofPoint(rect.x, rect.y);
		ofPoint tmpSize = ofPoint(rect.width, rect.height);
		this->setup(name, tmpPos, tmpSize);
	};

	void setup(string name, ofPoint pos, ofPoint size)
	{
		this->name = name;
		this->pos = pos;
		this->size = size;

		fbo.allocate(size.x, size.y, GL_RGBA);
		fbo.begin();
		ofBackground(ofColor::black);
		fbo.end();
	};

	void draw();


	string name;
	ofPoint pos;
	ofPoint size;

	ofFbo fbo;



};


#endif
