/*
 * udpsvr.h
 *
 *  Created on: 2014.4.6
 *      Author: harlylei
 */

#ifndef UDPSVR_H_
#define UDPSVR_H_

int openudpclient(const char * ip, unsigned short port, char * errbuf);

class CUdpServer {
public:
    CUdpServer();

    virtual bool init() {return true; }

    int open(const char* ip, unsigned short port);

    void loopdealreq();

    const char *getErrMsg() const { return m_errbuf; }

    void stop_udpsvr() { m_stop = true; }

    virtual bool do_request(const char *buf __attribute__((unused)),
                            int len __attribute__((unused)), const char *ip __attribute__((unused))) {
      return true;
    }

    virtual ~CUdpServer() {}

private:
    int m_listenfd;
    bool m_stop;

protected:
    char m_errbuf[1024];
};

#endif /* UDPSVR_H_ */
