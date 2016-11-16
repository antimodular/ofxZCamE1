#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetWindowShape(400, 300);
    zcam.setup();
    zcam.full_zoom_time = 2800; // may depend on the lens
}
//--------------------------------------------------------------
void ofApp::update()
{

}
//--------------------------------------------------------------
void ofApp::draw()
{
    ofSetBackgroundColor(ofColor(0,0,0));

    stringstream info;
    info << "Simple example for the ZCamE1 addon." << endl;
    info << "(run from a console to see messages).\n" << endl;
    info << "Press 'g' to Get all settings from ZCam." << endl;
    info << "Press 'k' to Save settings." << endl;
    info << "Press 'l' to Load saved settings." << endl;
    info << "Press 's' to Send all settings to ZCam.\n" << endl;

    info << "Press 'y' to zoom to max." << endl;
    info << "Press 'u' to zoom to middle." << endl;
    info << "Press 'i' to zoom to min." << endl;
    info << "Press 'h' to zoom to 1/3." << endl;
    info << "Press 'j' to zoom to 2/3.\n" << endl;

    info << "Press 'f' to focus to center." << endl;

    ofDrawBitmapStringHighlight(info.str(), 50, 50);
	
	ofxJSONElement setting;
	ofxJSONElement settings_api = zcam.api["settings"]; 
	ofxJSONElement keys = zcam.api["settings"]["keys"];
	string key;
    
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
            break;
        case 'f': // focus to center
			zcam.focus_at(0.5, 0.5);
            break;
        case 'y': // zoom to max
			zcam.zoom_in(0.0);
            break;
        case 'u': // zoom to middle
			zcam.zoom_in(0.5);
            break;
        case 'i': // zoom to min
			zcam.zoom_in(1.0);
            break;
        case 'h': // zoom to 1/3
			zcam.zoom_in(0.33);
			break;
        case 'j': // zoom to 2/3
			zcam.zoom_in(0.66);
            break;
        default:
            break;
    }
}

