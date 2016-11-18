/* OF addon to control the Z Camera E1 through Wifi; simple example  */
/* © Antimodular Reasearch  */
/* Marc Lavallée, 2016/11/18  */

#pragma once

#include "ofxZCamE1.h"
	
class ofApp : public ofBaseApp{

	public:
        ofxZCamE1 zcam;
    
        void setup();
        void draw();
        void exit();
        void keyPressed(int key);
};
