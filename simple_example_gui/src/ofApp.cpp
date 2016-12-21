/* OF addon to control the Z Camera E1 through Wifi; simple example  */
/* © Antimodular Reasearch  */
/* Marc Lavallée, 2016/11/18  */

#include "ofApp.h"

void ofApp::setup()
{
    
string _str = "!!! camera not detected !!!";
    ofLog()<<"_str "<<_str<<" length = "<<_str.length();
    
    gui_main.setup("cam");
    gui_main.setPosition(10,10);
    gui_main.setDefaultHeaderBackgroundColor(ofColor(255,0,0));
    gui_main.add(bShowGUI.set("showGui",false));
    gui_main.add(zcamIsConnected.set("isConnected",false));
    gui_main.add(zcamConnect.set("connect",false));
    gui_main.add(zcamDisConnect.set("disconnect",false));
    
    gui_main.add(zcamGet.set("get",false));
    gui_main.add(applyCamSliders.set("applySliders",false));
    gui_main.add(zoom.set("zoom",1,0,1));
    gui_main.add(ev.set("ev",0.7,-3,3));
    gui_main.loadFromFile("gui_main.xml");
    
    bShowGUI = true;
    
    zcam.full_zoom_time = 2800; // may depend on the lens
    zcam.settings_skip_list = {"video_system", "video_output"}; // settings to skip (set and send)

    zcam.setup();
}


void ofApp::update(){
    
    if(zcamGet){
        zcamGet = false;
        zcam.getSettings();

    }
    if(applyCamSliders){
        ofxJSONElement setting;
        ofxJSONElement settings_api = zcam.api["settings"];
        ofxJSONElement keys = zcam.api["settings"]["keys"];

        
        applyCamSliders = false;
        ofxJSONElement json;
        if(old_zoom != zoom){
            old_zoom = zoom;
            zcam.zoom_in(zoom);
            ofLog()<<"zoom "<<zoom;
        }
        
        if(old_ev != ev){
            old_ev = ev;
            zcam.set_ev(ev);
        }
    }
    
    if(zcamConnect){
        zcamConnect = false;
        zcam.connect(true);
    }
    if(zcamDisConnect){
        zcamDisConnect = false;
        zcam.connect(false);
    }
    
    
    zcamIsConnected = zcam.ready;
}
void ofApp::draw()
{
    if (! zcam.connection)
        ofSetBackgroundColor(ofColor(0,0,0));
    else if (zcam.ready) {
        if (zcam.fl.empty()) // no functions executing in the thread
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
    
    ofDrawBitmapStringHighlight(info.str(), 40, 250);
    
    
    if(bShowGUI){
        gui_main.draw();
    }
    
}

void ofApp::exit()
{
    zcam.session(false);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    if(key == ' '){
        bShowGUI = !bShowGUI;
        
        if(bShowGUI == false){
              gui_main.saveToFile("gui_main.xml");
        }
    }
    
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

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    

}

