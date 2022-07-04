#ifndef __WEBSOCKETSTREAMCLIENT_H_
#define __WEBSOCKETSTREAMCLIENT_H_

#include <WiFiClientSecure.h>
#include <WebSocketClient.h>
#include <Client.h>

class WebSocketStreamClient : public Client
{
  public:

    WebSocketStreamClient(WebSocketClient &webSocketClient, const char *path)
    {
      _webSocketClient = &webSocketClient; 
      _path = path;
    }

    int connect(IPAddress ip, uint16_t port)
    {
      _webSocketClient->begin(_path);
      return 1;
    }

    int connect(const char *host, uint16_t port)
    {
      _webSocketClient->begin(_path);
      return 1;
    }

    size_t write(uint8_t b)
    {
      if (!connected())
        return -1;
      return write(&b, 1);
    }

    size_t write(const uint8_t *buf, size_t size)
    {
      if (!connected())
        return -1;
      _webSocketClient->beginMessage(TYPE_BINARY);
      _webSocketClient->write(buf, size);
      _webSocketClient->endMessage();
      return size;
    }

    int available()
    {
      if (!connected())
        return 0;
      if (_webSocketClient->available() == 0)
        _webSocketClient->parseMessage();
      return _webSocketClient->available();
    }

    int read()
    {
      if (!connected() || !available())
        return -1;
      return _webSocketClient->read();
    }

    int read(uint8_t *buf, size_t size)
    {
      if (!connected() || !available())
        return -1;
      return _webSocketClient->read(buf, size);
    }

    int peek()
    {
      if (!connected() || !available())
        return -1;
      return _webSocketClient->peek();
    }

    void flush() {
      if (connected())
        _webSocketClient->flush();
    }

    void stop()
    {
      if (connected())
        _webSocketClient->stop();
    }

    uint8_t connected()
    {
      return _webSocketClient->connected();
    }

    operator bool()
    {
      return _webSocketClient != NULL;
    }

  private:
    WebSocketClient *_webSocketClient; 
    const char *_path;
};

#endif