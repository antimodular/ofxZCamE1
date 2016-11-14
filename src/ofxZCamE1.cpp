#include "ofxZCamE1.h"

void ofxZCamE1::setup()
{
	this->loadAPI("settings");
		
    if ( ! this->getInfo() ) {
		ofLogError() << "Can't get info from the ZCam E1.";
		return;
	}
	
	if ( ! this->session()) {
		ofLogError() << "The ZCam E1 is controlled by another session.";
		return;
   	}
		
};


void ofxZCamE1::loadAPI(string section)
{
	ofLogNotice() << "Loading ZCam API section : \"" << section << "\"..." << endl;

	ofxJSONElement result;
	string path = "zcam/api/"+section;
	ofDirectory dir(path);
	dir.allowExt("json");
	dir.listDir();
	
	if (dir.size() == 0) {
		ofLogFatalError() << "Missing api files from the bin/data folder; check ofxZCamE1 doc." << endl;
		exit(1);
	}

	for (int i = 0; i < dir.size(); i++) {
		if (! result.open( string(dir.getPath(i)) ) ) continue;
		if (result["key"] == null) continue;
		this->api[section]["keys"][i] = result["key"].asString();
		this->api[section][result["key"].asString()] = result;
	}
};


ofxJSONElement ofxZCamE1::apiCall(string uri)
{
	ofHttpResponse response; 
	ofxJSONElement json;
	stringstream url;
	
	url << base_url << uri;
	response = ofLoadURL(url.str());

    if ( response.status == 409 ) {
        session_is_active = 0;
        ofLogError() << "The ZCam E1 is controlled by another session!" << endl;
    }
    else if (response.status != 200)
        ofLogError() << response.status << " " << response.error << endl;
    else
		json.parse(response.data);

    return json;
};


bool ofxZCamE1::getInfo() {
	ofLogNotice() << "Getting info from ZCam..." << endl;
	this->info = this->apiCall("/info");
	ofLogNotice() << "ZCam E1 info: " << this->info << endl;
	return (this->info["model"] == "E1"); // crude validation
}


bool ofxZCamE1::session(bool activate) {
	ofxJSONElement json;
	
	if (activate) {
        json = this->apiCall("/ctrl/session?action=heart_x_beat");
        this->session_is_active = (json !=null && json["code"] == 0);
    } else if (session_is_active) {
        json = this->apiCall("/ctrl/session?action=quit");
        this->session_is_active = ! (json !=null && json["code"] == 0);
    }
    
    if (!this->session_is_active)
		ofLogNotice() << "ZCam E1 session closed." << endl;
		
    return this->session_is_active;
}


ofxJSONElement ofxZCamE1::getSetting(string key)
{
	stringstream uri;
	uri << "/ctrl/get?k=" << key;
	ofxJSONElement json = this->apiCall(uri.str());
	if (json !=null)
		this->settings[key] = json["value"].asString();
	return json;
};


void ofxZCamE1::getSettings()
{
	ofxJSONElement setting;
	ofxJSONElement keys = this->api["settings"]["keys"];
	string key;

	ofLogNotice() << "Getting all settings from ZCam..." << endl;
	for (int i = 0; i < keys.size(); i++) {
		if (keys[i] == null) continue;
		key = keys[i].asString();
		setting = getSetting(key);
		if (setting == null)
			ofLogError() << "Error getting " << key << " from ZCam." << endl;
	}
	ofLogNotice() << "Finished to get all settings from ZCam." << endl;
};


bool ofxZCamE1::sendSetting(string key, string value)
{
	stringstream uri;
	uri << "/ctrl/set?" << key << "=" << value;
	ofxJSONElement json = this->apiCall(uri.str());
	bool success = (json !=null && json["code"] == 0);
	if (success)
		this->settings[key] = value;
	return success;
};


void ofxZCamE1::sendSettings()
{
	ofxJSONElement keys = this->api["settings"]["keys"];
	string key, value;
	
	ofLogNotice() << "Starting to send all settings to ZCam." << endl;
	for (int i = 0; i < keys.size(); i++) {
		if (keys[i] == null) continue;
		key = keys[i].asString();
		if (this->api["settings"][key]["ro"].asString() == "1") continue;
		value = this->settings[key].asString();
		if (!sendSetting(key, value)) {
			ofLogError() << "error setting " << key << "=" << value << " to ZCam." << endl;
		}
	}
	ofLogNotice() << "Finished sending all settings to ZCam." << endl;
};


bool ofxZCamE1::saveSettings() // save all settings
{
	string path = "zcam/settings.json";
	bool success = this->settings.save(path, true);
	if (success)
		ofLogNotice("ofxZCamE1::saveSettings") << path << " successfully saved.";
	else
		ofLogError("ofxZCamE1::saveSettings") << path << " saving failed.";
	return success;
};


bool ofxZCamE1::loadSettings() // load saved settings
{
	string path = "zcam/settings.json";
	bool success = this->settings.open(path);
	if (success)
		ofLogNotice("ofxZCamE1::loadSettings") << path << " successfully loaded.";
	else
		ofLogError("ofxZCamE1::loadSettings") << path << " loading failed.";
	return success;
};
