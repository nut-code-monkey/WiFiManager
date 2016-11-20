#ifndef WiFiManagerAsyncWebServer_h
#define WiFiManagerAsyncWebServer_h

#ifndef EXTERNAL_WEB_SERVER

#include <WiFiManagerServerBase.h>
#include <ESP8266WebServer.h>

namespace wifi_manager {
  
  template <>
  class Server<ESP8266WebServer> : public ServerBase, Request, Responce {
    ESP8266WebServer _server;
  public:
    Server(const ESP8266WebServer &server) : _server(server){}
    virtual ~Server() {}
    
    virtual void on(const String& path, RestCallback callback) override {
      _server.on(path.c_str(), std::bind(callback, static_cast<Request *>(this), static_cast<Responce *>(this)));
    }
    
    virtual void onNotFound(RestCallback callback) override {
      _server.onNotFound(std::bind(callback, static_cast<Request *>(this), static_cast<Responce *>(this)));
    }
    
    virtual void begin() override {
      _server.begin();
    }
    
    virtual void handleClient() override {
      _server.handleClient();
    }
    
    virtual void reset() override {
    }
    
    virtual String host() override {
      return _server.hostHeader();
    };
    
    virtual String uri() override {
      return _server.uri();
    };
    
    virtual String methodString() override {
      switch (_server.method()) {
        case HTTP_ANY:
          return "ANY";
        case HTTP_GET:
          return "GET";
        case HTTP_POST:
          return "POST";
        case HTTP_PUT:
          return "PUT";
        case HTTP_PATCH:
          return "PATCH";
        case HTTP_DELETE:
          return "DELETE";
        default:
          return "<NONE>";
      }
    };
    
    virtual size_t args() override {
      return _server.args();
    };
    
    virtual String arg(size_t i) override {
      return _server.arg(i);
    }
    
    virtual String arg(const String& name) override {
      return _server.arg(name);
    }
    
    virtual String argName(size_t i) override {
      return _server.argName(i);
    }
    
    virtual void send(int code, const String& type, const String& value) override {
      _server.send(code, type, value);
    }
    
    virtual void setHeader(const String& key, const String& value, bool first = false) override {
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
