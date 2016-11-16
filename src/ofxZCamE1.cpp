#include "ofxZCamE1.h"
#include <time.h>

void ofxZCamE1::setup()
{
	this->loadAPI("settings");
		
    if ( ! this->getInfo() ) {
		ofLogError("Can't get info from the ZCam E1.");
		return;
	}
	
	if ( ! this->session()) {
		ofLogError("The ZCam E1 is controlled by another session.");
		return;
   	}
		
};


void ofxZCamE1::loadAPI(string section)
{
	ofLogNotice("Loading ZCam API section : \"") << section << "\"..." << endl;

	ofxJSONElement result;
	string path = "zcam/api/"+section;
	ofDirectory dir(path);
	dir.allowExt("json");
	dir.listDir();
	
	if (dir.size() == 0) {
		ofLogFatalError("Missing api files from the bin/data folder; check ofxZCamE1 doc.");
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
        ofLogError("The ZCam E1 is controlled by another session!");
    }
    else if (response.status != 200)
        ofLogError() << response.status << " " << response.error << endl;
    else
		json.parse(response.data);

    return json;
};


bool ofxZCamE1::getInfo() {
	ofLogNotice("Getting info from ZCam...");
	this->info = this->apiCall("/info");
	ofLogNotice("ZCam E1 info: ") << this->info << endl;
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
		ofLogNotice("ZCam E1 session closed.");
		
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

	ofLogNotice("Getting all settings from ZCam...");
	for (int i = 0; i < keys.size(); i++) {
		if (keys[i] == null) continue;
		key = keys[i].asString();
		setting = getSetting(key);
		if (setting == null)
			ofLogError("Error getting ") << key << " from ZCam." << endl;
	}
	ofLogNotice("Finished to get all settings from ZCam.");
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

	ofLogNotice("Starting to send all settings to ZCam.") << endl;
	for (int i = 0; i < keys.size(); i++) {
		if (keys[i] == null) continue;
		key = keys[i].asString();
		if (key == "focus_pos" || key == "zoom_in") continue;

		value = this->settings[key].asString();
		if (this->api["settings"][key]["ro"].asString() == "1") continue;
		value = this->settings[key].asString();
		if (!sendSetting(key, value)) {
			ofLogError("error setting ") << key << "=" << value << " to ZCam." << endl;
		}
	}

	// send zoom
	if (this->settings["zoom_in"] != null) {
		value = this->settings["zoom_in"].asString();
		zoom_in(stof(value));
	}

	// must send focus twice; the zoom sets in, then focus occurs...
	if (this->settings["focus_pos"] != null) {
		value = this->settings["focus_pos"].asString();
		send_focus_pos(value);
		send_focus_pos(value);
	}

	ofLogNotice("Finished sending all settings to ZCam.");
};


bool ofxZCamE1::saveSettings() // save all settings
{
	string path = "zcam/settings.json";
	bool success = this->settings.save(path, true);
	if (success)
		ofLogNotice("ofxZCamE1::saveSettings") << path << " successfully saved." << endl;
	else
		ofLogError("ofxZCamE1::saveSettings") << path << " saving failed." << endl;
	return success;
};


bool ofxZCamE1::loadSettings() // load saved settings
{
	string path = "zcam/settings.json";
	bool success = this->settings.open(path);
	if (success)
		ofLogNotice("ofxZCamE1::loadSettings") << path << " successfully loaded." << endl;
	else
		ofLogError("ofxZCamE1::loadSettings") << path << " loading failed." << endl;
	return success;
};


bool ofxZCamE1::send_focus_pos(string focus_pos) {
	ofLogNotice("Focus in rectangle id ") << focus_pos << endl;

	stringstream uri;
	uri << "/ctrl/af?pos=" << focus_pos;
	ofxJSONElement json = this->apiCall(uri.str());
	return (json !=null && json["code"] == 0);
};

bool ofxZCamE1::focus_at(float x, float y) {
	int ix, iy, pos;
	string focus_pos;

	// contrain x and y between 0 and 1
	if (x < 0.0) x = 0.0;
	if (x > 1.0) x = 1.0;
	if (y < 0.0) y = 0.0;
	if (y > 1.0) y = 1.0;

	ix = int(floor(x * 15.0));
	if (ix == 15) ix = 14;
	iy = int(floor(y * 7.0));
	if (iy == 7) iy = 6;
	ofLogNotice("Focus at position ") << x << "," << y << endl;

	pos = int((ix + (iy * 15)));
	focus_pos = this->focus_pos[pos];
	this->settings["focus_pos"] = focus_pos;
	send_focus_pos(focus_pos);
};

void ofxZCamE1::sleep_for(uint64_t delay) {
	std::this_thread::sleep_for(
		std::chrono::milliseconds(delay)
	);
};

bool ofxZCamE1::zoom_in(float zoom) { // 0 = full out, 1 = full in
	uint64_t start, elapsed;

	// contrain x and y between 0 and 1
	if (zoom < 0.0) zoom = 0.0;
	if (zoom > 1.0) zoom = 1.0;

	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << zoom;
	std::string zoom_in = ss.str();
	this->settings["zoom_in"] = zoom_in;

	ofLogNotice("zooming out...");
	if (! sendSetting("lens_zoom", "out"))
		return 0;

	this->sleep_for(this->full_zoom_time + 500);

	ofLogNotice("zooming in to ") << zoom << "..." << endl;
	if (! sendSetting("lens_zoom", "in"))
		return 0;

	this->sleep_for(uint64_t(round(this->full_zoom_time * zoom)));

	ofLogNotice("zooming stopped.");
	return sendSetting("lens_zoom", "stop");

};
