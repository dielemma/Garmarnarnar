#include "vizFBO.h" 

void vizFBO::draw()
{
	// draw name
	ofSetColor(ofColor::white);
	fbo.begin();
	ofDrawBitmapString(name, 5, 12);
	fbo.end();

	fbo.draw(pos.x, pos.y, size.x, size.y);	
}

