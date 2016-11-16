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
      
      inline
      virtual String host() override {
        return _request->host();
      };
      
      inline
      virtual String uri() override {
        return _request->url();
      };
      
      inline
      virtual String methodString() override {
        return _request->methodToString();
      };
      
      inline
      virtual String arg(size_t i) override {
        return _request->arg(i);
      }
      
      inline
      virtual String arg(const String& name) override {
        return _request->arg(name.c_str());
      }
      
      inline
      virtual size_t args() override {
        return _request->args();
      }
      
      inline
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
    
    struct WebHandler{
      WebHandler *next;
      AsyncCallbackWebHandler *handler;
      WebHandler(AsyncCallbackWebHandler *h): handler(h){}
      ~WebHandler(){
        if (next) delete next;
        handler->onRequest(NULL);
      }
    };
  }
  
  template <>
  class Server<AsyncWebServer *> : public ServerBase {
    AsyncWebServer* _server;
    async_web_server::WebHandler* _handler;
  public:
    Server(AsyncWebServer* server) : _server(server) {
    }
    virtual ~Server(){
      reset();
    }
    
    void addHendler(AsyncCallbackWebHandler *handler){
      using namespace async_web_server;
      
      if (!_handler) {
        _handler = new WebHandler(handler);
      }
      else{
        WebHandler *h = _handler;
        while (h->next) {
          h = h->next;
        }
        h->next = new WebHandler(handler);
      }
    }
    
    virtual void on(const String& path, RestCallback callback) override {
      using namespace async_web_server;
      
      auto handler = _server->on(path.c_str(), [=](AsyncWebServerRequest *request){
        Adapter<Request> requestAdapter(request);
        Adapter<Responce> responceAdapter(request);
        callback(&requestAdapter, &responceAdapter);
      });

      addHendler(&handler);
    }
    
    virtual void onNotFound(RestCallback callback)  override {
      using namespace async_web_server;
      
      _server->onNotFound([=](AsyncWebServerRequest *request){
        Adapter<Request> requestAdapter(request);
        Adapter<Responce> responceAdapter(request);
        callback(&requestAdapter, &responceAdapter);
      });
    }
    
    inline
    virtual void begin() override {
      _server->begin();
    }
    virtual void handleClient() override {}
    
    virtual void reset() override {
      if (_handler) {
        delete _handler;
        _handler = NULL;
      }
      _server->onNotFound(NULL);
    }
  };
}

#endif

#endif
