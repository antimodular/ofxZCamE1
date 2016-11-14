#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    zcam.setup();
}
//--------------------------------------------------------------
void ofApp::update()
{

}
//--------------------------------------------------------------
void ofApp::draw()
{
    stringstream info;
    info << "Press 'g' to Get all settings from ZCam." << endl;
    info << "Press 'k' to Save settings." << endl;
    info << "Press 'l' to Load saved settings." << endl;
    info << "Press 's' to Send all settings to ZCam." << endl;
    ofDrawBitmapStringHighlight(info.str(), 5, ofGetHeight()-50);
	
	ofxJSONElement setting;
	ofxJSONElement settings_api = zcam.api["settings"]; 
	ofxJSONElement keys = zcam.api["settings"]["keys"];
	string key;

    ofSetBackgroundColor(ofColor(127, 127, 127));  
    
}

void ofApp::exit()
{
    zcam.session(0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	ofxJSONElement json;
    switch (key) {
        case 'k': // save all settings
			zcam.saveSettings();
            break;
        case 'l': // load all saved settings
			zcam.loadSettings();
            cout << zcam.settings << endl;
            break;
        case 'g': // get all settings from ZCam
			zcam.getSettings();
            cout << zcam.settings << endl;
            break;
        case 's': // send all settings to ZCam
			zcam.sendSettings();
            break;
        default:
            break;
    }
}

