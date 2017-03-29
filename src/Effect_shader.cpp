#include "Effect_shader.h"

void Effect_shader::draw()
{
	fbo.draw(previewPosition.x, previewPosition.y, previewSize.x, previewSize.y);	
}

void Effect_shader::update( float time, ofPoint drawSize)
{
	ofSetColor(ofColor::white);

	shader.begin();


	// TODO: list of params should be input string vector (parsed from XML)
	// param type
	// param name

	shader.setUniform1f("time",time);
	shader.setUniform2f("WindowSize", drawSize.x, drawSize.y);
	for (uint i=0; i<colors.size(); i++)
	{
		const string cname = "colour" + ofToString(i+1);
		shader.setUniform4f(cname, colors[i].r, colors[i].g, colors[i].b, colors[i].a);			
	}
	shader.setUniform1f("mastBright", brightness);
	ofDrawRectangle(0, 0, drawSize.x, drawSize.y);
	shader.end();

}

void Effect_shader::updateSelf( float time, ofPoint drawSize)
{
	fbo.begin();

	ofBackground(0);

	// draw a green rectangle around LHS of preview if active
	if( isActive)
	{
		ofSetColor(ofColor::green);
		ofDrawRectangle(0,0,previewSize.x/2, previewSize.y);
	}
	// draw red rectangle around RHS of preview if selected
	if (isSelected)
	{
		ofSetColor(ofColor::red);
		ofDrawRectangle(previewSize.x/2, 0, previewSize.x/2, previewSize.y);
	}

	ofPushMatrix();
	ofTranslate(2,2); 
	update(time,drawSize -4);
	ofPopMatrix();

	fbo.end();
}

bool Effect_shader::mouseClick(int x, int y)
{
	// click LHS of preview to toggle active
	if (x > previewPosition.x && x < previewPosition.x + previewSize.x/2 && y > previewPosition.y && y < previewPosition.y + previewSize.y)
    {
    	isActive = !isActive;
    	isSelected = true;
    	return true;
    }

    // click RHS of preview to toggle selected
    if (x > previewPosition.x+ previewSize.x/2 && x < previewPosition.x + previewSize.x && y > previewPosition.y && y < previewPosition.y + previewSize.y)
    {
        isSelected = !isSelected;
        return true;
    }
        
    return false;
}