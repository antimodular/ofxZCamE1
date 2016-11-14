#!/usr/bin/env python
#
# Script to parse Charles Proxy sessions saved in JSON format,
# to query the settings of the Z Camera E1 and create api files.
#
## Note: this is a tool to help developing this addon, not to use it.
#
# Instructions:
#
# Collect http traffic from the Z Camera E1 Android application
# https://play.google.com/store/apps/details?id=com.imaginevision.eagle
# In order to do it:
# - use a rooted Android phone in developer mode and install the app 
# - install the Android tools on your development system
#   (on OSX, use "brew install android-platform-tools")
# create an alias to capture network packets from the phone:
# alias adb-tcpdump='adb exec-out "tcpdump -n -s 0 -w - 2>/dev/null"'
# Set the phone to USB debug mode and connect it to the computer.
# Capture packets from a console though USB:
# adb-tcpdump > pcap/capture.pcap
# (captured files are saved in the pcap folder and can have any name)
# Start the app and use all features to capture the api calls
# (it can be done in several passes, saved in the pcap (sub)folder(s))
# Convert the capture.pcap file(s) to a json session file(s):
# - open the .pcap file with Charles Proxy
# - export this session to a JSON Session file (pcap/capture.chlsj)
#


import os
import sys
import json
import glob
from functools import reduce

settings = {}
settings_other = []
settings_unknow = []
actions = {}

host = '10.98.32.1'

# get the path of this script and use it as a relative path
this_path = os.path.dirname(sys.argv[0])


# parsing all Charles Proxy sessions (saved as json)
for json_session_file in glob.glob("%s/pcap/**/*.chlsj"%this_path, 
	recursive = True):

	for request in json.loads(open(json_session_file).read()):
		if not(
			request['host'] == host
			and request['port'] == 80 
			and request['method'] == 'GET'
			and request['status'] == 'COMPLETE'
		):
			continue
		
		# ctrl path only
		path = type(request.get('path') is type('')) \
			and request.get('path') or ''
		if not path.startswith('/ctrl/'):
			#~ print(path)
			continue
			
		ctrl = path.split('/ctrl/')[1].strip()
		
		# valid json response is mandatory
		try:
			response = json.loads(request['response']['body']['text'])
		except:
			response = {}

		query = 'query' in request and request.get('query') or ''
		if not query:
			continue
		
		key, value = query.split('=')

		if ctrl == 'get' and key == 'k' and value not in settings:
			settings_type = response.get('type')
			if settings_type == 1:
				settings[value] = {a:response[a] for a in \
					['key', 'type', 'ro', 'opts']}
			elif settings_type == 2:
				settings[value] = {a:response[a] for a in \
					['key', 'type', 'ro', 'min', 'max', 'step']}
			else:
				settings[value] = None
				
		# save set commands for later
		elif ctrl == 'set' \
		and key not in settings \
		and key not in settings_other:
			settings_other.append(key)
		elif key == 'action' and ctrl not in actions:
			actions[ctrl] = value

#~ sys.exit('...')

# transfer set commands
for key in settings_other:
	if key not in settings:
		settings[key] = None

#~ print("%s settings: %s\n" % (len(settings), settings))
#~ print("%s actions: %s\n" % (len(actions), actions))

# Save setting API as one json file per setting
for key, setting in settings.items():
	with open('%s/data/api/settings/%s.json'%(this_path, key), 'w') as json_file:
			json.dump(setting, json_file, sort_keys=True, indent=2)
		
sys.exit("API files saved in %s/data/api/" % this_path)		


#######################################
## 
#######################################
# query settings

#import sys
#import json

#settings = {}
#actions = {}
# host = '10.98.32.1'

import atexit
import requests

session_is_active = False


# execute http request and collect results
def zcam_request(*argv, fields=[], success_only=False, uri=None):
	#argv : query, key, value,
		
	global host
	global session_is_active
	
	# validations
	if not uri and not session_is_active:
		print("Z Cam session is inactive.")
		return
		
	if uri:
		url = "http://%s%s" % (host, uri)
	else:
		query, key, value = argv
		url = "http://%s/ctrl/%s?%s=%s" % (host, query, key, value)
		
	# request
	r = requests.get(url)
	if not r.ok:
		print("Request failed: %s ; %s" % (r.reason, url))
		return
		
	# result
	try:
		result = json.loads(r.text)
	except:
		result = {}
	
	if success_only:
		return result.get('code') # -1=error, 0=ok, or None
	
	if not fields: # no specific fields: return all
		return result
		
	if type(fields) == type(str):
		return result.get(fields) # returns specific field or None
	elif type(fields) == type(list):
		results = {}
		for field in fields:
			results[field] = result.get(field)
		return result


# get setting
def zcam_setting_get(key, **kwargs): # field='', success_only=False):
	# valid fields: 
	# 'code', 'key', 'value', 'type', 'ro',
	# type 1: 'opts'
	# type 2: 'value', 'min', 'max', 'step'
	return zcam_request(
		'get', 'k', key, **kwargs)


# set setting
def zcam_setting_set(key, value):
	return zcam_request('set', key, value, success_only=True)


# action 
def zcam_action(query, action, **kwargs): # ):
	# valid fields: 'code'
	result = zcam_request(
		query, 'action', action, success_only=True)
	return bool(result) and bool(result.get('code') == 0)
	

# get info about camera (works without an active session)
def zcam_info():
	return zcam_request(uri='/info')


# handle session
def zcam_session(quit=False):
	if quit and not session_is_active:
		return False
	action = ('heart_x_beat', 'quit') [quit]
	result = zcam_request(success_only=True,
		uri="/ctrl/session?action=%s"%action)
	return result == 0 and not quit

# unregister camera session at exit time
atexit.register(zcam_session, True)


# from http://stackoverflow.com/questions/2953462/pinging-servers-in-python
def ping(host):
    """
    Returns True if host responds to a ping request
    """
    import os, platform

    # Ping parameters as function of OS
    ping_str = "-n 1" if  platform.system().lower()=="windows" else "-c 1"

    # Ping
    return os.system("ping " + ping_str + " " + host) == 0

# Initialisation
if not ping(host):
	sys.exit('Z Camera not reachable; check your wifi connection to it')
print()

info = zcam_info()
if info:
	print("Z Camera : ", info)
# note: this script was made for :
# {'sw': '0.21', 'hw': '3', 'ble': '0.12', 'model': 'E1'}

else:
	sys.exit("Z Camera : no answer")

session_is_active = zcam_session()
if session_is_active:
	print("session opened")
else:
	sys.exit('session not opened')

# save all settings to json files (one per setting)
settings_keys = [] # memorize keys
for setting in settings:
	data = zcam_setting_get(setting)
	if not data:
		print("no answer for : %s\n" % setting)
		continue
	settings_keys.append(list(data.keys()))
	with open('%s/json/%s.json'%(this_path, setting), 'w') as json_file:
		json.dump(data, json_file, sort_keys=True, indent=2)
	print("%s : %s\n" % (setting, data))

# list of common keys, for validation
common_keys = list(reduce(set.intersection, map(set, settings_keys)))
print("common keys : %s\n" % common_keys)

settings_rw = []
settings_ro = []
# reload all rw settings
for json_file in glob.glob("%s/json/*.json"%this_path):
	with open(json_file, 'r') as json_handler:
		data =  json.load(json_handler)
		#~ print(data)
		#~ continue
		if not data: continue
		if not set(common_keys).issubset(data):
			print("data is missing common keys: %s" % data)
			continue
		if data['ro']:
			#~ print("data is read only: %s" % data)
			settings_ro.append(data['key'])
			continue
		settings_rw.append(data['key'])
		result = zcam_setting_set(data['key'], data['value'])
		if result != 0:
			print("Error at setting : %s = %s ; %s", data['key'], data['value'], result)

print("%s rw settings : %s\n" % (len(settings_rw), settings_rw) )
print("%s ro settings : %s\n" % (len(settings_ro), settings_ro) )

sys.exit("That's all folks!")
