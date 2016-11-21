#ifndef ServerBase_h
#define ServerBase_h

namespace wifi_manager {
  class Request{
  public:
    virtual String host() = 0;
    virtual String uri() = 0;
    virtual String methodString() = 0;
    virtual String arg(size_t) = 0;
    virtual String arg(const String& argName) = 0;
    virtual size_t args() = 0;
    virtual String argName(size_t i) = 0;
    virtual void redirect(const String& url) = 0;
    
    virtual ~Request(){};
  };
  
  class Responce {
  public:
    virtual void send(int code, const String& type, const String& content) = 0;
    virtual void setHeader(const String& key, const String& value, bool first = false) = 0;
    
    virtual ~Responce(){};
  };
  
  typedef std::function<void(Request *, Responce *)> RestCallback;
  
  class ServerBase{
  public:
    virtual void handle(const String& path, RestCallback callback) = 0;
    virtual void onNotFound(RestCallback) = 0;
    virtual void begin() = 0;
    virtual void handleClient() = 0;
    virtual void reset() = 0;
    
    virtual ~ServerBase(){};
  };
  
  template <typename T>
  class Server : public ServerBase, Request, Responce {
  };
}

#endif
