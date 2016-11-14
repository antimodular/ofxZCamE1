#include "ofMain.h"
#include "ofxJSON.h"

class ofxZCamE1 : public Json::Value {
    
    public:
        void setup();
        
		void loadAPI(string section); // get api structure
		
        ofxJSONElement apiCall(string call); // http request to the ZCam
        bool session(bool activate=1); // session control
        bool getInfo(); // get info about ZCam
        
        ofxJSONElement api; // api structure
        ofxJSONElement settings; // settings

		ofxJSONElement getSetting(string setting); // get one setting from ZCam
		bool sendSetting(string setting, string value); // sed one setting to ZCam
		
		void getSettings();  // get all settings from ZCam
		void sendSettings(); // send all settings to ZCam		
		bool saveSettings(); // save all settings from ZCam		
		bool loadSettings(); // load saved settings to ZCam
    
    private:
        string base_url  = "http://10.98.32.1:80"; // (should be stable)
        bool session_is_active = 0;
        ofxJSONElement info; // info about ZCam
        
};


    
