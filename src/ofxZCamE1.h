/* OF addon to control the Z Camera E1 through Wifi  */
/* © Antimodular Reasearch  */
/* Marc Lavallée, 2016/11/18  */

#include "ofMain.h"
#include "ofxJSON.h"
#include <time.h>

// function queue for thread loop
// http://stackoverflow.com/questions/13108663/storing-functions-call-and-list-of-parameters-to-invoke-later
typedef vector<function<void(void)>> function_list;


class ofxZCamE1: public ofThread, Json::Value 
{
    public:
		bool ready = false;
		bool init();
        void setup();

		void threadedFunction();
		function_list fl;

		bool loadAPI(string section); // get api structure

        ofxJSONElement apiCall(string call); // http request to the ZCam
        bool session(bool activate=1, bool thread=false); // session control
        bool getInfo(); // get info about ZCam

        ofxJSONElement api; // api structure
        
        ofxJSONElement settings; // settings
        set<string> settings_skip_list; // settings to skip (set and send)

		ofxJSONElement getSetting(string setting, bool thread=false); // get one setting from ZCam
		void getSettings(bool thread=true);  // get all settings from ZCam

		bool sendSetting(string setting, string value, bool thread=true); // sed one setting to ZCam
		void sendAllSettings(bool thread=true); // send all settings to ZCam		

		bool saveSettings(); // save all settings from ZCam		
		bool loadSettings(); // load saved settings to ZCam

		/*  ev */
		bool set_ev(float ev, bool thread=true);
		
		/* Focus at position */
		bool send_focus_pos(string focus_pos); // rectangle id
		bool focus_at(float x, float y, bool thread=true); // values between 0 and 1

		/* Zoom; time based zoom in from full zoom out (0 to 1) */
		uint64_t full_zoom_time = 2800; // default value
		void sleep_for(uint64_t delay); // wrapper for milliseconds sleep
		bool zoom_in(float zoom, bool thread=true); // 0 to 1


    private:
        string base_url  = "http://10.98.32.1:80"; // (should be stable)
        bool session_is_active = false;
        ofxJSONElement info; // info about ZCam

        /*  Manual Focus */
		string focus_pos[105] = {
			"196623", "393231", "786447", "1572879", "3145743", "6291471", "12582927", "25165839", "50331663", "100663311", "201326607", "402653199", "805306383", "1610612751", "-1073741809", 
			"196668", "393276", "786492", "1572924", "3145788", "6291516", "12582972", "25165884", "50331708", "100663356", "201326652", "402653244", "805306428", "1610612796", "-1073741764", 
			"196848", "393456", "786672", "1573104", "3145968", "6291696", "12583152", "25166064", "50331888", "100663536", "201326832", "402653424", "805306608", "1610612976", "-1073741584", 
			"197568", "394176", "787392", "1573824", "3146688", "6292416", "12583872", "25166784", "50332608", "100664256", "201327552", "402654144", "805307328", "1610613696", "-1073740864", 
			"200448", "397056", "790272", "1576704", "3149568", "6295296", "12586752", "25169664", "50335488", "100667136", "201330432", "402657024", "805310208", "1610616576", "-1073737984", 
			"211968", "408576", "801792", "1588224", "3161088", "6306816", "12598272", "25181184", "50347008", "100678656", "201341952", "402668544", "805321728", "1610628096", "-1073726464", 
			"258048", "454656", "847872", "1634304", "3207168", "6352896", "12644352", "25227264", "50393088", "100724736", "201388032", "402714624", "805367808", "1610674176", "-1073680384"
		};

};




