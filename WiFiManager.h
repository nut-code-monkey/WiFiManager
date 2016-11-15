/**************************************************************
   WiFiManager is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to enable easy
   configuration and reconfiguration of WiFi credentials using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/esp8266/hardware/esp8266com/esp8266/libraries/DNSServer/examples/CaptivePortalAdvanced
   Built by AlexT https://github.com/tzapu
   Licensed under MIT license
 **************************************************************/

#ifndef WiFiManager_h
#define WiFiManager_h

#include <ESP8266WiFi.h>

#include "WiFiManagerServerBase.h"

#ifndef ASYNC_WEB_SERVER
#include "WiFiManagerESP8266WebServer.h"
#include <ESP8266WebServer.h>
#else
#include "WiFiManagerAsyncWebServer.h"
#endif

#include <DNSServer.h>
#include "FS.h"
#include <ArduinoJson.h>
#include <memory>
#undef min
#undef max
#include <algorithm>
extern "C" {
  #include "user_interface.h"
}

#define WFM_LABEL_BEFORE 1
#define WFM_LABEL_AFTER 2
#define WFM_NO_LABEL 0

#define WIFI_MANAGER_MAX_PARAMS 10

namespace wifi_manager{

  class Parameter {
  public:
    Parameter(const char *custom);
    Parameter(const char *id, const char *placeholder, const char *defaultValue, int length);
    Parameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);
    Parameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom, int labelPlacement);

    const char *getID() const;
    const char *getValue() const;
    const char *getPlaceholder() const;
    size_t      getValueLength() const;
    int         getLabelPlacement() const;
    const char *getCustomHTML() const;
  private:
    const char *_id;
    const char *_placeholder;
    char       *_value;
    size_t      _length;
    int         _labelPlacement;
    const char *_customHTML;

    void init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom, int labelPlacement);
  };
}

class WiFiManager
{
  std::function<wifi_manager::ServerBase *()> newServer;
public:
  
  #ifndef ASYNC_WEB_SERVER
    WiFiManager() : newServer([]{
      return new wifi_manager::Server<ESP8266WebServer>(ESP8266WebServer(80));
    }) {}
  #else
    template <typename T>
    WiFiManager(T t) : newServer([&](){
      return new wifi_manager::Server<T>(t);
    }) {}
  #endif
  
    boolean       autoConnect(); //Deprecated. Do not use.
    boolean       autoConnect(char const *apName, char const *apPassword = NULL); //Deprecated. Do not use.

    //if you want to start the config portal
    boolean       startConfigPortal();
    boolean       startConfigPortal(char const *apName, char const *apPassword = NULL);

    // get the AP name of the config portal, so it can be used in the callback
    String        getConfigPortalSSID();

    void          resetSettings();

    //sets timeout before webserver loop ends and exits even if there has been no setup.
    //usefully for devices that failed to connect at some point and got stuck in a webserver loop
    //in seconds setConfigPortalTimeout is a new name for setTimeout
    void          setConfigPortalTimeout(unsigned long seconds);
    void          setTimeout(unsigned long seconds);

    //sets timeout for which to attempt connecting, usefull if you get a lot of failed connects
    void          setConnectTimeout(unsigned long seconds);


    void          setDebugOutput(boolean debug);
    //defaults to not showing anything under 8% signal quality if called
    void          setMinimumSignalQuality(int quality = 8);
    //sets a custom ip /gateway /subnet configuration
    void          setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //sets config for a static IP
    void          setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //called when AP mode and config portal is started
    void          setAPCallback( void (*func)(WiFiManager*) );
    //called when settings have been changed and connection was successful
    void          setSaveConfigCallback( void (*func)(void) );
    //adds a custom parameter
    void          addParameter(wifi_manager::Parameter *p);
    //if this is set, it will exit after config, even if connection is unsucessful.
    void          setBreakAfterConfig(boolean shouldBreak);
    //if this is set, try WPS setup when starting (this will delay config portal for up to 2 mins)
    //TODO
    //if this is set, customise style
    void          setCustomHeadElement(const char* element);
    //if this is true, remove duplicated Access Points - defaut true
    void          setRemoveDuplicateAPs(boolean removeDuplicates);
    //Scan for WiFiNetworks in range and sort by signal strength
    //space for indices array allocated on the heap and should be freed when no longer required
    int           scanWifiNetworks(int **indicesptr);
    // Get static credentials from config
    void getStatCred();
    // Remove WIFI coonfig with static credentials
    void removeWFConfig();
    
  private:
    std::unique_ptr<DNSServer>            dnsServer;
    std::unique_ptr<wifi_manager::ServerBase> server;

    //const int     WM_DONE                 = 0;
    //const int     WM_WAIT                 = 10;

    //const String  HTTP_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

    void          setupConfigPortal();
    void          startWPS();
    char*         getStatus(int status);

    const char*   _apName                 = "no-net";
    const char*   _apPassword             = NULL;
    String        _ssid                   = "";
    String        _pass                   = "";
    unsigned long _configPortalTimeout    = 0;
    unsigned long _connectTimeout         = 0;
    unsigned long _configPortalStart      = 0;
    /* hostname for mDNS. Set to a valid internet address so that user
    will see an information page if they are connected to the wrong network */
	  const char *myHostname = "wifi.urremote.com";

    IPAddress     _ap_static_ip;
    IPAddress     _ap_static_gw;
    IPAddress     _ap_static_sn;
    IPAddress     _sta_static_ip;
    IPAddress     _sta_static_gw;
    IPAddress     _sta_static_sn;

    int           _paramsCount            = 0;
    int           _minimumQuality         = -1;
    boolean       _removeDuplicateAPs     = true;
    boolean       _shouldBreakAfterConfig = false;
    boolean       _tryWPS                 = false;

    const char*   _customHeadElement      = "";

    //String        getEEPROMString(int start, int len);
    //void          setEEPROMString(int start, int len, String string);

    int           status = WL_IDLE_STATUS;
    int           connectWifi(String ssid, String pass);
    uint8_t       waitForConnectResult();

    void          handleRoot(wifi_manager::Request*, wifi_manager::Responce*);
    void          handleWifi(wifi_manager::Request*, wifi_manager::Responce*, boolean scan);
    void          handleWifiSave(wifi_manager::Request*, wifi_manager::Responce*);
    void          handleServerClose(wifi_manager::Request*, wifi_manager::Responce*);
    void          handleInfo(wifi_manager::Request*, wifi_manager::Responce*);
    void          handleState(wifi_manager::Request*, wifi_manager::Responce*);
    void          handleScan(wifi_manager::Request*, wifi_manager::Responce*);
    void          handleReset(wifi_manager::Request*, wifi_manager::Responce*);
    void          handleNotFound(wifi_manager::Request*, wifi_manager::Responce*);
    boolean       captivePortal(wifi_manager::Request*, wifi_manager::Responce*);
    void          reportStatus(String &page);

    // DNS server
    const byte    DNS_PORT = 53;

    //helpers
    int           getRSSIasQuality(int RSSI);
    boolean       isIp(String str);
    String        toStringIp(IPAddress ip);

    boolean       connect;
    boolean       stopConfigPortal = false;
    boolean       _debug = true;

    void (*_apcallback)(WiFiManager*) = NULL;
    void (*_savecallback)(void) = NULL;

    wifi_manager::Parameter* _params[WIFI_MANAGER_MAX_PARAMS];

    template <typename Generic>
    void          DEBUG_WM(Generic text);

    template <class T>
    auto optionalIPFromString(T *obj, const char *s) -> decltype(  obj->fromString(s)  ) {
      return  obj->fromString(s);
    }
    auto optionalIPFromString(...) -> bool {
      DEBUG_WM("NO fromString METHOD ON IPAddress, you need ESP8266 core 2.1.0 or newer for Custom IP configuration to work.");
      return false;
    }
};
#endif
