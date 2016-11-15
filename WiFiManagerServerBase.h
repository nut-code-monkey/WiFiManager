#ifndef ServerBase_h
#define ServerBase_h

namespace wifi_manager {
  class Request{
  public:
    virtual String host() = 0;
    virtual String uri() = 0;
    virtual bool methodIsGet() = 0;
    virtual String arg(size_t) = 0;
    virtual String arg(const char *) = 0;
    virtual size_t args() = 0;
    virtual String argName(size_t i) = 0;
    virtual void redirect(const String& url) = 0;
    virtual ~Request(){};
  };
  
  class Responce {
  public:
    virtual void send(int, const char* type, const char* content) = 0;
    virtual void sendHeader(const char*, const char *, bool first = false) = 0;
    
    virtual ~Responce(){};
  };
  
  typedef std::function<void(Request *, Responce *)> RestCallback;
  
  class ServerBase{
  public:
    virtual void on(const char* path, RestCallback) = 0;
    virtual void onNotFound(RestCallback) = 0;
    virtual void begin() = 0;
    virtual void handleClient() = 0;
    virtual ~ServerBase(){};
  };
  
  template <typename T>
  class Server : public ServerBase, Request, Responce {
  };
}

#endif
