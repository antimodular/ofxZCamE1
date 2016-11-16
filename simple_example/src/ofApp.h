#pragma once

#include "ofMain.h"
#include "ofxZCamE1.h"
	
class ofApp : public ofBaseApp{

	public:
        ofxZCamE1 zcam;
    
        void setup();
        void update();
        void draw();
        void exit();
        void keyPressed(int key);

};
