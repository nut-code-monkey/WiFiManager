#ifndef WiFiManagerAsyncWebServer_h
#define WiFiManagerAsyncWebServer_h

#ifndef ASYNC_WEB_SERVER

#include <WiFiManagerServerBase.h>
#include <ESP8266WebServer.h>

namespace wifi_manager {
  
  template <>
  class Server<ESP8266WebServer> : public ServerBase, Request, Responce {
    ESP8266WebServer _server;
  public:
    Server(const ESP8266WebServer &server) : _server(server){}
    virtual ~Server() {}
    
#pragma mark - Request
    inline
    virtual void on(const char * path, RestCallback callback) override {
      _server.on(path, std::bind(callback, (Request *)this, (Responce *)this));
    }
    
    inline
    virtual void onNotFound(RestCallback callback) override {
      _server.onNotFound(std::bind(callback, (Request *)this, (Responce *)this));
    }
    
    inline
    virtual void begin() override {
      _server.begin();
    }
    
    inline
    virtual void handleClient() override {
      _server.handleClient();
    }
    
#pragma mark - Request
    inline
    virtual String host() override {
      return _server.hostHeader();
    };
    
    inline
    virtual String uri(){
      return _server.uri();
    };
    
    inline
    virtual bool methodIsGet() override {
      return _server.method() == HTTPMethod::HTTP_GET;
    };
    
    inline
    virtual size_t args() override {
      return _server.args();
    };
    
    inline
    virtual String arg(size_t i) override {
      return _server.arg(i);
    }
    
    inline
    virtual String arg(const char * name) override {
      return _server.arg(name);
    }
    
    inline
    virtual String argName(size_t i) override {
      return _server.argName(i);
    }
    
#pragma mark - Responce
    inline
    virtual void send(int code, const char* type, const char* value) override {
      _server.send(code, type, value);
    }
    
    inline
    virtual void sendHeader(const char* key, const char *value, bool first = false) override {
      _server.sendHeader(key, value, first);
    }
    
    virtual void redirect(const String& url) override {
      _server.sendHeader("Location", url.c_str(), true);
      _server.setContentLength(0);
      _server.send ( 302, "text/plain", "");
        // Empty content inhibits Content-length header so we have to close the socket ourselves.
        // server->client().stop(); // Stop is needed because we sent no content length
    }
  };
}

#endif

#endif
