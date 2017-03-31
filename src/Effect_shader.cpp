#include "Effect_shader.h"

void Effect_shader::drawPreview()
{
	fbo.draw(previewPosition.x, previewPosition.y, previewSize.x, previewSize.y);	
}

void Effect_shader::updateEffect( float time, ofPoint drawSize)
{
	ofSetColor(ofColor::white);
	ofFill();

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
	//shader.setUniform1f("mastBright", brightness);


	ofDrawRectangle(0, 0, drawSize.x, drawSize.y);

	shader.end();
	

}

void Effect_shader::updatePreview( float time, ofPoint drawSize)
{
	fbo.begin();

	ofBackground(0);

	ofFill();
	if (isSelected)
	{
		ofSetColor(255);
		ofDrawRectangle(0, 0, previewSize.x, previewSize.y);
	}

	// draw a green rectangle if active
	if( isActive)
	{
		ofSetColor(ofColor::green);
		ofDrawRectangle(0,0,previewSize.x, previewSize.y);
	}
	
	// draw a white rectangle on mouse over
	if( this->isMouseOver())
	{
		ofSetColor(255);
		ofDrawRectangle(0,0,previewSize.x, previewSize.y);
	}

	ofPushMatrix();
	ofTranslate(2,2); 
	updateEffect(time,drawSize - 4);
	ofPopMatrix();

	fbo.end();
}



bool Effect_shader::mouseClick(int x, int y)
{
	// // click LHS of preview to toggle active
	// if (x > previewPosition.x && x < previewPosition.x + previewSize.x/2 && y > previewPosition.y && y < previewPosition.y + previewSize.y)
 //    {
 //    	isActive = !isActive;
 //    	isSelected = true;
 //    	return true;
 //    }

 //    // click RHS of preview to toggle selected
 //    if (x > previewPosition.x+ previewSize.x/2 && x < previewPosition.x + previewSize.x && y > previewPosition.y && y < previewPosition.y + previewSize.y)
 //    {
 //        isSelected = !isSelected;
 //        return true;
 //    }
        
    return false;
}



void Effect_shader::onRollOver(int x, int y)
{

}

void Effect_shader::onRollOut()
{

}

void Effect_shader::onPress(int x, int y, int button)
{
	pressedIn = true;

}

void Effect_shader::onRelease(int x, int y, int button)
{
	if (pressedIn)
	{
		ofLog(OF_LOG_NOTICE,"active="+ofToString(isActive));
		// left clicked
		if (button == 0)
		{
			// ofLog(OF_LOG_NOTICE,"click0");
			if (isActive)
			{
				deactivate();
			}
			else if (!isActive)
			{
				activate();
			}

			isSelected = true;

		}

		// if (button ==1 )
		// {
		// 	ofLog(OF_LOG_NOTICE,"click1");
		// 	if(isActive)
		// 	{
		// 		deactivate();
		// 	}
		// 	isSelected = true;
		// }

		// right clicked
		if (button == 2)
		{
			ofLog(OF_LOG_NOTICE,"click2");
			isSelected = !isSelected;
		}
	}
	pressedIn = false;

	
}

void Effect_shader::onReleaseOutside(int x, int y, int button)
{
	pressedIn = false;
}

// void Effect_shader::onRollOut()
// {

// }

// void Effect_shader::onPress(int x, int y, int button)
// {

// }