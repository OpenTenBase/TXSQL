/*
 * udpsvr.cpp
 *
 *  Created on: 2014.4.6
 *      Author: harlylei
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>

#include <string>

#include "udpsvr.h"

using namespace std;

int openudpclient(const char* ip, unsigned short port, char* errbuf) {
    int connfd = -1;

    connfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (connfd < 0) {
        snprintf(errbuf, 1024, "open udp socket error:[%d:%s]", errno, strerror(errno));
        return connfd;
    }

    struct sockaddr * addr = NULL;
    struct sockaddr_in servaddr;

    socklen_t socklen = 0;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    addr =(struct sockaddr *) &servaddr;
    socklen = sizeof(struct sockaddr_in);

    int iret = connect(connfd, addr, socklen);
    if (iret < 0) {
        snprintf(errbuf, 1024, "open udp connect[%s,%d] error:[%d:%s]", ip, port, errno, strerror(errno));
        close(connfd);

        return -1;
    }

    //set to non-blocked 
    int val;
    if ((val = fcntl(connfd, F_GETFL, 0)) == -1) {
        snprintf(errbuf, 1024, "open udp fcntl F_GETFL error:[%d:%s]", errno, strerror(errno));
        close(connfd);
        return -1;
    }

    if (fcntl(connfd, F_SETFL, val | O_NONBLOCK) == -1) {
        snprintf(errbuf, 1024, "open udp fcntl F_SETFL error:[%d:%s]", errno, strerror(errno));
        close(connfd);
        return -1;
    }

    return connfd;
}

CUdpServer::CUdpServer() {
    m_listenfd = -1;
    bzero(m_errbuf, sizeof(m_errbuf));
    m_stop = false;
}

/// @param ip ipV4 address string.
/// @param port port number in host byte order.
int CUdpServer::open(const char* ip, unsigned short port) {
    if (m_listenfd >= 0) {
        return m_listenfd;
    }

    m_listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_listenfd < 0) {
        snprintf(m_errbuf, sizeof(m_errbuf), "CUdpServer::open socket error[%d][%s]", errno, strerror(errno));
        return -1;
    }

    int iReuseAddrFlag = 1;
    int ret = setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR,(char*) &iReuseAddrFlag, sizeof(iReuseAddrFlag));
    if (ret < 0) {
        snprintf(m_errbuf, sizeof(m_errbuf), "CUdpServer::open setsocopt error[%d][%s]", errno, strerror(errno));
        goto err;
    }
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;

    // mysql uses "*" to represent 'any ip'.
    if (strcmp(ip, "*") == 0)
        ip= "0.0.0.0";

    if ((ret = inet_aton(ip, &servaddr.sin_addr)) == 0) {
        snprintf(m_errbuf, sizeof(m_errbuf), "CUdpServer::open invalid ipv4 address: [%s]", ip);
        goto err;
    }

    servaddr.sin_port = htons(port);
    ret = bind(m_listenfd,(struct sockaddr*) &servaddr, sizeof(servaddr));
    if (ret < 0) {
        snprintf(m_errbuf, sizeof(m_errbuf), "CUdpServer::open bind error[%d][%s]", errno, strerror(errno));
        goto err;
    }

    fprintf(stderr, "\nMaster UDP server listening on IP(%s), port(%u), socket fd:%d.\n",
            ip, port, m_listenfd);
    
    return m_listenfd;
err:
    close(m_listenfd);
    m_listenfd = -1;
    return -1;
}

void CUdpServer::loopdealreq() {
    struct sockaddr_in from;
    socklen_t fromlen =(socklen_t) sizeof(from);
    char fromaddr[24] = {'\0'};

    int ret = 0;
    const int maxRecvLen = 1024 * 1024;
    char* recvbuf = new char[maxRecvLen];
    int recvlen = maxRecvLen;

    while (!m_stop) {
        ret = recvfrom(m_listenfd, recvbuf, recvlen, 0, (sockaddr*)(&from), &fromlen);

        if (ret > 0) {
            if (ret >= recvlen)// otherwise the message may have been truncated.
                fprintf(stderr, "recvfrom received %d bytes, equals to message buffer size(%d), "
                        "from int ip:%u, data may be truncated.",
                         ret, recvlen, from.sin_addr.s_addr);
            strncpy(fromaddr, inet_ntoa(from.sin_addr), strlen(fromaddr));
            do_request(recvbuf, ret, fromaddr);
        } else {
            fprintf(stderr, "recvfrom ret:%d<=0,errno:%d,errstr:%s,from int ip:%u",
                    ret, errno, strerror(errno), from.sin_addr.s_addr);
        }
    }

    if (recvbuf)
      delete []recvbuf;
}
