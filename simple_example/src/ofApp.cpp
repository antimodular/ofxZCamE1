/* OF addon to control the Z Camera E1 through Wifi; simple example  */
/* © Antimodular Reasearch  */
/* Marc Lavallée, 2016/11/18  */

#include "ofApp.h"

void ofApp::setup()
{
    ofSetWindowShape(400, 350);

    zcam.full_zoom_time = 2800; // may depend on the lens
    zcam.settings_skip_list = {"video_system", "video_output"}; // settings to skip (set and send)
		
    zcam.setup();
}


void ofApp::draw()
{
	if (! zcam.connection)
		ofSetBackgroundColor(ofColor(0,0,0));
	else if (zcam.ready) {
		if (zcam.fl.empty()) // not functions executing in the thread
			ofSetBackgroundColor(ofColor(0,255,0));
		else
			ofSetBackgroundColor(ofColor(127,127,127));
	} else
		ofSetBackgroundColor(ofColor(255,0,0));

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

    info << "Press 'f' to focus on center.\n" << endl;
    
    info << "Press 'w' to set ev to -3.0." << endl;
    info << "Press 'e' to set ev to 0.0." << endl;
    info << "Press 'r' to set ev to 3.0.\n" << endl;

    info << "Press 'c' to connect." << endl;
    info << "Press 'd' to disconnect." << endl;

    ofDrawBitmapStringHighlight(info.str(), 40, 40);
	
	ofxJSONElement setting;
	ofxJSONElement settings_api = zcam.api["settings"]; 
	ofxJSONElement keys = zcam.api["settings"]["keys"];
    
}

void ofApp::exit()
{
    zcam.session(false);
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
            break;
            
        case 'g': // get all settings from ZCam
			zcam.getSettings();
            break;
        case 's': // send all settings to ZCam
			zcam.sendAllSettings();
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
            
        case 'w': // zoom to 2/3
			zcam.set_ev(-3.0);
            break;
        case 'e': // zoom to 2/3
			zcam.set_ev(0.0);
            break;
        case 'r': // zoom to 2/3
			zcam.set_ev(3.0);
            break;
            
        case 'c': // connect
			zcam.connect(true);
            break;
        case 'd': // disconnect
			zcam.connect(false);
            break;
            
        default:
            break;
    }
}

