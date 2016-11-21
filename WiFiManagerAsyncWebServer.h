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
  
    template <typename T>
    class Adapter : public T{
      virtual ~Adapter() override {}
    };
      
    template <>
    class Adapter<Responce> : public Responce{
      AsyncWebServerRequest* _request;
      std::map<String, String> _headers;
      String _firstKey, _firstValue;
    public:
      
      Adapter(AsyncWebServerRequest* request) : _request(request){
      }
      
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
    
    template<>
    class Adapter<Request> : public Request {
      AsyncWebServerRequest* _request;
    public:
      
      Adapter(AsyncWebServerRequest* request) : _request(request){
      }
      
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
        _request->redirect(url);
      }
    };
  }
  
  template <>
  class Server<AsyncWebServer *> : public ServerBase {
    AsyncWebServer* _server;
  public:
    Server(AsyncWebServer* server) : _server(server) {}
    virtual ~Server() override {
      reset();
    }
    
    virtual void handle(const String& path, RestCallback callback) override {
      using namespace async_web_server;
      
      _server->on(path.c_str(), HTTP_ANY, [=](AsyncWebServerRequest *request){
        Adapter<Request> requestAdapter(request);
        Adapter<Responce> responceAdapter(request);
        callback(&requestAdapter, &responceAdapter);
      });
    }
    
    virtual void onNotFound(RestCallback callback)  override {
      using namespace async_web_server;
      
      _server->onNotFound([=](AsyncWebServerRequest *request){
        Adapter<Request> requestAdapter(request);
        Adapter<Responce> responceAdapter(request);
        callback(&requestAdapter, &responceAdapter);
      });
    }
    
    virtual void begin() override {
      _server->begin();
    }
    virtual void handleClient() override {}
    
    virtual void reset() override {
      _server->reset();
    }
  };
}

#endif

#endif
