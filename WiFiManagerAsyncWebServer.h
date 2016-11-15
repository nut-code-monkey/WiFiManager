#ifndef WiFiManagerAsyncWebServer_h
#define WiFiManagerAsyncWebServer_h

#ifndef ASYNC_WEB_SERVER
#define ASYNC_WEB_SERVER 1
#endif

#ifdef ASYNC_WEB_SERVER

#include <WiFiManagerServerBase.h>
#include <ESPAsyncWebServer.h>
#include <map>

namespace wifi_manager {

  namespace async_web_server{
    class RequestAdapter : public Request, Responce {
      AsyncWebServerRequest* _request;
      std::map<const char*, const char *> _headers;
      const char *_firstKey, *_firstValue;

    public:
      RequestAdapter(AsyncWebServerRequest* request) : _request(request) {}
      virtual ~RequestAdapter() {}
      
      inline
      virtual String host() override {
        return _request->host();
      };
      
      inline
      virtual String uri() override {
        return _request->url();
      };
      
      inline
      virtual bool methodIsGet() override {
        return _request->method() == HTTP_GET;
      };
      
      inline
      virtual String arg(size_t i) override {
        return _request->arg(i);
      }
      
      inline
      virtual String arg(const char *name) override {
        return _request->arg(name);
      }
      
      inline
      virtual size_t args() override {
        return _request->args();
      }
      
      inline
      virtual String argName(size_t i) override {
        return _request->argName(i);
      }
      
      inline
      virtual void redirect(const String& url) override {
        _request->redirect(url);
      }
      
      virtual void send(int status, const char* type, const char* value) override {
        
        auto responce = _request->beginResponse(status, type, value);
        if (_firstKey && _firstValue) {
          responce->addHeader(_firstKey, _firstValue);
        }
        for (auto key: _headers) {
          responce->addHeader(key.first, key.second);
        }
        
        _request->send(responce);
      }
      
      virtual void sendHeader(const char* key, const char * value, bool first = false) override {
        if (first) {
          _firstKey = key;
          _firstValue = value;
        }
        else{
          _headers[key] = value;
        }
      }
    };
  }
  
  template <>
  class Server<AsyncWebServer> : public ServerBase {
    AsyncWebServer _server;
  public:
    Server(const AsyncWebServer& server) : _server(server) {}
    virtual ~Server(){}
    
    virtual void on(const char* path, RestCallback callback) override {
      _server.on(path, HTTP_ANY, [=](AsyncWebServerRequest *request){
        async_web_server::RequestAdapter adapter(request);
        callback((Request*)&adapter, (Responce*)&adapter);
      });
    }
    
    virtual void onNotFound(RestCallback callback)  override {
      _server.onNotFound([=](AsyncWebServerRequest *request){
        async_web_server::RequestAdapter adapter(request);
        callback((Request*)&adapter, (Responce*)&adapter);
      });
    }
    
    virtual void begin() override {
      _server.begin();
    }
    virtual void handleClient() override {}
  };
}

#endif

#endif
