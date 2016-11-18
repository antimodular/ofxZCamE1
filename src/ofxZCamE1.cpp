/* OF addon to control the Z Camera E1 through Wifi  */
/* © Antimodular Reasearch  */
/* Marc Lavallée, 2016/11/18  */

#include "ofxZCamE1.h"


bool ofxZCamE1::init()
{
	if (this->loadAPI("settings"))
		if (this->getInfo())
			if (this->session())
				this->ready = true;
   	return this->ready;
};


void ofxZCamE1::setup()
{
	if (! this->ready) {
		ofLogFatalError("Instance of ofxZCamE1 is not ready.");
		return;
	}
   	this->startThread();
};


void ofxZCamE1::threadedFunction() {
    while(isThreadRunning())
    {
		sleep(25);
		if (this->fl.empty()) continue;
		auto& f = this->fl.back();
		f();
		this->fl.pop_back();
    }
};


bool ofxZCamE1::loadAPI(string section)
{
	ofLogNotice("Loading ZCam API section : \"") << section << "\"...";

	ofxJSONElement result;
	string path = "zcam/api/"+section;
	ofDirectory dir(path);
	dir.allowExt("json");
	dir.listDir();
	
	if (dir.size() == 0) {
		ofLogFatalError("Missing api files from the bin/data folder; check ofxZCamE1 doc.");
		return false;
	}

	for (int i = 0; i < dir.size(); i++) {
		if (! result.open( string(dir.getPath(i)) ) ) return false;
		if (result["key"] == null) continue;
		this->api[section]["keys"][i] = result["key"].asString();
		this->api[section][result["key"].asString()] = result;
	}
	return true;
};


ofxJSONElement ofxZCamE1::apiCall(string uri)
{
	ofxJSONElement json;
	
	stringstream url;
	url << base_url << uri;
	ofHttpResponse response = ofLoadURL(url.str());

    if ( response.status == 409 ) {
        session_is_active = false;
        ofLogError("The ZCam E1 is controlled by another session!");
    }
    else if (response.status != 200)
        ofLogError() << response.status << " " << response.error;
    else
		json.parse(response.data);

    return json;
};


bool ofxZCamE1::getInfo() {
	ofLogNotice("Getting info from ZCam...");
	this->info = this->apiCall("/info");
	ofLogNotice("ZCam E1 info: ") << this->info;
	return (this->info["model"] == "E1"); // crude validation
}


bool ofxZCamE1::session(bool activate, bool thread) {

	if (thread) {
		this->fl.insert(this->fl.begin(), 
			bind(&ofxZCamE1::session, this, activate, false)
		);
		return true;
	}

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


ofxJSONElement ofxZCamE1::getSetting(string key, bool thread)
{
	
	ofxJSONElement json;
	
	if (settings_skip_list.find(key) != settings_skip_list.end()) {
		ofLogNotice("skipping ") << key;
		return json;
	}
	
	if (thread) {
		this->fl.insert(this->fl.begin(), 
			bind(&ofxZCamE1::getSetting, this, key, false)
		);
		return json;
	}
	stringstream uri;
	uri << "/ctrl/get?k=" << key;
	json = this->apiCall(uri.str());
	if (json !=null)
		this->settings[key] = json["value"].asString();
	return json;
};


void ofxZCamE1::getSettings(bool thread)
{
	if (thread) {
		this->fl.insert(this->fl.begin(), 
			bind(&ofxZCamE1::getSettings, this, false)
		);
		return;
	}
	
	ofxJSONElement setting;
	ofxJSONElement keys = this->api["settings"]["keys"];

	ofLogNotice("Getting all settings from ZCam...");
	for (int i = 0; i < keys.size(); i++) {
		if (keys[i] == null) continue;
		string key = keys[i].asString();
		if (settings_skip_list.find(key) != settings_skip_list.end()) {
			ofLogNotice("skipping ") << key;
			continue;
		}
		setting = getSetting(key, false);
		if (setting == null)
			ofLogError("Error getting ") << key << " from ZCam.";
	}
	ofLogNotice("Finished to get all settings from ZCam : ") << this->settings;
};


bool ofxZCamE1::sendSetting(string key, string value, bool thread)
{
	if (settings_skip_list.find(key) != settings_skip_list.end()) {
		ofLogNotice("skipping ") << key;
		return false;
	}
	
	if (thread) {
		this->fl.insert(this->fl.begin(), 
			bind(&ofxZCamE1::sendSetting, this, key, value, false)
		);
		return true;
	}
	stringstream uri;
	uri << "/ctrl/set?" << key << "=" << value;
	ofxJSONElement json = this->apiCall(uri.str());
	bool success = (json !=null && json["code"] == 0);
	if (success)
		this->settings[key] = value;
	return success;
};


void ofxZCamE1::sendAllSettings(bool thread)
{
	if (thread) {
		this->fl.insert(this->fl.begin(), 
			bind(&ofxZCamE1::sendAllSettings, this, false)
		);
		return;
	}

	ofLogNotice("Starting to send all settings to ZCam.");

	ofxJSONElement keys = this->api["settings"]["keys"];
	string value;
	
	for (int i = 0; i < keys.size(); i++) {
		if (keys[i] == null) continue;
		string key = keys[i].asString();
		if (key == "focus_pos" || key == "zoom_in") continue;

		if (this->api["settings"][key]["ro"].asString() == "1") 
			continue;
			
		value = this->settings[key].asString();
		if (value == "") {
			ofLogError("setting has no value:") << key;
			continue;
		}
		
		if (!sendSetting(key, value, thread)) {
			ofLogError("error setting ") << key << "=" << value << " to ZCam.";
		}
	}

	// send zoom
	if (this->settings["zoom_in"] != null) {
		float zoom_in_val = stof(this->settings["zoom_in"].asString());
		zoom_in(zoom_in_val, thread);
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
		ofLogNotice("ofxZCamE1::loadSettings") << path << " successfully loaded : " << this->settings;
	else
		ofLogError("ofxZCamE1::loadSettings") << path << " loading failed.";
	return success;
};


bool ofxZCamE1::send_focus_pos(string focus_pos) {
	ofLogNotice("Focus in rectangle id ") << focus_pos;

	stringstream uri;
	uri << "/ctrl/af?pos=" << focus_pos;
	ofxJSONElement json = this->apiCall(uri.str());
	return (json !=null && json["code"] == 0);
};


bool ofxZCamE1::focus_at(float x, float y, bool thread) {
		
	if (thread) {
		this->fl.insert(this->fl.begin(), 
			bind(&ofxZCamE1::focus_at, this, x, y, false)
		);
		return true;
	}
	
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
	ofLogNotice("Focus at position ") << x << "," << y;

	pos = int((ix + (iy * 15)));
	focus_pos = this->focus_pos[pos];
	this->settings["focus_pos"] = focus_pos;
	
	return send_focus_pos(focus_pos);
};


void ofxZCamE1::sleep_for(uint64_t delay) {
	std::this_thread::sleep_for(
		std::chrono::milliseconds(delay)
	);
};


bool ofxZCamE1::zoom_in(float zoom, bool thread) { // 0 = full out, 1 = full in
	
	if (thread) {
		this->fl.insert(this->fl.begin(), 
			bind(&ofxZCamE1::zoom_in, this, zoom, false)
		);
		return true;
	}

	// contrain x and y between 0 and 1
	if (zoom < 0.0) zoom = 0.0;
	if (zoom > 1.0) zoom = 1.0;

	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << zoom;
	std::string zoom_in = ss.str();
	this->settings["zoom_in"] = zoom_in;

	ofLogNotice("zooming out...");
	if (! sendSetting("lens_zoom", "out", thread))
		return false;

	if (thread)
		this->sleep(this->full_zoom_time + 500);
	else
		this->sleep_for(this->full_zoom_time + 500);

	ofLogNotice("zooming in to ") << zoom << "...";
	if (! sendSetting("lens_zoom", "in", thread))
		return false;

	if (thread)
		this->sleep(int(round(this->full_zoom_time * zoom)));
	else
		this->sleep_for(uint64_t(round(this->full_zoom_time * zoom)));

	ofLogNotice("zooming stopped.");
	return sendSetting("lens_zoom", "stop", thread);

};


bool ofxZCamE1::set_ev(float ev, bool thread) {
	
	if (thread) {
		this->fl.insert(this->fl.begin(), 
			bind(&ofxZCamE1::set_ev, this, ev, false)
		);
		return true;
	}
	
	if (ev < -3.0) ev = -3.0;
	if (ev >  3.0) ev =  3.0;
	
	string evh = to_string(int(round(32*ev)));
	ofLogNotice("set ev to ") << ev << " (" << evh << ")";
	this->settings["ev"] = evh;
	return sendSetting("ev", evh);
};


