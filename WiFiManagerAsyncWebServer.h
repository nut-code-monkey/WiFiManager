#ifndef WiFiManagerAsyncWebServer_h
#define WiFiManagerAsyncWebServer_h

#ifndef EXTERNAL_WEB_SERVER
#define EXTERNAL_WEB_SERVER 1
#endif

#ifdef EXTERNAL_WEB_SERVER

#include <WiFiManagerServerBase.h>
#include <ESPAsyncWebServer.h>
#include <map>

namespace wifi_manager {

  namespace async_web_server{
    class ResponceAdapter : public Responce{
      AsyncWebServerRequest* _request;
      std::map<String, String> _headers;
      String _firstKey, _firstValue;
    public:
      ResponceAdapter(AsyncWebServerRequest* request) : _request(request){
      }
      virtual ~ResponceAdapter(){}
      
      virtual void send(int status, const String& type, const String& value) override {
        
        auto responce = _request->beginResponse(status, type, value);
        if (_firstKey && _firstValue) {
          responce->addHeader(_firstKey, _firstValue);
        }
        for (const auto& key: _headers) {
          responce->addHeader(key.first, key.second);
        }

        _request->send(responce);
      }
      
      virtual void setHeader(const String& key, const String& value, bool first = false) override {
        if (first) {
          _firstKey = key;
          _firstValue = value;
        }
        else{
          _headers[key] = value;
        }
      }
    };
    
    class RequestAdapter : public Request {
      AsyncWebServerRequest* _request;
    public:
      RequestAdapter(AsyncWebServerRequest* request) : _request(request) {
      }
      virtual ~RequestAdapter() {}
      
      virtual String host() override {
        return _request->host();
      };
      
      virtual String uri() override {
        return _request->url();
      };
      
      virtual String methodString() override {
        return _request->methodToString();
      };
      
      virtual String arg(size_t i) override {
        return _request->arg(i);
      }
      
      virtual String arg(const String& name) override {
        return _request->arg(name.c_str());
      }
      
      virtual size_t args() override {
        return _request->args();
      }
      
      virtual String argName(size_t i) override {
        return _request->argName(i);
      }
      
      virtual void redirect(const String& url) override {
        AsyncWebServerResponse * response = _request->beginResponse(302);
        response->addHeader("Location",url);
        response->addHeader("text/plain", "");
        response->setContentLength(0);
        _request->send(response);
      }
    };
  }
  
  template <>
  class Server<AsyncWebServer *> : public ServerBase {
    AsyncWebServer* _server;
    
  public:
      // server not owned AsyncServer
    Server(AsyncWebServer* server) : _server(server) {
    }
    virtual ~Server(){
//      _server->resetAllHandlers();
//      _server = nullptr;
    }
    
    virtual void on(const String& path, RestCallback callback) override {
      
      _server->on(path.c_str(), [=](AsyncWebServerRequest *request){
        Serial.printf("-> %s %s\n", request->methodToString(), path.c_str());
        
        async_web_server::RequestAdapter requestAdapter(request);
        async_web_server::ResponceAdapter responceAdapter(request);
        callback(&requestAdapter, &responceAdapter);
      });
    }
    
    virtual void onNotFound(RestCallback callback)  override {
      _server->onNotFound([=](AsyncWebServerRequest *request){
        async_web_server::RequestAdapter requestAdapter(request);
        async_web_server::ResponceAdapter responceAdapter(request);
        callback(&requestAdapter, &responceAdapter);
      });
    }
    
    virtual void begin() override {
      _server->begin();
    }
    virtual void handleClient() override {}
    
    virtual void reset() override {
//      _server->resetAllHandlers();
    }
  };
}

#endif

#endif
