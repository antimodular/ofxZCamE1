/* OF addon to control the Z Camera E1 through Wifi; simple example  */
/* © Antimodular Reasearch  */
/* Marc Lavallée, 2016/11/18  */

#pragma once

#include "ofMain.h"

#include "ofxZCamE1.h"

#include "ofxGui.h"
#include "ofParameterGroup.h"
#include "ofParameter.h"

class ofApp : public ofBaseApp{
    
public:
    ofxZCamE1 zcam;
    
    void setup();
    void draw();
    void update();
    void exit();
    void keyPressed(int key);
    
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    
    ofxPanel gui_main;
    ofParameter<bool> bShowGUI;
    ofParameter<float> zoom;
    float old_zoom;
    ofParameter<float> ev;
    float old_ev;
    ofParameter<bool> applyCamSliders;
    
    ofParameter<bool> zcamGet;
     ofParameter<bool> zcamConnect;
    ofParameter<bool>  zcamDisConnect;
    ofParameter<bool> zcamIsConnected;
    
};
