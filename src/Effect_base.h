#ifndef Effect_base_hpp
#define Effect_base_hpp


#include <stdio.h>
#include <memory>
#include <iostream>
#include <string>
#include "ofMain.h"

using namespace std;

class ofApp;

class Effect_base: public std::enable_shared_from_this<Effect_base>
{
public:
	Effect_base(ofApp* parent, string name)
	{
		ofLog(OF_LOG_NOTICE,"EBname="+name);
		this->parent = parent;
		this-> name = name;
		isActive = false;
		isSelected = false;
	};

	~Effect_base()
	{

	};

	void exit()
	{

	};

	bool activate();
	void deactivate();

	void draw();

	virtual void updateEffect(float time, ofPoint drawSize){};
	// virtual void updatePreview(float time, ofPoint drawSize);

	ofApp* parent;
	uint activeID;

	string name;
	double speed;
	double brightness;
	bool isActive;
	bool isSelected;

private:
	
};

#endif