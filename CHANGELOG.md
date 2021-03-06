# Change Log

## [0.5 - Development Version]
### Added Subtypes:
- trigger - momentary switches
- ORP - oxidation redux potential
- salinity - Salinity (ppt)
- daylight - Simple daylight state
- moonphase - Phase of moon in %full
- tristate - On/Off/Etc
### Added Collectors:
- astrocoll - Gather daylight/moonlight data from various sources
- balboacoll - Balboa wifi spa controller
- alarmcoll - A collector that generates alarms for events
### Added Commands:
- apiv - Get api version of gnhastd
### New Features:
- owsrvcoll - Add support for moisture and wetness Hobby Boards sensors.
- insteroncoll - Rewrite how we pull data off the PLM and process.
- Add a protocol API version command and specifier
- Break out the UDP 30303 discovery code into it's own common library so
  multiple collectors can use it.
- Convert common into a libgnhast shared library
- Update libconfuse to 3.0

## [0.4 - Release Version]
### Added Collectors:
- moncoll
- jsoncgicoll
### Added Tools:
- gnhastweb
- gtk-insteonedit (partially complete)
###	Added Commands:
- cfeed
- setalarm
- dumpalarms
- listenalarms
- ping
- imalive
###	New Features:

- Support for new style insteon HUBs.

- Collectors now have self-health checks, and gnhastd monitors them.  A
  collector will be flagged as bad if it doesn't send updates, or fails
  to respond to "pings".  This can help with certain devices that get
  wedged up after awhile, or collectors that die off mysteriously.

- moncoll collector is a special collector that interfaces with the
  collector devices created on the gnhast server, and when a collector
  is marked bad, it can restart the collector, forcibly if needed.

- cfeed is a new type of feed to monitor switches.  Unlike the usual
  timed feed, cfeed will notify the listeners immediately upon change.

- Add an alarm subsystem.  This is very basic.  Each alarm has a
  user-generated uid, a blob of text, and an arbitrary severity number.
  This can be used by scripts to post generic alarms to other things.
  Currently nothing uses this, eventually the web interface will be
  made aware of it.

- gtk-insteonedit is a tool to allow link editing of insteon devices
  directly.  It is still partially complete.

- gnhastweb is a full fledged web GUI for gnhast.  It allows basic
  editing of devices, and placing devices into groups.  However, what
  it is actually designed for, is to give you a tablet/phone based
  control panel for your house.  Each device is a small "card" on the UI,
  and you can interact with them.  They are dynamically updated with the
  current status, and you can do things like flip switches on and off,
  adjust dimmers, etc.  It can also interface with your rrd files
  generated by rrdcoll to display simple graphs of sensor data.  The
  interface is completely configurable via conf file.  You can even
  substitute hand-generated html for some, or all of the groups, to do
  something like use an image map of your house as the navigation.  More
  documentation on this will need to be provided.
	
## [0.3 - Release Version (never actually released, sorry) ]
- Add SIGUSR1 handling to all collectors (flip debug mode switch)
- Add rpm spec file	

## [0.2.4 - Development Version]
###	Added Collectors:
- ad2usbcoll
- icaddycoll
- venstarcall
- gtk-gnhast
###	Added Handlers:
- timer
- timer2
###	Added Tools:
- ssdp_scan
- modhargs
- addhandler
- notify_listen
- venstar_stats
###	New Features:

- Added gtk-gnhast. A gtk2 application that allows for easy overview and
  editing of your setup.  Designed to make creation of groups easier.
	
- Fully flesh out group system.  Now devices can be members of groups,
  as well as groups being members of other groups.

- New command "regg" to register a group.

- New command "lgrps" to list groups.

- Add venstar_stats script, to pull daily usage data off the venstar and
  save to an rrd.  This doesn't fit well with the general gnhastd
  concept, so did it as an extra. (only useful for graphing).

- Add Venstar T5900/T5800 collector for Venstar Thermostats.

- Added http and jsmn helper routines to ease coding of future
  http/json stuff.

- Add icaddycoll, the collector for the IrrigationCaddy. Can read status
  of valves, programs, and run individual valves or programs.

- Add ssdp_scan and notify_listen.  Simple tools to view SSDP and UPNP
  data on the network, and see what is out there.

- Imported jsmn from: http://zserge.bitbucket.org

- More Insteon fixes, change the way groups work in the insteon tools.

- Added automatic reset code to the brultech collector.  If it detects
  a hang condition, will automatically connect to the wiznet device and
  reset it.

- Add tools directory.  Add modhargs and addhandler, simple scripts to
  patch a handler or hargs into a running gnhast instance.

- Add example timer scripts, timer and timer2, for setting a light on
  for a given time when a condition occurs.

- Add ad2usbcoll collector for the NuTech AD2USB device.  Interfaces
  with Honeywell Vista alarms, and allows reading of alarm states.

- Added new subtypes, alarmstatus, number, percentage,
  thmode (thermostat mode), smnumber (small number), flowrate, distance,
  volume (sound).

## [0.2.3 - Release 7/9/13]
### Added Collectors:
- wupwscoll
- wmr918coll
###	Added Handlers:
- nightlight
- switchon
- switchoff
### New features:
	
- Make gnhastd aware of the unit that data is stored in.
  Now a collector tells gnhast what scale it is feeding data in
  (ex, Celcius). When requesting data, you can request it in a
  different scale, and the server will auto-caclulate.
	
- Add a wupwscoll collector.  This collector will publish weather data
  to weather underground, and pwsweather.com.
	
- Add wmr918coll collector.  Collects data from oregon scientific
  weather stations. Works on wx200's wmr918's, and can also connect to
  a running wx200d instance and gather data from that.
	
- Insteon i2cs code now tested and working fully.
	
- Added basic handlers for a nightlight routine, and switching devices
  on and off.
	
- Add a modify command to gnhastd, so you can edit the config file on
  the fly by sending it simple commands.  This allows you to add a
  handler or tweak names/watermarks on a running instance.
	
- Port to Linux.

## [0.2 - Initial Release 5/29/13]
### Added Collectors:
- brulcoll
- insteoncoll
- owsrvcoll

## [0.1 - Initial Development Version 4/13/13]
