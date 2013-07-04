/*
 * Unit test suite for winsock functions
 *
 * Copyright 2002 Martin Wilck
 * Copyright 2005 Thomas Kho
 * Copyright 2008 Jeff Zaroyko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <mstcpip.h>
#include <stdio.h>
#include "wine/test.h"
#include <ndk/umtypes.h>

// ReactOS: Wine has this in mstcpip.h, but it doesn't belong there
#define WSA_CMSG_ALIGN(len)     (((len) + sizeof(SIZE_T) - 1) & ~(sizeof(SIZE_T) - 1))

#define MAX_CLIENTS 4      /* Max number of clients */
#define FIRST_CHAR 'A'     /* First character in transferred pattern */
#define BIND_SLEEP 10      /* seconds to wait between attempts to bind() */
#define BIND_TRIES 6       /* Number of bind() attempts */
#define TEST_TIMEOUT 30    /* seconds to wait before killing child threads
                              after server initialization, if something hangs */

#define NUM_UDP_PEERS 3    /* Number of UDP sockets to create and test > 1 */

#define NUM_THREADS 3      /* Number of threads to run getservbyname */
#define NUM_QUERIES 250    /* Number of getservbyname queries per thread */

#define SERVERIP "127.0.0.1"   /* IP to bind to */
#define SERVERPORT 9374        /* Port number to bind to */

#define wsa_ok(op, cond, msg) \
   do { \
        int tmp, err = 0; \
        tmp = op; \
        if ( !(cond tmp) ) err = WSAGetLastError(); \
        ok ( cond tmp, msg, GetCurrentThreadId(), err); \
   } while (0);

#define make_keepalive(k, enable, time, interval) \
   k.onoff = enable; \
   k.keepalivetime = time; \
   k.keepaliveinterval = interval;

/* Function pointers */
static void  (WINAPI *pfreeaddrinfo)(struct addrinfo *);
static int   (WINAPI *pgetaddrinfo)(LPCSTR,LPCSTR,const struct addrinfo *,struct addrinfo **);
static void  (WINAPI *pFreeAddrInfoW)(PADDRINFOW);
static int   (WINAPI *pGetAddrInfoW)(LPCWSTR,LPCWSTR,const ADDRINFOW *,PADDRINFOW *);
static PCSTR (WINAPI *pInetNtop)(INT,LPVOID,LPSTR,ULONG);

/**************** Structs and typedefs ***************/

typedef struct thread_info
{
    HANDLE thread;
    DWORD id;
} thread_info;

/* Information in the server about open client connections */
typedef struct sock_info
{
    SOCKET                 s;
    struct sockaddr_in     addr;
    struct sockaddr_in     peer;
    char                  *buf;
    int                    n_recvd;
    int                    n_sent;
} sock_info;

/* Test parameters for both server & client */
typedef struct test_params
{
    int          sock_type;
    int          sock_prot;
    const char  *inet_addr;
    short        inet_port;
    int          chunk_size;
    int          n_chunks;
    int          n_clients;
} test_params;

/* server-specific test parameters */
typedef struct server_params
{
    test_params   *general;
    DWORD          sock_flags;
    int            buflen;
} server_params;

/* client-specific test parameters */
typedef struct client_params
{
    test_params   *general;
    DWORD          sock_flags;
    int            buflen;
} client_params;

/* This type combines all information for setting up a test scenario */
typedef struct test_setup
{
    test_params              general;
    LPVOID                   srv;
    server_params            srv_params;
    LPVOID                   clt;
    client_params            clt_params;
} test_setup;

/* Thread local storage for server */
typedef struct server_memory
{
    SOCKET                  s;
    struct sockaddr_in      addr;
    sock_info               sock[MAX_CLIENTS];
} server_memory;

/* Thread local storage for client */
typedef struct client_memory
{
    SOCKET s;
    struct sockaddr_in      addr;
    char                   *send_buf;
    char                   *recv_buf;
} client_memory;

/* SelectReadThread thread parameters */
typedef struct select_thread_params
{
    SOCKET s;
    BOOL ReadKilled;
} select_thread_params;

/**************** Static variables ***************/

static DWORD      tls;              /* Thread local storage index */
static HANDLE     thread[1+MAX_CLIENTS];
static DWORD      thread_id[1+MAX_CLIENTS];
static HANDLE     server_ready;
static HANDLE     client_ready[MAX_CLIENTS];
static int        client_id;

/**************** General utility functions ***************/

static int tcp_socketpair(SOCKET *src, SOCKET *dst)
{
    SOCKET server = INVALID_SOCKET;
    struct sockaddr_in addr;
    int len;
    int ret;

    *src = INVALID_SOCKET;
    *dst = INVALID_SOCKET;

    *src = socket(AF_INET, SOCK_STREAM, 0);
    if (*src == INVALID_SOCKET)
        goto end;

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET)
        goto end;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    ret = bind(server, (struct sockaddr*)&addr, sizeof(addr));
    if (ret != 0)
        goto end;

    len = sizeof(addr);
    ret = getsockname(server, (struct sockaddr*)&addr, &len);
    if (ret != 0)
        goto end;

    ret = listen(server, 1);
    if (ret != 0)
        goto end;

    ret = connect(*src, (struct sockaddr*)&addr, sizeof(addr));
    if (ret != 0)
        goto end;

    len = sizeof(addr);
    *dst = accept(server, (struct sockaddr*)&addr, &len);

end:
    if (server != INVALID_SOCKET)
        closesocket(server);
    if (*src != INVALID_SOCKET && *dst != INVALID_SOCKET)
        return 0;
    closesocket(*src);
    closesocket(*dst);
    return -1;
}

static void set_so_opentype ( BOOL overlapped )
{
    int optval = !overlapped, newval, len = sizeof (int);

    ok ( setsockopt ( INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE,
                      (LPVOID) &optval, sizeof (optval) ) == 0,
         "setting SO_OPENTYPE failed\n" );
    ok ( getsockopt ( INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE,
                      (LPVOID) &newval, &len ) == 0,
         "getting SO_OPENTYPE failed\n" );
    ok ( optval == newval, "failed to set SO_OPENTYPE\n" );
}

static int set_blocking ( SOCKET s, BOOL blocking )
{
    u_long val = !blocking;
    return ioctlsocket ( s, FIONBIO, &val );
}

static void fill_buffer ( char *buf, int chunk_size, int n_chunks )
{
    char c, *p;
    for ( c = FIRST_CHAR, p = buf; c < FIRST_CHAR + n_chunks; c++, p += chunk_size )
        memset ( p, c, chunk_size );
}

static int test_buffer ( char *buf, int chunk_size, int n_chunks )
{
    char c, *p;
    int i;
    for ( c = FIRST_CHAR, p = buf; c < FIRST_CHAR + n_chunks; c++, p += chunk_size )
    {
        for ( i = 0; i < chunk_size; i++ )
            if ( p[i] != c ) return i;
    }
    return -1;
}

/*
 * This routine is called when a client / server does not expect any more data,
 * but needs to acknowledge the closing of the connection (by reading 0 bytes).
 */
static void read_zero_bytes ( SOCKET s )
{
    char buf[256];
    int tmp, n = 0;
    while ( ( tmp = recv ( s, buf, 256, 0 ) ) > 0 )
        n += tmp;
    ok ( n <= 0, "garbage data received: %d bytes\n", n );
}

static int do_oob_send ( SOCKET s, char *buf, int buflen, int sendlen )
{
    char* last = buf + buflen, *p;
    int n = 1;
    for ( p = buf; n > 0 && p < last; p += n )
        n = send ( s, p, min ( sendlen, last - p ), MSG_OOB );
    wsa_ok ( n, 0 <=, "do_oob_send (%x): error %d\n" );
    return p - buf;
}

static int do_synchronous_send ( SOCKET s, char *buf, int buflen, int sendlen )
{
    char* last = buf + buflen, *p;
    int n = 1;
    for ( p = buf; n > 0 && p < last; p += n )
        n = send ( s, p, min ( sendlen, last - p ), 0 );
    wsa_ok ( n, 0 <=, "do_synchronous_send (%x): error %d\n" );
    return p - buf;
}

static int do_synchronous_recv ( SOCKET s, char *buf, int buflen, int recvlen )
{
    char* last = buf + buflen, *p;
    int n = 1;
    for ( p = buf; n > 0 && p < last; p += n )
        n = recv ( s, p, min ( recvlen, last - p ), 0 );
    wsa_ok ( n, 0 <=, "do_synchronous_recv (%x): error %d:\n" );
    return p - buf;
}

static int do_synchronous_recvfrom ( SOCKET s, char *buf, int buflen,int flags,struct sockaddr *from, int *fromlen, int recvlen )
{
    char* last = buf + buflen, *p;
    int n = 1;
    for ( p = buf; n > 0 && p < last; p += n )
      n = recvfrom ( s, p, min ( recvlen, last - p ), 0, from, fromlen );
    wsa_ok ( n, 0 <=, "do_synchronous_recv (%x): error %d:\n" );
    return p - buf;
}

/*
 *  Call this routine right after thread startup.
 *  SO_OPENTYPE must by 0, regardless what the server did.
 */
static void check_so_opentype (void)
{
    int tmp = 1, len;
    len = sizeof (tmp);
    getsockopt ( INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE, (LPVOID) &tmp, &len );
    ok ( tmp == 0, "check_so_opentype: wrong startup value of SO_OPENTYPE: %d\n", tmp );
}

/**************** Server utility functions ***************/

/*
 *  Even if we have closed our server socket cleanly,
 *  the OS may mark the address "in use" for some time -
 *  this happens with native Linux apps, too.
 */
static void do_bind ( SOCKET s, struct sockaddr* addr, int addrlen )
{
    int err, wsaerr = 0, n_try = BIND_TRIES;

    while ( ( err = bind ( s, addr, addrlen ) ) != 0 &&
            ( wsaerr = WSAGetLastError () ) == WSAEADDRINUSE &&
            n_try-- >= 0)
    {
        trace ( "address in use, waiting ...\n" );
        Sleep ( 1000 * BIND_SLEEP );
    }
    ok ( err == 0, "failed to bind: %d\n", wsaerr );
}

static void server_start ( server_params *par )
{
    int i;
    test_params *gen = par->general;
    server_memory *mem = LocalAlloc ( LPTR, sizeof ( server_memory ) );

    TlsSetValue ( tls, mem );
    mem->s = WSASocketA ( AF_INET, gen->sock_type, gen->sock_prot,
                          NULL, 0, par->sock_flags );
    ok ( mem->s != INVALID_SOCKET, "Server: WSASocket failed\n" );

    mem->addr.sin_family = AF_INET;
    mem->addr.sin_addr.s_addr = inet_addr ( gen->inet_addr );
    mem->addr.sin_port = htons ( gen->inet_port );

    for (i = 0; i < MAX_CLIENTS; i++)
    {
        mem->sock[i].s = INVALID_SOCKET;
        mem->sock[i].buf = LocalAlloc ( LPTR, gen->n_chunks * gen->chunk_size );
        mem->sock[i].n_recvd = 0;
        mem->sock[i].n_sent = 0;
    }

    if ( gen->sock_type == SOCK_STREAM )
        do_bind ( mem->s, (struct sockaddr*) &mem->addr, sizeof (mem->addr) );
}

static void server_stop (void)
{
    int i;
    server_memory *mem = TlsGetValue ( tls );

    for (i = 0; i < MAX_CLIENTS; i++ )
    {
        LocalFree ( mem->sock[i].buf );
        if ( mem->sock[i].s != INVALID_SOCKET )
            closesocket ( mem->sock[i].s );
    }
    ok ( closesocket ( mem->s ) == 0, "closesocket failed\n" );
    LocalFree ( mem );
    ExitThread ( GetCurrentThreadId () );
}

/**************** Client utilitiy functions ***************/

static void client_start ( client_params *par )
{
    test_params *gen = par->general;
    client_memory *mem = LocalAlloc (LPTR, sizeof (client_memory));

    TlsSetValue ( tls, mem );

    WaitForSingleObject ( server_ready, INFINITE );

    mem->s = WSASocketA ( AF_INET, gen->sock_type, gen->sock_prot,
                          NULL, 0, par->sock_flags );

    mem->addr.sin_family = AF_INET;
    mem->addr.sin_addr.s_addr = inet_addr ( gen->inet_addr );
    mem->addr.sin_port = htons ( gen->inet_port );

    ok ( mem->s != INVALID_SOCKET, "Client: WSASocket failed\n" );

    mem->send_buf = LocalAlloc ( LPTR, 2 * gen->n_chunks * gen->chunk_size );
    mem->recv_buf = mem->send_buf + gen->n_chunks * gen->chunk_size;
    fill_buffer ( mem->send_buf, gen->chunk_size, gen->n_chunks );

    SetEvent ( client_ready[client_id] );
    /* Wait for the other clients to come up */
    WaitForMultipleObjects ( min ( gen->n_clients, MAX_CLIENTS ), client_ready, TRUE, INFINITE );
}

static void client_stop (void)
{
    client_memory *mem = TlsGetValue ( tls );
    wsa_ok ( closesocket ( mem->s ), 0 ==, "closesocket error (%x): %d\n" );
    LocalFree ( mem->send_buf );
    LocalFree ( mem );
    ExitThread(0);
}

/**************** Servers ***************/

/*
 * simple_server: A very basic server doing synchronous IO.
 */
static VOID WINAPI simple_server ( server_params *par )
{
    test_params *gen = par->general;
    server_memory *mem;
    int pos, n_recvd, n_sent, n_expected = gen->n_chunks * gen->chunk_size, tmp, i,
        id = GetCurrentThreadId();

    trace ( "simple_server (%x) starting\n", id );

    set_so_opentype ( FALSE ); /* non-overlapped */
    server_start ( par );
    mem = TlsGetValue ( tls );

    wsa_ok ( set_blocking ( mem->s, TRUE ), 0 ==, "simple_server (%x): failed to set blocking mode: %d\n");
    wsa_ok ( listen ( mem->s, SOMAXCONN ), 0 ==, "simple_server (%x): listen failed: %d\n");

    trace ( "simple_server (%x) ready\n", id );
    SetEvent ( server_ready ); /* notify clients */

    for ( i = 0; i < min ( gen->n_clients, MAX_CLIENTS ); i++ )
    {
        trace ( "simple_server (%x): waiting for client\n", id );

        /* accept a single connection */
        tmp = sizeof ( mem->sock[0].peer );
        mem->sock[0].s = accept ( mem->s, (struct sockaddr*) &mem->sock[0].peer, &tmp );
        wsa_ok ( mem->sock[0].s, INVALID_SOCKET !=, "simple_server (%x): accept failed: %d\n" );

        ok ( mem->sock[0].peer.sin_addr.s_addr == inet_addr ( gen->inet_addr ),
             "simple_server (%x): strange peer address\n", id );

        /* Receive data & check it */
        n_recvd = do_synchronous_recv ( mem->sock[0].s, mem->sock[0].buf, n_expected, par->buflen );
        ok ( n_recvd == n_expected,
             "simple_server (%x): received less data than expected: %d of %d\n", id, n_recvd, n_expected );
        pos = test_buffer ( mem->sock[0].buf, gen->chunk_size, gen->n_chunks );
        ok ( pos == -1, "simple_server (%x): test pattern error: %d\n", id, pos);

        /* Echo data back */
        n_sent = do_synchronous_send ( mem->sock[0].s, mem->sock[0].buf, n_expected, par->buflen );
        ok ( n_sent == n_expected,
             "simple_server (%x): sent less data than expected: %d of %d\n", id, n_sent, n_expected );

        /* cleanup */
        read_zero_bytes ( mem->sock[0].s );
        wsa_ok ( closesocket ( mem->sock[0].s ),  0 ==, "simple_server (%x): closesocket error: %d\n" );
        mem->sock[0].s = INVALID_SOCKET;
    }

    trace ( "simple_server (%x) exiting\n", id );
    server_stop ();
}

/*
 * oob_server: A very basic server receiving out-of-band data.
 */
static VOID WINAPI oob_server ( server_params *par )
{
    test_params *gen = par->general;
    server_memory *mem;
    u_long atmark = 0;
    int pos, n_recvd, n_expected = gen->n_chunks * gen->chunk_size, tmp,
        id = GetCurrentThreadId();

    trace ( "oob_server (%x) starting\n", id );

    set_so_opentype ( FALSE ); /* non-overlapped */
    server_start ( par );
    mem = TlsGetValue ( tls );

    wsa_ok ( set_blocking ( mem->s, TRUE ), 0 ==, "oob_server (%x): failed to set blocking mode: %d\n");
    wsa_ok ( listen ( mem->s, SOMAXCONN ), 0 ==, "oob_server (%x): listen failed: %d\n");

    trace ( "oob_server (%x) ready\n", id );
    SetEvent ( server_ready ); /* notify clients */

    trace ( "oob_server (%x): waiting for client\n", id );

    /* accept a single connection */
    tmp = sizeof ( mem->sock[0].peer );
    mem->sock[0].s = accept ( mem->s, (struct sockaddr*) &mem->sock[0].peer, &tmp );
    wsa_ok ( mem->sock[0].s, INVALID_SOCKET !=, "oob_server (%x): accept failed: %d\n" );

    ok ( mem->sock[0].peer.sin_addr.s_addr == inet_addr ( gen->inet_addr ),
         "oob_server (%x): strange peer address\n", id );

    /* check atmark state */
    ioctlsocket ( mem->sock[0].s, SIOCATMARK, &atmark );
    ok ( atmark == 1, "oob_server (%x): unexpectedly at the OOB mark: %i\n", id, atmark );

    /* Receive normal data and check atmark state */
    n_recvd = do_synchronous_recv ( mem->sock[0].s, mem->sock[0].buf, n_expected, par->buflen );
    ok ( n_recvd == n_expected,
         "simple_server (%x): received less data than expected: %d of %d\n", id, n_recvd, n_expected );
    pos = test_buffer ( mem->sock[0].buf, gen->chunk_size, gen->n_chunks );
    ok ( pos == -1, "simple_server (%x): test pattern error: %d\n", id, pos);

    ioctlsocket ( mem->sock[0].s, SIOCATMARK, &atmark );
    ok ( atmark == 1, "oob_server (%x): unexpectedly at the OOB mark: %i\n", id, atmark );

    /* Receive a part of the out-of-band data and check atmark state */
    n_recvd = do_synchronous_recv ( mem->sock[0].s, mem->sock[0].buf, 8, par->buflen );
    ok ( n_recvd == 8,
         "oob_server (%x): received less data than expected: %d of %d\n", id, n_recvd, 8 );
    n_expected -= 8;

    ioctlsocket ( mem->sock[0].s, SIOCATMARK, &atmark );
    todo_wine ok ( atmark == 0, "oob_server (%x): not at the OOB mark: %i\n", id, atmark );

    /* Receive the rest of the out-of-band data and check atmark state */
    do_synchronous_recv ( mem->sock[0].s, mem->sock[0].buf, n_expected, par->buflen );

    ioctlsocket ( mem->sock[0].s, SIOCATMARK, &atmark );
    todo_wine ok ( atmark == 0, "oob_server (%x): not at the OOB mark: %i\n", id, atmark );

    /* cleanup */
    wsa_ok ( closesocket ( mem->sock[0].s ),  0 ==, "oob_server (%x): closesocket error: %d\n" );
    mem->sock[0].s = INVALID_SOCKET;

    trace ( "oob_server (%x) exiting\n", id );
    server_stop ();
}

/*
 * select_server: A non-blocking server.
 */
static VOID WINAPI select_server ( server_params *par )
{
    test_params *gen = par->general;
    server_memory *mem;
    int n_expected = gen->n_chunks * gen->chunk_size, tmp, i,
        id = GetCurrentThreadId(), n_connections = 0, n_sent, n_recvd,
        n_set, delta, n_ready;
    struct timeval timeout = {0,10}; /* wait for 10 milliseconds */
    fd_set fds_recv, fds_send, fds_openrecv, fds_opensend;

    trace ( "select_server (%x) starting\n", id );

    set_so_opentype ( FALSE ); /* non-overlapped */
    server_start ( par );
    mem = TlsGetValue ( tls );

    wsa_ok ( set_blocking ( mem->s, FALSE ), 0 ==, "select_server (%x): failed to set blocking mode: %d\n");
    wsa_ok ( listen ( mem->s, SOMAXCONN ), 0 ==, "select_server (%x): listen failed: %d\n");

    trace ( "select_server (%x) ready\n", id );
    SetEvent ( server_ready ); /* notify clients */

    FD_ZERO ( &fds_openrecv );
    FD_ZERO ( &fds_recv );
    FD_ZERO ( &fds_send );
    FD_ZERO ( &fds_opensend );

    FD_SET ( mem->s, &fds_openrecv );

    while(1)
    {
        fds_recv = fds_openrecv;
        fds_send = fds_opensend;

        n_set = 0;

        wsa_ok ( ( n_ready = select ( 0, &fds_recv, &fds_send, NULL, &timeout ) ), SOCKET_ERROR !=, 
            "select_server (%x): select() failed: %d\n" );

        /* check for incoming requests */
        if ( FD_ISSET ( mem->s, &fds_recv ) ) {
            n_set += 1;

            trace ( "select_server (%x): accepting client connection\n", id );

            /* accept a single connection */
            tmp = sizeof ( mem->sock[n_connections].peer );
            mem->sock[n_connections].s = accept ( mem->s, (struct sockaddr*) &mem->sock[n_connections].peer, &tmp );
            wsa_ok ( mem->sock[n_connections].s, INVALID_SOCKET !=, "select_server (%x): accept() failed: %d\n" );

            ok ( mem->sock[n_connections].peer.sin_addr.s_addr == inet_addr ( gen->inet_addr ),
                "select_server (%x): strange peer address\n", id );

            /* add to list of open connections */
            FD_SET ( mem->sock[n_connections].s, &fds_openrecv );
            FD_SET ( mem->sock[n_connections].s, &fds_opensend );

            n_connections++;
        }

        /* handle open requests */

        for ( i = 0; i < n_connections; i++ )
        {
            if ( FD_ISSET( mem->sock[i].s, &fds_recv ) ) {
                n_set += 1;

                if ( mem->sock[i].n_recvd < n_expected ) {
                    /* Receive data & check it */
                    n_recvd = recv ( mem->sock[i].s, mem->sock[i].buf + mem->sock[i].n_recvd, min ( n_expected - mem->sock[i].n_recvd, par->buflen ), 0 );
                    ok ( n_recvd != SOCKET_ERROR, "select_server (%x): error in recv(): %d\n", id, WSAGetLastError() );
                    mem->sock[i].n_recvd += n_recvd;

                    if ( mem->sock[i].n_recvd == n_expected ) {
                        int pos = test_buffer ( mem->sock[i].buf, gen->chunk_size, gen->n_chunks );
                        ok ( pos == -1, "select_server (%x): test pattern error: %d\n", id, pos );
                        FD_CLR ( mem->sock[i].s, &fds_openrecv );
                    }

                    ok ( mem->sock[i].n_recvd <= n_expected, "select_server (%x): received too many bytes: %d\n", id, mem->sock[i].n_recvd );
                }
            }

            /* only echo back what we've received */
            delta = mem->sock[i].n_recvd - mem->sock[i].n_sent;

            if ( FD_ISSET ( mem->sock[i].s, &fds_send ) ) {
                n_set += 1;

                if ( ( delta > 0 ) && ( mem->sock[i].n_sent < n_expected ) ) {
                    /* Echo data back */
                    n_sent = send ( mem->sock[i].s, mem->sock[i].buf + mem->sock[i].n_sent, min ( delta, par->buflen ), 0 );
                    ok ( n_sent != SOCKET_ERROR, "select_server (%x): error in send(): %d\n", id, WSAGetLastError() );
                    mem->sock[i].n_sent += n_sent;

                    if ( mem->sock[i].n_sent == n_expected ) {
                        FD_CLR ( mem->sock[i].s, &fds_opensend );
                    }

                    ok ( mem->sock[i].n_sent <= n_expected, "select_server (%x): sent too many bytes: %d\n", id, mem->sock[i].n_sent );
                }
            }
        }

        /* check that select returned the correct number of ready sockets */
        ok ( ( n_set == n_ready ), "select_server (%x): select() returns wrong number of ready sockets\n", id );

        /* check if all clients are done */
        if ( ( fds_opensend.fd_count == 0 ) 
            && ( fds_openrecv.fd_count == 1 ) /* initial socket that accepts clients */
            && ( n_connections  == min ( gen->n_clients, MAX_CLIENTS ) ) ) {
            break;
        }
    }

    for ( i = 0; i < min ( gen->n_clients, MAX_CLIENTS ); i++ )
    {
        /* cleanup */
        read_zero_bytes ( mem->sock[i].s );
        wsa_ok ( closesocket ( mem->sock[i].s ),  0 ==, "select_server (%x): closesocket error: %d\n" );
        mem->sock[i].s = INVALID_SOCKET;
    }

    trace ( "select_server (%x) exiting\n", id );
    server_stop ();
}

/**************** Clients ***************/

/*
 * simple_client: A very basic client doing synchronous IO.
 */
static VOID WINAPI simple_client ( client_params *par )
{
    test_params *gen = par->general;
    client_memory *mem;
    int pos, n_sent, n_recvd, n_expected = gen->n_chunks * gen->chunk_size, id;

    id = GetCurrentThreadId();
    trace ( "simple_client (%x): starting\n", id );
    /* wait here because we want to call set_so_opentype before creating a socket */
    WaitForSingleObject ( server_ready, INFINITE );
    trace ( "simple_client (%x): server ready\n", id );

    check_so_opentype ();
    set_so_opentype ( FALSE ); /* non-overlapped */
    client_start ( par );
    mem = TlsGetValue ( tls );

    /* Connect */
    wsa_ok ( connect ( mem->s, (struct sockaddr*) &mem->addr, sizeof ( mem->addr ) ),
             0 ==, "simple_client (%x): connect error: %d\n" );
    ok ( set_blocking ( mem->s, TRUE ) == 0,
         "simple_client (%x): failed to set blocking mode\n", id );
    trace ( "simple_client (%x) connected\n", id );

    /* send data to server */
    n_sent = do_synchronous_send ( mem->s, mem->send_buf, n_expected, par->buflen );
    ok ( n_sent == n_expected,
         "simple_client (%x): sent less data than expected: %d of %d\n", id, n_sent, n_expected );

    /* shutdown send direction */
    wsa_ok ( shutdown ( mem->s, SD_SEND ), 0 ==, "simple_client (%x): shutdown failed: %d\n" );

    /* Receive data echoed back & check it */
    n_recvd = do_synchronous_recv ( mem->s, mem->recv_buf, n_expected, par->buflen );
    ok ( n_recvd == n_expected,
         "simple_client (%x): received less data than expected: %d of %d\n", id, n_recvd, n_expected );

    /* check data */
    pos = test_buffer ( mem->recv_buf, gen->chunk_size, gen->n_chunks );
    ok ( pos == -1, "simple_client (%x): test pattern error: %d\n", id, pos);

    /* cleanup */
    read_zero_bytes ( mem->s );
    trace ( "simple_client (%x) exiting\n", id );
    client_stop ();
}

/*
 * oob_client: A very basic client sending out-of-band data.
 */
static VOID WINAPI oob_client ( client_params *par )
{
    test_params *gen = par->general;
    client_memory *mem;
    int n_sent, n_expected = gen->n_chunks * gen->chunk_size, id;

    id = GetCurrentThreadId();
    trace ( "oob_client (%x): starting\n", id );
    /* wait here because we want to call set_so_opentype before creating a socket */
    WaitForSingleObject ( server_ready, INFINITE );
    trace ( "oob_client (%x): server ready\n", id );

    check_so_opentype ();
    set_so_opentype ( FALSE ); /* non-overlapped */
    client_start ( par );
    mem = TlsGetValue ( tls );

    /* Connect */
    wsa_ok ( connect ( mem->s, (struct sockaddr*) &mem->addr, sizeof ( mem->addr ) ),
             0 ==, "oob_client (%x): connect error: %d\n" );
    ok ( set_blocking ( mem->s, TRUE ) == 0,
         "oob_client (%x): failed to set blocking mode\n", id );
    trace ( "oob_client (%x) connected\n", id );

    /* send data to server */
    n_sent = do_synchronous_send ( mem->s, mem->send_buf, n_expected, par->buflen );
    ok ( n_sent == n_expected,
         "oob_client (%x): sent less data than expected: %d of %d\n", id, n_sent, n_expected );

    /* send out-of-band data to server */
    n_sent = do_oob_send ( mem->s, mem->send_buf, n_expected, par->buflen );
    ok ( n_sent == n_expected,
         "oob_client (%x): sent less data than expected: %d of %d\n", id, n_sent, n_expected );

    /* shutdown send direction */
    wsa_ok ( shutdown ( mem->s, SD_SEND ), 0 ==, "simple_client (%x): shutdown failed: %d\n" );

    /* cleanup */
    read_zero_bytes ( mem->s );
    trace ( "oob_client (%x) exiting\n", id );
    client_stop ();
}

/*
 * simple_mixed_client: mixing send and recvfrom
 */
static VOID WINAPI simple_mixed_client ( client_params *par )
{
    test_params *gen = par->general;
    client_memory *mem;
    int pos, n_sent, n_recvd, n_expected = gen->n_chunks * gen->chunk_size, id;
    int fromLen = sizeof(mem->addr);
    struct sockaddr test;

    id = GetCurrentThreadId();
    trace ( "simple_client (%x): starting\n", id );
    /* wait here because we want to call set_so_opentype before creating a socket */
    WaitForSingleObject ( server_ready, INFINITE );
    trace ( "simple_client (%x): server ready\n", id );

    check_so_opentype ();
    set_so_opentype ( FALSE ); /* non-overlapped */
    client_start ( par );
    mem = TlsGetValue ( tls );

    /* Connect */
    wsa_ok ( connect ( mem->s, (struct sockaddr*) &mem->addr, sizeof ( mem->addr ) ),
             0 ==, "simple_client (%x): connect error: %d\n" );
    ok ( set_blocking ( mem->s, TRUE ) == 0,
         "simple_client (%x): failed to set blocking mode\n", id );
    trace ( "simple_client (%x) connected\n", id );

    /* send data to server */
    n_sent = do_synchronous_send ( mem->s, mem->send_buf, n_expected, par->buflen );
    ok ( n_sent == n_expected,
         "simple_client (%x): sent less data than expected: %d of %d\n", id, n_sent, n_expected );

    /* shutdown send direction */
    wsa_ok ( shutdown ( mem->s, SD_SEND ), 0 ==, "simple_client (%x): shutdown failed: %d\n" );

    /* this shouldn't change, since lpFrom, is not updated on
       connection oriented sockets - exposed by bug 11640
    */
    ((struct sockaddr_in*)&test)->sin_addr.s_addr = inet_addr("0.0.0.0");

    /* Receive data echoed back & check it */
    n_recvd = do_synchronous_recvfrom ( mem->s,
					mem->recv_buf,
					n_expected,
					0,
					(struct sockaddr *)&test,
					&fromLen,
					par->buflen );
    ok ( n_recvd == n_expected,
         "simple_client (%x): received less data than expected: %d of %d\n", id, n_recvd, n_expected );

    /* check that lpFrom was not updated */
    ok(0 ==
       strcmp(
	      inet_ntoa(((struct sockaddr_in*)&test)->sin_addr),
	      "0.0.0.0"), "lpFrom shouldn't be updated on connection oriented sockets\n");

    /* check data */
    pos = test_buffer ( mem->recv_buf, gen->chunk_size, gen->n_chunks );
    ok ( pos == -1, "simple_client (%x): test pattern error: %d\n", id, pos);

    /* cleanup */
    read_zero_bytes ( mem->s );
    trace ( "simple_client (%x) exiting\n", id );
    client_stop ();
}

/*
 * event_client: An event-driven client
 */
static void WINAPI event_client ( client_params *par )
{
    test_params *gen = par->general;
    client_memory *mem;
    int id = GetCurrentThreadId(), n_expected = gen->n_chunks * gen->chunk_size,
        tmp, err, n;
    HANDLE event;
    WSANETWORKEVENTS wsa_events;
    char *send_last, *recv_last, *send_p, *recv_p;
    LONG mask = FD_READ | FD_WRITE | FD_CLOSE;

    trace ( "event_client (%x): starting\n", id );
    client_start ( par );
    trace ( "event_client (%x): server ready\n", id );

    mem = TlsGetValue ( tls );

    /* Prepare event notification for connect, makes socket nonblocking */
    event = WSACreateEvent ();
    WSAEventSelect ( mem->s, event, FD_CONNECT );
    tmp = connect ( mem->s, (struct sockaddr*) &mem->addr, sizeof ( mem->addr ) );
    if ( tmp != 0 ) {
        err = WSAGetLastError ();
        ok ( err == WSAEWOULDBLOCK, "event_client (%x): connect error: %d\n", id, err );
        tmp = WaitForSingleObject ( event, INFINITE );
        ok ( tmp == WAIT_OBJECT_0, "event_client (%x): wait for connect event failed: %d\n", id, tmp );
        err = WSAEnumNetworkEvents ( mem->s, event, &wsa_events );
        ok ( err == 0, "event_client (%x): WSAEnumNetworkEvents error: %d\n", id, err );
        err = wsa_events.iErrorCode[ FD_CONNECT_BIT ];
        ok ( err == 0, "event_client (%x): connect error: %d\n", id, err );
        if ( err ) goto out;
    }

    trace ( "event_client (%x) connected\n", id );

    WSAEventSelect ( mem->s, event, mask );

    recv_p = mem->recv_buf;
    recv_last = mem->recv_buf + n_expected;
    send_p = mem->send_buf;
    send_last = mem->send_buf + n_expected;

    while ( TRUE )
    {
        err = WaitForSingleObject ( event, INFINITE );
        ok ( err == WAIT_OBJECT_0, "event_client (%x): wait failed\n", id );

        err = WSAEnumNetworkEvents ( mem->s, event, &wsa_events );
        ok( err == 0, "event_client (%x): WSAEnumNetworkEvents error: %d\n", id, err );

        if ( wsa_events.lNetworkEvents & FD_WRITE )
        {
            err = wsa_events.iErrorCode[ FD_WRITE_BIT ];
            ok ( err == 0, "event_client (%x): FD_WRITE error code: %d\n", id, err );

            if ( err== 0 )
                do
                {
                    n = send ( mem->s, send_p, min ( send_last - send_p, par->buflen ), 0 );
                    if ( n < 0 )
                    {
                        err = WSAGetLastError ();
                        ok ( err == WSAEWOULDBLOCK, "event_client (%x): send error: %d\n", id, err );
                    }
                    else
                        send_p += n;
                }
                while ( n >= 0 && send_p < send_last );

            if ( send_p == send_last )
            {
                trace ( "event_client (%x): all data sent - shutdown\n", id );
                shutdown ( mem->s, SD_SEND );
                mask &= ~FD_WRITE;
                WSAEventSelect ( mem->s, event, mask );
            }
        }
        if ( wsa_events.lNetworkEvents & FD_READ )
        {
            err = wsa_events.iErrorCode[ FD_READ_BIT ];
            ok ( err == 0, "event_client (%x): FD_READ error code: %d\n", id, err );
            if ( err != 0 ) break;
            
            /* First read must succeed */
            n = recv ( mem->s, recv_p, min ( recv_last - recv_p, par->buflen ), 0 );
            wsa_ok ( n, 0 <=, "event_client (%x): recv error: %d\n" );

            while ( n >= 0 ) {
                recv_p += n;
                if ( recv_p == recv_last )
                {
                    mask &= ~FD_READ;
                    trace ( "event_client (%x): all data received\n", id );
                    WSAEventSelect ( mem->s, event, mask );
                    break;
                }
                n = recv ( mem->s, recv_p, min ( recv_last - recv_p, par->buflen ), 0 );
                if ( n < 0 && ( err = WSAGetLastError()) != WSAEWOULDBLOCK )
                    ok ( 0, "event_client (%x): read error: %d\n", id, err );
                
            }
        }   
        if ( wsa_events.lNetworkEvents & FD_CLOSE )
        {
            trace ( "event_client (%x): close event\n", id );
            err = wsa_events.iErrorCode[ FD_CLOSE_BIT ];
            ok ( err == 0, "event_client (%x): FD_CLOSE error code: %d\n", id, err );
            break;
        }
    }

    n = send_p - mem->send_buf;
    ok ( send_p == send_last,
         "simple_client (%x): sent less data than expected: %d of %d\n", id, n, n_expected );
    n = recv_p - mem->recv_buf;
    ok ( recv_p == recv_last,
         "simple_client (%x): received less data than expected: %d of %d\n", id, n, n_expected );
    n = test_buffer ( mem->recv_buf, gen->chunk_size, gen->n_chunks );
    ok ( n == -1, "event_client (%x): test pattern error: %d\n", id, n);

out:
    WSACloseEvent ( event );
    trace ( "event_client (%x) exiting\n", id );
    client_stop ();
}

/* Tests for WSAStartup */
static void test_WithoutWSAStartup(void)
{
    LPVOID ptr;

    WSASetLastError(0xdeadbeef);
    ptr = gethostbyname("localhost");

    ok(ptr == NULL, "gethostbyname() succeeded unexpectedly: %d\n", WSAGetLastError());
    ok(WSAGetLastError() == WSANOTINITIALISED, "gethostbyname() failed with unexpected error: %d\n",
                WSAGetLastError());
}

static void test_WithWSAStartup(void)
{
    WSADATA data;
    WORD version = MAKEWORD( 2, 2 );
    INT res;
    LPVOID ptr;

    res = WSAStartup( version, &data );
    ok(res == 0, "WSAStartup() failed unexpectedly: %d\n", res);

    ptr = gethostbyname("localhost");
    ok(ptr != NULL, "gethostbyname() failed unexpectedly: %d\n", WSAGetLastError());

    WSACleanup();
}

/**************** Main program utility functions ***************/

static void Init (void)
{
    WORD ver = MAKEWORD (2, 2);
    WSADATA data;
    HMODULE hws2_32 = GetModuleHandle("ws2_32.dll");

    pfreeaddrinfo = (void *)GetProcAddress(hws2_32, "freeaddrinfo");
    pgetaddrinfo = (void *)GetProcAddress(hws2_32, "getaddrinfo");
    pFreeAddrInfoW = (void *)GetProcAddress(hws2_32, "FreeAddrInfoW");
    pGetAddrInfoW = (void *)GetProcAddress(hws2_32, "GetAddrInfoW");
    pInetNtop = (void *)GetProcAddress(hws2_32, "inet_ntop");

    ok ( WSAStartup ( ver, &data ) == 0, "WSAStartup failed\n" );
    tls = TlsAlloc();
}

static void Exit (void)
{
    INT ret, err;
    TlsFree ( tls );
    ret = WSACleanup();
    err = WSAGetLastError();
    ok ( ret == 0, "WSACleanup failed ret = %d GetLastError is %d\n", ret, err);
    ret = WSACleanup();
    err = WSAGetLastError();
    ok ( (ret == SOCKET_ERROR && err ==  WSANOTINITIALISED) ||
         broken(ret == 0),  /* WinME */
            "WSACleanup returned %d GetLastError is %d\n", ret, err);
}

static void StartServer (LPTHREAD_START_ROUTINE routine,
                         test_params *general, server_params *par)
{
    par->general = general;
    thread[0] = CreateThread ( NULL, 0, routine, par, 0, &thread_id[0] );
    ok ( thread[0] != NULL, "Failed to create server thread\n" );
}

static void StartClients (LPTHREAD_START_ROUTINE routine,
                          test_params *general, client_params *par)
{
    int i;
    par->general = general;
    for ( i = 1; i <= min ( general->n_clients, MAX_CLIENTS ); i++ )
    {
        client_id = i - 1;
        thread[i] = CreateThread ( NULL, 0, routine, par, 0, &thread_id[i] );
        ok ( thread[i] != NULL, "Failed to create client thread\n" );
        /* Make sure the client is up and running */
        WaitForSingleObject ( client_ready[client_id], INFINITE );
    };
}

static void do_test( test_setup *test )
{
    DWORD i, n = min (test->general.n_clients, MAX_CLIENTS);
    DWORD wait;

    server_ready = CreateEventA ( NULL, TRUE, FALSE, NULL );
    for (i = 0; i <= n; i++)
        client_ready[i] = CreateEventA ( NULL, TRUE, FALSE, NULL );

    StartServer ( test->srv, &test->general, &test->srv_params );
    StartClients ( test->clt, &test->general, &test->clt_params );
    WaitForSingleObject ( server_ready, INFINITE );

    wait = WaitForMultipleObjects ( 1 + n, thread, TRUE, 1000 * TEST_TIMEOUT );
    ok ( wait <= WAIT_OBJECT_0 + n ,
         "some threads have not completed: %x\n", wait );

    if ( ! ( wait <= WAIT_OBJECT_0 + n ) )
    {
        for (i = 0; i <= n; i++)
        {
            if ( WaitForSingleObject ( thread[i], 0 ) != WAIT_OBJECT_0 )
            {
                trace ("terminating thread %08x\n", thread_id[i]);
                TerminateThread ( thread [i], 0 );
            }
        }
    }
    CloseHandle ( server_ready );
    for (i = 0; i <= n; i++)
        CloseHandle ( client_ready[i] );
}

/********* some tests for getsockopt(setsockopt(X)) == X ***********/
/* optname = SO_LINGER */
static const LINGER linger_testvals[] = {
    {0,0},
    {0,73}, 
    {1,0},
    {5,189}
};

/* optname = SO_RCVTIMEO, SOSNDTIMEO */
#define SOCKTIMEOUT1 63000 /* 63 seconds. Do not test fractional part because of a
                        bug in the linux kernel (fixed in 2.6.8) */ 
#define SOCKTIMEOUT2 997000 /* 997 seconds */

static void test_set_getsockopt(void)
{
    SOCKET s;
    int i, err, lasterr;
    int timeout;
    LINGER lingval;
    int size;

    s = socket(AF_INET, SOCK_STREAM, 0);
    ok(s!=INVALID_SOCKET, "socket() failed error: %d\n", WSAGetLastError());
    if( s == INVALID_SOCKET) return;
    /* SO_RCVTIMEO */
    timeout = SOCKTIMEOUT1;
    size = sizeof(timeout);
    err = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, size); 
    if( !err)
        err = getsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, &size); 
    ok( !err, "get/setsockopt(SO_RCVTIMEO) failed error: %d\n", WSAGetLastError());
    ok( timeout == SOCKTIMEOUT1, "getsockopt(SO_RCVTIMEO) returned wrong value %d\n", timeout);

    timeout = 0;
    size = sizeof(timeout);
    err = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, size);
    if( !err)
        err = getsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, &size);
    ok( !err, "get/setsockopt(SO_RCVTIMEO) failed error: %d\n", WSAGetLastError());
    ok( timeout == 0, "getsockopt(SO_RCVTIMEO) returned wrong value %d\n", timeout);

    /* SO_SNDTIMEO */
    timeout = SOCKTIMEOUT2; /* 997 seconds. See remark above */
    size = sizeof(timeout);
    err = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, size); 
    if( !err)
        err = getsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, &size); 
    ok( !err, "get/setsockopt(SO_SNDTIMEO) failed error: %d\n", WSAGetLastError());
    ok( timeout == SOCKTIMEOUT2, "getsockopt(SO_SNDTIMEO) returned wrong value %d\n", timeout);
    /* SO_LINGER */
    for( i = 0; i < sizeof(linger_testvals)/sizeof(LINGER);i++) {
        size =  sizeof(lingval);
        lingval = linger_testvals[i];
        err = setsockopt(s, SOL_SOCKET, SO_LINGER, (char *) &lingval, size); 
        if( !err)
            err = getsockopt(s, SOL_SOCKET, SO_LINGER, (char *) &lingval, &size); 
        ok( !err, "get/setsockopt(SO_LINGER) failed error: %d\n", WSAGetLastError());
        ok( !lingval.l_onoff == !linger_testvals[i].l_onoff &&
                (lingval.l_linger == linger_testvals[i].l_linger ||
                 (!lingval.l_linger && !linger_testvals[i].l_onoff))
                , "getsockopt(SO_LINGER #%d) returned wrong value %d,%d not %d,%d\n", i, 
                 lingval.l_onoff, lingval.l_linger,
                 linger_testvals[i].l_onoff, linger_testvals[i].l_linger);
    }

    size =  sizeof(lingval);
    err = setsockopt(s, SOL_SOCKET, SO_LINGER, NULL, size);
    ok(err == SOCKET_ERROR && WSAGetLastError() == WSAEFAULT,
       "got %d with %d (expected SOCKET_ERROR with WSAEFAULT)\n", err, WSAGetLastError());
    err = setsockopt(s, SOL_SOCKET, SO_LINGER, NULL, 0);
    ok(err == SOCKET_ERROR && WSAGetLastError() == WSAEFAULT,
       "got %d with %d (expected SOCKET_ERROR with WSAEFAULT)\n", err, WSAGetLastError());

    size =  sizeof(BOOL);
    err = setsockopt(s, SOL_SOCKET, SO_DONTLINGER, NULL, size);
    ok(err == SOCKET_ERROR && WSAGetLastError() == WSAEFAULT,
       "got %d with %d (expected SOCKET_ERROR with WSAEFAULT)\n", err, WSAGetLastError());
    err = setsockopt(s, SOL_SOCKET, SO_DONTLINGER, NULL, 0);
    ok(err == SOCKET_ERROR && WSAGetLastError() == WSAEFAULT,
       "got %d with %d (expected SOCKET_ERROR with WSAEFAULT)\n", err, WSAGetLastError());

    /* Test for erroneously passing a value instead of a pointer as optval */
    size = sizeof(char);
    err = setsockopt(s, SOL_SOCKET, SO_DONTROUTE, (char *)1, size);
    ok(err == SOCKET_ERROR, "setsockopt with optval being a value passed "
                            "instead of failing.\n");
    lasterr = WSAGetLastError();
    ok(lasterr == WSAEFAULT, "setsockopt with optval being a value "
                             "returned 0x%08x, not WSAEFAULT(0x%08x)\n",
                             lasterr, WSAEFAULT);

    /* SO_RCVTIMEO with invalid values for level */
    size = sizeof(timeout);
    timeout = SOCKTIMEOUT1;
    SetLastError(0xdeadbeef);
    err = setsockopt(s, 0xffffffff, SO_RCVTIMEO, (char *) &timeout, size);
    ok( (err == SOCKET_ERROR) && (WSAGetLastError() == WSAEINVAL),
        "got %d with %d (expected SOCKET_ERROR with WSAEINVAL\n",
        err, WSAGetLastError());

    timeout = SOCKTIMEOUT1;
    SetLastError(0xdeadbeef);
    err = setsockopt(s, 0x00008000, SO_RCVTIMEO, (char *) &timeout, size);
    ok( (err == SOCKET_ERROR) && (WSAGetLastError() == WSAEINVAL),
        "got %d with %d (expected SOCKET_ERROR with WSAEINVAL\n",
        err, WSAGetLastError());

    closesocket(s);
}

static void test_so_reuseaddr(void)
{
    struct sockaddr_in saddr;
    SOCKET s1,s2;
    unsigned int rc,reuse;
    int size;

    saddr.sin_family      = AF_INET;
    saddr.sin_port        = htons(9375);
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    s1=socket(AF_INET, SOCK_STREAM, 0);
    ok(s1!=INVALID_SOCKET, "socket() failed error: %d\n", WSAGetLastError());
    rc = bind(s1, (struct sockaddr*)&saddr, sizeof(saddr));
    ok(rc!=SOCKET_ERROR, "bind(s1) failed error: %d\n", WSAGetLastError());

    s2=socket(AF_INET, SOCK_STREAM, 0);
    ok(s2!=INVALID_SOCKET, "socket() failed error: %d\n", WSAGetLastError());

    reuse=0x1234;
    size=sizeof(reuse);
    rc=getsockopt(s2, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, &size );
    ok(rc==0 && reuse==0,"wrong result in getsockopt(SO_REUSEADDR): rc=%d reuse=%d\n",rc,reuse);

    rc = bind(s2, (struct sockaddr*)&saddr, sizeof(saddr));
    ok(rc==SOCKET_ERROR, "bind() succeeded\n");

    reuse = 1;
    rc = setsockopt(s2, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
    ok(rc==0, "setsockopt() failed error: %d\n", WSAGetLastError());

    /* On Win2k3 and above, all SO_REUSEADDR seems to do is to allow binding to
     * a port immediately after closing another socket on that port, so
     * basically following the BSD socket semantics here. */
    closesocket(s1);
    rc = bind(s2, (struct sockaddr*)&saddr, sizeof(saddr));
    ok(rc==0, "bind() failed error: %d\n", WSAGetLastError());

    closesocket(s2);
}

#define IP_PKTINFO_LEN (sizeof(WSACMSGHDR) + WSA_CMSG_ALIGN(sizeof(struct in_pktinfo)))

static void test_ip_pktinfo(void)
{
    ULONG addresses[2] = {inet_addr("127.0.0.1"), htonl(INADDR_ANY)};
    char recvbuf[10], pktbuf[512], msg[] = "HELLO";
    struct sockaddr_in s1addr, s2addr, s3addr;
    GUID WSARecvMsg_GUID = WSAID_WSARECVMSG;
    LPFN_WSARECVMSG pWSARecvMsg = NULL;
    unsigned int rc, foundhdr, yes = 1;
    DWORD dwBytes, dwSize, dwFlags;
    socklen_t addrlen;
    WSACMSGHDR *cmsg;
    WSAOVERLAPPED ov;
    WSABUF iovec[1];
    SOCKET s1, s2;
    WSAMSG hdr;
    int i, err;

    memset(&ov, 0, sizeof(ov));
    ov.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (ov.hEvent == INVALID_HANDLE_VALUE)
    {
        skip("Could not create event object, some tests will be skipped. errno = %d\n", GetLastError());
        return;
    }

    memset(&hdr, 0x00, sizeof(hdr));
    s1addr.sin_family = AF_INET;
    s1addr.sin_port   = htons(0);
    /* Note: s1addr.sin_addr is set below */
    iovec[0].buf      = recvbuf;
    iovec[0].len      = sizeof(recvbuf);
    hdr.name          = (struct sockaddr*)&s3addr;
    hdr.namelen       = sizeof(s3addr);
    hdr.lpBuffers     = &iovec[0];
    hdr.dwBufferCount = 1;
    hdr.Control.buf   = pktbuf;
    /* Note: hdr.Control.len is set below */
    hdr.dwFlags       = 0;

    for (i=0;i<sizeof(addresses)/sizeof(UINT32);i++)
    {
        s1addr.sin_addr.s_addr = addresses[i];

        /* Build "server" side socket */
        s1=socket(AF_INET, SOCK_DGRAM, 0);
        if (s1 == INVALID_SOCKET)
        {
            skip("socket() failed error, some tests skipped: %d\n", WSAGetLastError());
            goto cleanup;
        }

        /* Obtain the WSARecvMsg function */
        WSAIoctl(s1, SIO_GET_EXTENSION_FUNCTION_POINTER, &WSARecvMsg_GUID, sizeof(WSARecvMsg_GUID),
                 &pWSARecvMsg, sizeof(pWSARecvMsg), &dwBytes, NULL, NULL);
        if (!pWSARecvMsg)
        {
            win_skip("WSARecvMsg is unsupported, some tests will be skipped.\n");
            closesocket(s1);
            goto cleanup;
        }

        /* Setup the server side socket */
        rc=bind(s1, (struct sockaddr*)&s1addr, sizeof(s1addr));
        ok(rc != SOCKET_ERROR, "bind() failed error: %d\n", WSAGetLastError());
        rc=setsockopt(s1, IPPROTO_IP, IP_PKTINFO, (const char*)&yes, sizeof(yes));
        ok(rc == 0, "failed to set IPPROTO_IP flag IP_PKTINFO!\n");

        /* Build "client" side socket */
        addrlen = sizeof(s2addr);
        if (getsockname(s1, (struct sockaddr *) &s2addr, &addrlen) != 0)
        {
            skip("Failed to call getsockname, some tests skipped: %d\n", WSAGetLastError());
            closesocket(s1);
            goto cleanup;
        }
        s2addr.sin_addr.s_addr = addresses[0]; /* Always target the local adapter address */
        s2=socket(AF_INET, SOCK_DGRAM, 0);
        if (s2 == INVALID_SOCKET)
        {
            skip("socket() failed error, some tests skipped: %d\n", WSAGetLastError());
            closesocket(s1);
            goto cleanup;
        }

        /* Test an empty message header */
        rc=pWSARecvMsg(s1, NULL, NULL, NULL, NULL);
        err=WSAGetLastError();
        ok(rc == SOCKET_ERROR && err == WSAEFAULT, "WSARecvMsg() failed error: %d (ret = %d)\n", err, rc);

        /*
         * Send a packet from the client to the server and test for specifying
         * a short control header.
         */
        rc=sendto(s2, msg, sizeof(msg), 0, (struct sockaddr*)&s2addr, sizeof(s2addr));
        ok(rc == sizeof(msg), "sendto() failed error: %d\n", WSAGetLastError());
        hdr.Control.len = 1;
        rc=pWSARecvMsg(s1, &hdr, &dwSize, NULL, NULL);
        err=WSAGetLastError();
        ok(rc == SOCKET_ERROR && err == WSAEMSGSIZE && (hdr.dwFlags & MSG_CTRUNC),
           "WSARecvMsg() failed error: %d (ret: %d, flags: %d)\n", err, rc, hdr.dwFlags);
        hdr.dwFlags = 0; /* Reset flags */

        /* Perform another short control header test, this time with an overlapped receive */
        hdr.Control.len = 1;
        rc=pWSARecvMsg(s1, &hdr, NULL, &ov, NULL);
        err=WSAGetLastError();
        ok(rc != 0 && err == WSA_IO_PENDING, "WSARecvMsg() failed error: %d\n", err);
        rc=sendto(s2, msg, sizeof(msg), 0, (struct sockaddr*)&s2addr, sizeof(s2addr));
        ok(rc == sizeof(msg), "sendto() failed error: %d\n", WSAGetLastError());
        if (WaitForSingleObject(ov.hEvent, 100) != WAIT_OBJECT_0)
        {
            skip("Server side did not receive packet, some tests skipped.\n");
            closesocket(s2);
            closesocket(s1);
            continue;
        }
        dwFlags = 0;
        WSAGetOverlappedResult(s1, &ov, NULL, FALSE, &dwFlags);
        ok(dwFlags == 0,
           "WSAGetOverlappedResult() returned unexpected flags %d!\n", dwFlags);
        ok(hdr.dwFlags == MSG_CTRUNC,
           "WSARecvMsg() overlapped operation set unexpected flags %d.\n", hdr.dwFlags);
        hdr.dwFlags = 0; /* Reset flags */

        /*
         * Setup an overlapped receive, send a packet, then wait for the packet to be retrieved
         * on the server end and check that the returned packet matches what was sent.
         */
        hdr.Control.len = sizeof(pktbuf);
        rc=pWSARecvMsg(s1, &hdr, NULL, &ov, NULL);
        err=WSAGetLastError();
        ok(rc != 0 && err == WSA_IO_PENDING, "WSARecvMsg() failed error: %d\n", err);
        ok(hdr.Control.len == sizeof(pktbuf),
           "WSARecvMsg() control length mismatch (%d != sizeof pktbuf).\n", hdr.Control.len);
        rc=sendto(s2, msg, sizeof(msg), 0, (struct sockaddr*)&s2addr, sizeof(s2addr));
        ok(rc == sizeof(msg), "sendto() failed error: %d\n", WSAGetLastError());
        if (WaitForSingleObject(ov.hEvent, 100) != WAIT_OBJECT_0)
        {
            skip("Server side did not receive packet, some tests skipped.\n");
            closesocket(s2);
            closesocket(s1);
            continue;
        }
        dwSize = 0;
        WSAGetOverlappedResult(s1, &ov, &dwSize, FALSE, NULL);
        ok(dwSize == sizeof(msg),
           "WSARecvMsg() buffer length does not match transmitted data!\n");
        ok(strncmp(iovec[0].buf, msg, sizeof(msg)) == 0,
           "WSARecvMsg() buffer does not match transmitted data!\n");
        ok(hdr.Control.len == IP_PKTINFO_LEN,
           "WSARecvMsg() control length mismatch (%d).\n", hdr.Control.len);

        /* Test for the expected IP_PKTINFO return information. */
        foundhdr = FALSE;
        for (cmsg = WSA_CMSG_FIRSTHDR(&hdr); cmsg != NULL; cmsg = WSA_CMSG_NXTHDR(&hdr, cmsg))
        {
            if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO)
            {
                struct in_pktinfo *pi = (struct in_pktinfo *)WSA_CMSG_DATA(cmsg);

                ok(pi->ipi_addr.s_addr == s2addr.sin_addr.s_addr, "destination ip mismatch!\n");
                foundhdr = TRUE;
            }
        }
        ok(foundhdr, "IP_PKTINFO header information was not returned!\n");

        closesocket(s2);
        closesocket(s1);
    }

cleanup:
    CloseHandle(ov.hEvent);
}

/************* Array containing the tests to run **********/

#define STD_STREAM_SOCKET \
            SOCK_STREAM, \
            0, \
            SERVERIP, \
            SERVERPORT

static test_setup tests [] =
{
    /* Test 0: synchronous client and server */
    {
        {
            STD_STREAM_SOCKET,
            2048,
            16,
            2
        },
        simple_server,
        {
            NULL,
            0,
            64
        },
        simple_client,
        {
            NULL,
            0,
            128
        }
    },
    /* Test 1: event-driven client, synchronous server */
    {
        {
            STD_STREAM_SOCKET,
            2048,
            16,
            2
        },
        simple_server,
        {
            NULL,
            0,
            64
        },
        event_client,
        {
            NULL,
            WSA_FLAG_OVERLAPPED,
            128
        }
    },
    /* Test 2: synchronous client, non-blocking server via select() */
    {
        {
            STD_STREAM_SOCKET,
            2048,
            16,
            2
        },
        select_server,
        {
            NULL,
            0,
            64
        },
        simple_client,
        {
            NULL,
            0,
            128
        }
    },
    /* Test 3: OOB client, OOB server */
    {
        {
            STD_STREAM_SOCKET,
            128,
            16,
            1
        },
        oob_server,
        {
            NULL,
            0,
            128
        },
        oob_client,
        {
            NULL,
            0,
            128
        }
    },
    /* Test 4: synchronous mixed client and server */
    {
        {
            STD_STREAM_SOCKET,
            2048,
            16,
            2
        },
        simple_server,
        {
            NULL,
            0,
            64
        },
        simple_mixed_client,
        {
            NULL,
            0,
            128
        }
    }
};

static void test_UDP(void)
{
    /* This function tests UDP sendto() and recvfrom(). UDP is unreliable, so it is
       possible that this test fails due to dropped packets. */

    /* peer 0 receives data from all other peers */
    struct sock_info peer[NUM_UDP_PEERS];
    char buf[16];
    int ss, i, n_recv, n_sent;

    memset (buf,0,sizeof(buf));
    for ( i = NUM_UDP_PEERS - 1; i >= 0; i-- ) {
        ok ( ( peer[i].s = socket ( AF_INET, SOCK_DGRAM, 0 ) ) != INVALID_SOCKET, "UDP: socket failed\n" );

        peer[i].addr.sin_family         = AF_INET;
        peer[i].addr.sin_addr.s_addr    = inet_addr ( SERVERIP );

        if ( i == 0 ) {
            peer[i].addr.sin_port       = htons ( SERVERPORT );
        } else {
            peer[i].addr.sin_port       = htons ( 0 );
        }

        do_bind ( peer[i].s, (struct sockaddr *) &peer[i].addr, sizeof( peer[i].addr ) );

        /* test getsockname() to get peer's port */
        ss = sizeof ( peer[i].addr );
        ok ( getsockname ( peer[i].s, (struct sockaddr *) &peer[i].addr, &ss ) != SOCKET_ERROR, "UDP: could not getsockname()\n" );
        ok ( peer[i].addr.sin_port != htons ( 0 ), "UDP: bind() did not associate port\n" );
    }

    /* test getsockname() */
    ok ( peer[0].addr.sin_port == htons ( SERVERPORT ), "UDP: getsockname returned incorrect peer port\n" );

    for ( i = 1; i < NUM_UDP_PEERS; i++ ) {
        /* send client's ip */
        memcpy( buf, &peer[i].addr.sin_port, sizeof(peer[i].addr.sin_port) );
        n_sent = sendto ( peer[i].s, buf, sizeof(buf), 0, (struct sockaddr*) &peer[0].addr, sizeof(peer[0].addr) );
        ok ( n_sent == sizeof(buf), "UDP: sendto() sent wrong amount of data or socket error: %d\n", n_sent );
    }

    for ( i = 1; i < NUM_UDP_PEERS; i++ ) {
        n_recv = recvfrom ( peer[0].s, buf, sizeof(buf), 0,(struct sockaddr *) &peer[0].peer, &ss );
        ok ( n_recv == sizeof(buf), "UDP: recvfrom() received wrong amount of data or socket error: %d\n", n_recv );
        ok ( memcmp ( &peer[0].peer.sin_port, buf, sizeof(peer[0].addr.sin_port) ) == 0, "UDP: port numbers do not match\n" );
    }
}

static DWORD WINAPI do_getservbyname( void *param )
{
    struct {
        const char *name;
        const char *proto;
        int port;
    } serv[2] = { {"domain", "udp", 53}, {"telnet", "tcp", 23} };

    HANDLE *starttest = param;
    int i, j;
    struct servent *pserv[2];

    ok ( WaitForSingleObject ( *starttest, TEST_TIMEOUT * 1000 ) != WAIT_TIMEOUT, "test_getservbyname: timeout waiting for start signal\n");

    /* ensure that necessary buffer resizes are completed */
    for ( j = 0; j < 2; j++) {
        pserv[j] = getservbyname ( serv[j].name, serv[j].proto );
    }

    for ( i = 0; i < NUM_QUERIES / 2; i++ ) {
        for ( j = 0; j < 2; j++ ) {
            pserv[j] = getservbyname ( serv[j].name, serv[j].proto );
            ok ( pserv[j] != NULL, "getservbyname could not retrieve information for %s: %d\n", serv[j].name, WSAGetLastError() );
            if ( !pserv[j] ) continue;
            ok ( pserv[j]->s_port == htons(serv[j].port), "getservbyname returned the wrong port for %s: %d\n", serv[j].name, ntohs(pserv[j]->s_port) );
            ok ( !strcmp ( pserv[j]->s_proto, serv[j].proto ), "getservbyname returned the wrong protocol for %s: %s\n", serv[j].name, pserv[j]->s_proto );
            ok ( !strcmp ( pserv[j]->s_name, serv[j].name ), "getservbyname returned the wrong name for %s: %s\n", serv[j].name, pserv[j]->s_name );
        }

        ok ( pserv[0] == pserv[1], "getservbyname: winsock resized servent buffer when not necessary\n" );
    }

    return 0;
}

static void test_getservbyname(void)
{
    int i;
    HANDLE starttest, thread[NUM_THREADS];
    DWORD thread_id[NUM_THREADS];

    starttest = CreateEvent ( NULL, 1, 0, "test_getservbyname_starttest" );

    /* create threads */
    for ( i = 0; i < NUM_THREADS; i++ ) {
        thread[i] = CreateThread ( NULL, 0, do_getservbyname, &starttest, 0, &thread_id[i] );
    }

    /* signal threads to start */
    SetEvent ( starttest );

    for ( i = 0; i < NUM_THREADS; i++) {
        WaitForSingleObject ( thread[i], TEST_TIMEOUT * 1000 );
    }
}

static void test_WSASocket(void)
{
    SOCKET sock = INVALID_SOCKET;
    WSAPROTOCOL_INFOA *pi;
    int providers[] = {6, 0};
    int ret, err;
    UINT pi_size;

    /* Set pi_size explicitly to a value below 2*sizeof(WSAPROTOCOL_INFOA)
     * to avoid a crash on win98.
     */
    pi_size = 0;
    ret = WSAEnumProtocolsA(providers, NULL, &pi_size);
    ok(ret == SOCKET_ERROR, "WSAEnumProtocolsA({6,0}, NULL, 0) returned %d\n",
            ret);
    err = WSAGetLastError();
    ok(err == WSAENOBUFS, "WSAEnumProtocolsA error is %d, not WSAENOBUFS(%d)\n",
            err, WSAENOBUFS);

    pi = HeapAlloc(GetProcessHeap(), 0, pi_size);
    ok(pi != NULL, "Failed to allocate memory\n");
    if (pi == NULL) {
        skip("Can't continue without memory.\n");
        return;
    }

    ret = WSAEnumProtocolsA(providers, pi, &pi_size);
    ok(ret != SOCKET_ERROR, "WSAEnumProtocolsA failed, last error is %d\n",
            WSAGetLastError());

    if (ret == 0) {
        skip("No protocols enumerated.\n");
        HeapFree(GetProcessHeap(), 0, pi);
        return;
    }

    sock = WSASocketA(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO,
                      FROM_PROTOCOL_INFO, &pi[0], 0, 0);
    ok(sock != INVALID_SOCKET, "Failed to create socket: %d\n",
            WSAGetLastError());

    closesocket(sock);
    HeapFree(GetProcessHeap(), 0, pi);
}

static void test_WSAAddressToStringA(void)
{
    SOCKET v6 = INVALID_SOCKET;
    INT ret;
    DWORD len;
    int GLE;
    SOCKADDR_IN sockaddr;
    CHAR address[22]; /* 12 digits + 3 dots + ':' + 5 digits + '\0' */

    CHAR expect1[] = "0.0.0.0";
    CHAR expect2[] = "255.255.255.255";
    CHAR expect3[] = "0.0.0.0:65535";
    CHAR expect4[] = "255.255.255.255:65535";

    SOCKADDR_IN6 sockaddr6;
    CHAR address6[54]; /* 32 digits + 7':' + '[' + '%" + 5 digits + ']:' + 5 digits + '\0' */

    CHAR addr6_1[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01};
    CHAR addr6_2[] = {0x20,0xab,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01};
    CHAR addr6_3[] = {0x20,0xab,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x01};

    CHAR expect6_1[] = "::1";
    CHAR expect6_2[] = "20ab::1";
    CHAR expect6_3[] = "[20ab::2001]:33274";
    CHAR expect6_3_nt[] = "20ab::2001@33274";
    CHAR expect6_3_w2k[] = "20ab::2001";
    CHAR expect6_3_2[] = "[20ab::2001%4660]:33274";
    CHAR expect6_3_2_nt[] = "4660/20ab::2001@33274";
    CHAR expect6_3_2_w2k[] = "20ab::2001%4660";
    CHAR expect6_3_3[] = "20ab::2001%4660";
    CHAR expect6_3_3_nt[] = "4660/20ab::2001";

    len = 0;

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAAddressToStringA( (SOCKADDR*)&sockaddr, sizeof(sockaddr), NULL, address, &len );
    GLE = WSAGetLastError();
    ok( (ret == SOCKET_ERROR && GLE == WSAEFAULT) || (ret == 0), 
        "WSAAddressToStringA() failed unexpectedly: WSAGetLastError()=%d, ret=%d\n",
        GLE, ret );

    len = sizeof(address);

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAAddressToStringA( (SOCKADDR*)&sockaddr, sizeof(sockaddr), NULL, address, &len );
    ok( !ret, "WSAAddressToStringA() failed unexpectedly: %d\n", WSAGetLastError() );

    ok( !strcmp( address, expect1 ), "Expected: %s, got: %s\n", expect1, address );
    ok( len == sizeof( expect1 ), "Got size %d\n", len);

    len = sizeof(address);

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0xffffffff;

    ret = WSAAddressToStringA( (SOCKADDR*)&sockaddr, sizeof(sockaddr), NULL, address, &len );
    ok( !ret, "WSAAddressToStringA() failed unexpectedly: %d\n", WSAGetLastError() );

    ok( !strcmp( address, expect2 ), "Expected: %s, got: %s\n", expect2, address );

    len = sizeof(address);

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = 0xffff;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAAddressToStringA( (SOCKADDR*)&sockaddr, sizeof(sockaddr), NULL, address, &len );
    ok( !ret, "WSAAddressToStringA() failed unexpectedly: %d\n", WSAGetLastError() );

    ok( !strcmp( address, expect3 ), "Expected: %s, got: %s\n", expect3, address );

    len = sizeof(address);

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = 0xffff;
    sockaddr.sin_addr.s_addr = 0xffffffff;

    ret = WSAAddressToStringA( (SOCKADDR*)&sockaddr, sizeof(sockaddr), NULL, address, &len );
    ok( !ret, "WSAAddressToStringA() failed unexpectedly: %d\n", WSAGetLastError() );

    ok( !strcmp( address, expect4 ), "Expected: %s, got: %s\n", expect4, address );
    ok( len == sizeof( expect4 ), "Got size %d\n", len);

    /*check to see it IPv6 is available */
    v6 = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (v6 == INVALID_SOCKET) {
        skip("Could not create IPv6 socket (LastError: %d; %d expected if IPv6 not available).\n",
            WSAGetLastError(), WSAEAFNOSUPPORT);
        goto end;
    }
    /* Test a short IPv6 address */
    len = sizeof(address6);

    sockaddr6.sin6_family = AF_INET6;
    sockaddr6.sin6_port = 0x0000;
    sockaddr6.sin6_scope_id = 0;
    memcpy (sockaddr6.sin6_addr.s6_addr, addr6_1, sizeof(addr6_1));

    ret = WSAAddressToStringA( (SOCKADDR*)&sockaddr6, sizeof(sockaddr6), NULL, address6, &len );
    ok( !ret, "WSAAddressToStringA() failed unexpectedly: %d\n", WSAGetLastError() );
    ok( !strcmp( address6, expect6_1 ), "Expected: %s, got: %s\n", expect6_1, address6 );
    ok( len == sizeof(expect6_1), "Got size %d\n", len);

    /* Test a longer IPv6 address */
    len = sizeof(address6);

    sockaddr6.sin6_family = AF_INET6;
    sockaddr6.sin6_port = 0x0000;
    sockaddr6.sin6_scope_id = 0;
    memcpy (sockaddr6.sin6_addr.s6_addr, addr6_2, sizeof(addr6_2));

    ret = WSAAddressToStringA( (SOCKADDR*)&sockaddr6, sizeof(sockaddr6), NULL, address6, &len );
    ok( !ret, "WSAAddressToStringA() failed unexpectedly: %d\n", WSAGetLastError() );
    ok( !strcmp( address6, expect6_2 ), "Expected: %s, got: %s\n", expect6_2, address6 );
    ok( len == sizeof(expect6_2), "Got size %d\n", len);

    /* Test IPv6 address and port number */
    len = sizeof(address6);

    sockaddr6.sin6_family = AF_INET6;
    sockaddr6.sin6_port = 0xfa81;
    sockaddr6.sin6_scope_id = 0;
    memcpy (sockaddr6.sin6_addr.s6_addr, addr6_3, sizeof(addr6_3));

    ret = WSAAddressToStringA( (SOCKADDR*)&sockaddr6, sizeof(sockaddr6), NULL, address6, &len );
    ok( !ret, "WSAAddressToStringA() failed unexpectedly: %d\n", WSAGetLastError() );
    ok( !strcmp( address6, expect6_3 ) ||
        broken( !strcmp( address6, expect6_3_nt ) ) || /* NT4 */
        broken( !strcmp( address6, expect6_3_w2k ) ), /* Win2000 */
        "Expected: %s, got: %s\n", expect6_3, address6 );
    ok( len == sizeof(expect6_3) ||
        broken( len == sizeof(expect6_3_nt) ) || /* NT4 */
        broken( len == sizeof(expect6_3_w2k) ), /* Win2000 */
        "Got size %d\n", len);

    /* Test IPv6 address, port number and scope_id */
    len = sizeof(address6);

    sockaddr6.sin6_family = AF_INET6;
    sockaddr6.sin6_port = 0xfa81;
    sockaddr6.sin6_scope_id = 0x1234;
    memcpy (sockaddr6.sin6_addr.s6_addr, addr6_3, sizeof(addr6_3));

    ret = WSAAddressToStringA( (SOCKADDR*)&sockaddr6, sizeof(sockaddr6), NULL, address6, &len );
    ok( !ret, "WSAAddressToStringA() failed unexpectedly: %d\n", WSAGetLastError() );
    ok( !strcmp( address6, expect6_3_2 ) ||
        broken( !strcmp( address6, expect6_3_2_nt ) ) || /* NT4 */
        broken( !strcmp( address6, expect6_3_2_w2k ) ), /* Win2000 */
        "Expected: %s, got: %s\n", expect6_3_2, address6 );
    ok( len == sizeof(expect6_3_2) ||
        broken( len == sizeof(expect6_3_2_nt) ) || /* NT4 */
        broken( len == sizeof(expect6_3_2_w2k) ), /* Win2000 */
        "Got size %d\n", len);

    /* Test IPv6 address and scope_id */
    len = sizeof(address6);

    sockaddr6.sin6_family = AF_INET6;
    sockaddr6.sin6_port = 0x0000;
    sockaddr6.sin6_scope_id = 0x1234;
    memcpy (sockaddr6.sin6_addr.s6_addr, addr6_3, sizeof(addr6_3));

    ret = WSAAddressToStringA( (SOCKADDR*)&sockaddr6, sizeof(sockaddr6), NULL, address6, &len );
    ok( !ret, "WSAAddressToStringA() failed unexpectedly: %d\n", WSAGetLastError() );
    ok( !strcmp( address6, expect6_3_3 ) ||
        broken( !strcmp( address6, expect6_3_3_nt ) ), /* NT4 */
        "Expected: %s, got: %s\n", expect6_3_3, address6 );
    ok( len == sizeof(expect6_3_3) ||
        broken( len == sizeof(expect6_3_3_nt) ), /* NT4 */
        "Got size %d\n", len);

end:
    if (v6 != INVALID_SOCKET)
        closesocket(v6);
}

static void test_WSAAddressToStringW(void)
{
    SOCKET v6 = INVALID_SOCKET;
    INT ret;
    DWORD len;
    int GLE;
    SOCKADDR_IN sockaddr;
    WCHAR address[22]; /* 12 digits + 3 dots + ':' + 5 digits + '\0' */

    WCHAR expect1[] = { '0','.','0','.','0','.','0', 0 };
    WCHAR expect2[] = { '2','5','5','.','2','5','5','.','2','5','5','.','2','5','5', 0 };
    WCHAR expect3[] = { '0','.','0','.','0','.','0', ':', '6', '5', '5', '3', '5', 0 };
    WCHAR expect4[] = { '2','5','5','.','2','5','5','.','2','5','5','.','2','5','5', ':',
                        '6', '5', '5', '3', '5', 0 };

    SOCKADDR_IN6 sockaddr6;
    WCHAR address6[54]; /* 32 digits + 7':' + '[' + '%" + 5 digits + ']:' + 5 digits + '\0' */

    CHAR addr6_1[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01};
    CHAR addr6_2[] = {0x20,0xab,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01};
    CHAR addr6_3[] = {0x20,0xab,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x01};

    WCHAR expect6_1[] = {':',':','1',0};
    WCHAR expect6_2[] = {'2','0','a','b',':',':','1',0};
    WCHAR expect6_3[] = {'[','2','0','a','b',':',':','2','0','0','1',']',':','3','3','2','7','4',0};
    WCHAR expect6_3_nt[] = {'2','0','a','b',':',':','2','0','0','1','@','3','3','2','7','4',0};
    WCHAR expect6_3_w2k[] = {'2','0','a','b',':',':','2','0','0','1',0};
    WCHAR expect6_3_2[] = {'[','2','0','a','b',':',':','2','0','0','1','%','4','6','6','0',']',':','3','3','2','7','4',0};
    WCHAR expect6_3_2_nt[] = {'4','6','6','0','/','2','0','a','b',':',':','2','0','0','1','@','3','3','2','7','4',0};
    WCHAR expect6_3_2_w2k[] = {'2','0','a','b',':',':','2','0','0','1','%','4','6','6','0',0};
    WCHAR expect6_3_3[] = {'2','0','a','b',':',':','2','0','0','1','%','6','5','5','3','4',0};
    WCHAR expect6_3_3_nt[] = {'6','5','5','3','4','/','2','0','a','b',':',':','2','0','0','1',0};

    len = 0;

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAAddressToStringW( (SOCKADDR*)&sockaddr, sizeof(sockaddr), NULL, address, &len );
    GLE = WSAGetLastError();
    ok( (ret == SOCKET_ERROR && GLE == WSAEFAULT) || (ret == 0), 
        "WSAAddressToStringW() failed unexpectedly: WSAGetLastError()=%d, ret=%d\n",
        GLE, ret );

    len = sizeof(address);

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAAddressToStringW( (SOCKADDR*)&sockaddr, sizeof(sockaddr), NULL, address, &len );
    ok( !ret, "WSAAddressToStringW() failed unexpectedly: %d\n", WSAGetLastError() );

    ok( !lstrcmpW( address, expect1 ), "Expected different address string\n" );
    ok( len == sizeof( expect1 )/sizeof( WCHAR ), "Got size %d\n", len);

    len = sizeof(address);

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0xffffffff;

    ret = WSAAddressToStringW( (SOCKADDR*)&sockaddr, sizeof(sockaddr), NULL, address, &len );
    ok( !ret, "WSAAddressToStringW() failed unexpectedly: %d\n", WSAGetLastError() );

    ok( !lstrcmpW( address, expect2 ), "Expected different address string\n" );

    len = sizeof(address);

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = 0xffff;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAAddressToStringW( (SOCKADDR*)&sockaddr, sizeof(sockaddr), NULL, address, &len );
    ok( !ret, "WSAAddressToStringW() failed unexpectedly: %d\n", WSAGetLastError() );

    ok( !lstrcmpW( address, expect3 ), "Expected different address string\n" );

    len = sizeof(address);

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = 0xffff;
    sockaddr.sin_addr.s_addr = 0xffffffff;

    ret = WSAAddressToStringW( (SOCKADDR*)&sockaddr, sizeof(sockaddr), NULL, address, &len );
    ok( !ret, "WSAAddressToStringW() failed unexpectedly: %d\n", WSAGetLastError() );

    ok( !lstrcmpW( address, expect4 ), "Expected different address string\n" );
    ok( len == sizeof( expect4 )/sizeof( WCHAR ), "Got %d\n", len);

    /*check to see it IPv6 is available */
    v6 = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (v6 == INVALID_SOCKET) {
        skip("Could not create IPv6 socket (LastError: %d; %d expected if IPv6 not available).\n",
            WSAGetLastError(), WSAEAFNOSUPPORT);
        goto end;
    }

    /* Test a short IPv6 address */
    len = sizeof(address6)/sizeof(WCHAR);

    sockaddr6.sin6_family = AF_INET6;
    sockaddr6.sin6_port = 0x0000;
    sockaddr6.sin6_scope_id = 0;
    memcpy (sockaddr6.sin6_addr.s6_addr, addr6_1, sizeof(addr6_1));

    ret = WSAAddressToStringW( (SOCKADDR*)&sockaddr6, sizeof(sockaddr6), NULL, address6, &len );
    ok( !ret, "WSAAddressToStringW() failed unexpectedly: %d\n", WSAGetLastError() );
    ok( !lstrcmpW( address6, expect6_1 ), "Wrong string returned\n" );
    ok( len == sizeof(expect6_1)/sizeof(WCHAR), "Got %d\n", len);

    /* Test a longer IPv6 address */
    len = sizeof(address6)/sizeof(WCHAR);

    sockaddr6.sin6_family = AF_INET6;
    sockaddr6.sin6_port = 0x0000;
    sockaddr6.sin6_scope_id = 0;
    memcpy (sockaddr6.sin6_addr.s6_addr, addr6_2, sizeof(addr6_2));

    ret = WSAAddressToStringW( (SOCKADDR*)&sockaddr6, sizeof(sockaddr6), NULL, address6, &len );
    ok( !ret, "WSAAddressToStringW() failed unexpectedly: %d\n", WSAGetLastError() );

    ok( !lstrcmpW( address6, expect6_2 ), "Wrong string returned\n" );
    ok( len == sizeof(expect6_2)/sizeof(WCHAR), "Got %d\n", len);

    /* Test IPv6 address and port number */
    len = sizeof(address6)/sizeof(WCHAR);

    sockaddr6.sin6_family = AF_INET6;
    sockaddr6.sin6_port = 0xfa81;
    sockaddr6.sin6_scope_id = 0;
    memcpy (sockaddr6.sin6_addr.s6_addr, addr6_3, sizeof(addr6_3));

    ret = WSAAddressToStringW( (SOCKADDR*)&sockaddr6, sizeof(sockaddr6), NULL, address6, &len );
    ok( !ret, "WSAAddressToStringW() failed unexpectedly: %d\n", WSAGetLastError() );
    ok( !lstrcmpW( address6, expect6_3 ) ||
        broken( !lstrcmpW( address6, expect6_3_nt ) ) || /* NT4 */
        broken( !lstrcmpW( address6, expect6_3_w2k ) ), /* Win2000 */
        "Expected: %s, got: %s\n", wine_dbgstr_w(expect6_3),
        wine_dbgstr_w(address6) );
    ok( len == sizeof(expect6_3)/sizeof(WCHAR) ||
        broken(len == sizeof(expect6_3_nt)/sizeof(WCHAR) ) || /* NT4 */
        broken(len == sizeof(expect6_3_w2k)/sizeof(WCHAR) ), /* Win2000 */
        "Got %d\n", len);

    /* Test IPv6 address, port number and scope_id */
    len = sizeof(address6)/sizeof(WCHAR);

    sockaddr6.sin6_family = AF_INET6;
    sockaddr6.sin6_port = 0xfa81;
    sockaddr6.sin6_scope_id = 0x1234;
    memcpy (sockaddr6.sin6_addr.s6_addr, addr6_3, sizeof(addr6_3));

    ret = WSAAddressToStringW( (SOCKADDR*)&sockaddr6, sizeof(sockaddr6), NULL, address6, &len );
    ok( !ret, "WSAAddressToStringW() failed unexpectedly: %d\n", WSAGetLastError() );
    ok( !lstrcmpW( address6, expect6_3_2 ) ||
        broken( !lstrcmpW( address6, expect6_3_2_nt ) ) || /* NT4 */
        broken( !lstrcmpW( address6, expect6_3_2_w2k ) ), /* Win2000 */
        "Expected: %s, got: %s\n", wine_dbgstr_w(expect6_3_2),
        wine_dbgstr_w(address6) );
    ok( len == sizeof(expect6_3_2)/sizeof(WCHAR) ||
        broken( len == sizeof(expect6_3_2_nt)/sizeof(WCHAR) ) || /* NT4 */
        broken( len == sizeof(expect6_3_2_w2k)/sizeof(WCHAR) ), /* Win2000 */
        "Got %d\n", len);

    /* Test IPv6 address and scope_id */
    len = sizeof(address6)/sizeof(WCHAR);

    sockaddr6.sin6_family = AF_INET6;
    sockaddr6.sin6_port = 0x0000;
    sockaddr6.sin6_scope_id = 0xfffe;
    memcpy (sockaddr6.sin6_addr.s6_addr, addr6_3, sizeof(addr6_3));

    ret = WSAAddressToStringW( (SOCKADDR*)&sockaddr6, sizeof(sockaddr6), NULL, address6, &len );
    ok( !ret, "WSAAddressToStringW() failed unexpectedly: %d\n", WSAGetLastError() );
    ok( !lstrcmpW( address6, expect6_3_3 ) ||
        broken( !lstrcmpW( address6, expect6_3_3_nt ) ), /* NT4 */
        "Expected: %s, got: %s\n", wine_dbgstr_w(expect6_3_3),
        wine_dbgstr_w(address6) );
    ok( len == sizeof(expect6_3_3)/sizeof(WCHAR) ||
        broken( len == sizeof(expect6_3_3_nt)/sizeof(WCHAR) ), /* NT4 */
        "Got %d\n", len);

end:
    if (v6 != INVALID_SOCKET)
        closesocket(v6);
}

static void test_WSAStringToAddressA(void)
{
    INT ret, len;
    SOCKADDR_IN sockaddr;
    SOCKADDR_IN6 sockaddr6;
    int GLE;

    CHAR address1[] = "0.0.0.0";
    CHAR address2[] = "127.127.127.127";
    CHAR address3[] = "255.255.255.255";
    CHAR address4[] = "127.127.127.127:65535";
    CHAR address5[] = "255.255.255.255:65535";
    CHAR address6[] = "::1";
    CHAR address7[] = "[::1]";
    CHAR address8[] = "[::1]:65535";

    len = 0;
    sockaddr.sin_family = AF_INET;

    ret = WSAStringToAddressA( address1, AF_INET, NULL, (SOCKADDR*)&sockaddr, &len );
    ok( ret == SOCKET_ERROR, "WSAStringToAddressA() succeeded unexpectedly: %d\n",
        WSAGetLastError() );

    len = sizeof(sockaddr);
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAStringToAddressA( address1, AF_INET, NULL, (SOCKADDR*)&sockaddr, &len );
    ok( !ret && sockaddr.sin_addr.s_addr == 0,
        "WSAStringToAddressA() failed unexpectedly: %d\n", WSAGetLastError() );

    len = sizeof(sockaddr);
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAStringToAddressA( address2, AF_INET, NULL, (SOCKADDR*)&sockaddr, &len );
    ok( !ret && sockaddr.sin_addr.s_addr == 0x7f7f7f7f,
        "WSAStringToAddressA() failed unexpectedly: %d\n", WSAGetLastError() );

    len = sizeof(sockaddr);
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAStringToAddressA( address3, AF_INET, NULL, (SOCKADDR*)&sockaddr, &len );
    GLE = WSAGetLastError();
    ok( (ret == 0 && sockaddr.sin_addr.s_addr == 0xffffffff) || 
        (ret == SOCKET_ERROR && (GLE == ERROR_INVALID_PARAMETER || GLE == WSAEINVAL)),
        "WSAStringToAddressA() failed unexpectedly: %d\n", GLE );

    len = sizeof(sockaddr);
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAStringToAddressA( address4, AF_INET, NULL, (SOCKADDR*)&sockaddr, &len );
    ok( !ret && sockaddr.sin_addr.s_addr == 0x7f7f7f7f && sockaddr.sin_port == 0xffff,
        "WSAStringToAddressA() failed unexpectedly: %d\n", WSAGetLastError() );

    len = sizeof(sockaddr);
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAStringToAddressA( address5, AF_INET, NULL, (SOCKADDR*)&sockaddr, &len );
    GLE = WSAGetLastError();
    ok( (ret == 0 && sockaddr.sin_addr.s_addr == 0xffffffff && sockaddr.sin_port == 0xffff) || 
        (ret == SOCKET_ERROR && (GLE == ERROR_INVALID_PARAMETER || GLE == WSAEINVAL)),
        "WSAStringToAddressA() failed unexpectedly: %d\n", GLE );

    len = sizeof(sockaddr6);
    memset(&sockaddr6, 0, len);
    sockaddr6.sin6_family = AF_INET6;

    ret = WSAStringToAddressA( address6, AF_INET6, NULL, (SOCKADDR*)&sockaddr6,
            &len );
    GLE = WSAGetLastError();
    ok( ret == 0 || (ret == SOCKET_ERROR && GLE == WSAEINVAL),
        "WSAStringToAddressA() failed for IPv6 address: %d\n", GLE);

    len = sizeof(sockaddr6);
    memset(&sockaddr6, 0, len);
    sockaddr6.sin6_family = AF_INET6;

    ret = WSAStringToAddressA( address7, AF_INET6, NULL, (SOCKADDR*)&sockaddr6,
            &len );
    GLE = WSAGetLastError();
    ok( ret == 0 || (ret == SOCKET_ERROR && GLE == WSAEINVAL),
        "WSAStringToAddressA() failed for IPv6 address: %d\n", GLE);

    len = sizeof(sockaddr6);
    memset(&sockaddr6, 0, len);
    sockaddr6.sin6_family = AF_INET6;

    ret = WSAStringToAddressA( address8, AF_INET6, NULL, (SOCKADDR*)&sockaddr6,
            &len );
    GLE = WSAGetLastError();
    ok( (ret == 0 && sockaddr6.sin6_port == 0xffff) ||
        (ret == SOCKET_ERROR && GLE == WSAEINVAL),
        "WSAStringToAddressA() failed for IPv6 address: %d\n", GLE);

}

static void test_WSAStringToAddressW(void)
{
    INT ret, len;
    SOCKADDR_IN sockaddr, *sin;
    SOCKADDR_IN6 sockaddr6;
    SOCKADDR_STORAGE sockaddr_storage;
    int GLE;

    WCHAR address1[] = { '0','.','0','.','0','.','0', 0 };
    WCHAR address2[] = { '1','2','7','.','1','2','7','.','1','2','7','.','1','2','7', 0 };
    WCHAR address3[] = { '2','5','5','.','2','5','5','.','2','5','5','.','2','5','5', 0 };
    WCHAR address4[] = { '1','2','7','.','1','2','7','.','1','2','7','.','1','2','7',
                         ':', '6', '5', '5', '3', '5', 0 };
    WCHAR address5[] = { '2','5','5','.','2','5','5','.','2','5','5','.','2','5','5', ':',
                         '6', '5', '5', '3', '5', 0 };
    WCHAR address6[] = {':',':','1','\0'};
    WCHAR address7[] = {'[',':',':','1',']','\0'};
    WCHAR address8[] = {'[',':',':','1',']',':','6','5','5','3','5','\0'};

    len = 0;
    sockaddr.sin_family = AF_INET;

    ret = WSAStringToAddressW( address1, AF_INET, NULL, (SOCKADDR*)&sockaddr, &len );
    ok( ret == SOCKET_ERROR, "WSAStringToAddressW() failed unexpectedly: %d\n",
        WSAGetLastError() );

    len = sizeof(sockaddr);
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAStringToAddressW( address1, AF_INET, NULL, (SOCKADDR*)&sockaddr, &len );
    ok( !ret && sockaddr.sin_addr.s_addr == 0,
        "WSAStringToAddressW() failed unexpectedly: %d\n", WSAGetLastError() );

    len = sizeof(sockaddr);
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAStringToAddressW( address2, AF_INET, NULL, (SOCKADDR*)&sockaddr, &len );
    ok( !ret && sockaddr.sin_addr.s_addr == 0x7f7f7f7f,
        "WSAStringToAddressW() failed unexpectedly: %d\n", WSAGetLastError() );

    len = sizeof(sockaddr);
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAStringToAddressW( address3, AF_INET, NULL, (SOCKADDR*)&sockaddr, &len );
    GLE = WSAGetLastError();
    ok( (ret == 0 && sockaddr.sin_addr.s_addr == 0xffffffff) || 
        (ret == SOCKET_ERROR && (GLE == ERROR_INVALID_PARAMETER || GLE == WSAEINVAL)),
        "WSAStringToAddressW() failed unexpectedly: %d\n", GLE );

    len = sizeof(sockaddr);
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAStringToAddressW( address4, AF_INET, NULL, (SOCKADDR*)&sockaddr, &len );
    ok( !ret && sockaddr.sin_addr.s_addr == 0x7f7f7f7f && sockaddr.sin_port == 0xffff,
        "WSAStringToAddressW() failed unexpectedly: %d\n", WSAGetLastError() );

    len = sizeof(sockaddr);
    sockaddr.sin_port = 0;
    sockaddr.sin_addr.s_addr = 0;

    ret = WSAStringToAddressW( address5, AF_INET, NULL, (SOCKADDR*)&sockaddr, &len );
    ok( (ret == 0 && sockaddr.sin_addr.s_addr == 0xffffffff && sockaddr.sin_port == 0xffff) || 
        (ret == SOCKET_ERROR && (GLE == ERROR_INVALID_PARAMETER || GLE == WSAEINVAL)),
        "WSAStringToAddressW() failed unexpectedly: %d\n", GLE );

    /* Test with a larger buffer than necessary */
    len = sizeof(sockaddr_storage);
    sin = (SOCKADDR_IN *)&sockaddr_storage;
    sin->sin_port = 0;
    sin->sin_addr.s_addr = 0;

    ret = WSAStringToAddressW( address5, AF_INET, NULL, (SOCKADDR*)sin, &len );
    ok( (ret == 0 && sin->sin_addr.s_addr == 0xffffffff && sin->sin_port == 0xffff) ||
        (ret == SOCKET_ERROR && (GLE == ERROR_INVALID_PARAMETER || GLE == WSAEINVAL)),
        "WSAStringToAddressW() failed unexpectedly: %d\n", GLE );
    ok( len == sizeof(SOCKADDR_IN) ||
        broken(len == sizeof(SOCKADDR_STORAGE)) /* NT4/2k */,
        "unexpected length %d\n", len );

    len = sizeof(sockaddr6);
    memset(&sockaddr6, 0, len);
    sockaddr6.sin6_family = AF_INET6;

    ret = WSAStringToAddressW( address6, AF_INET6, NULL, (SOCKADDR*)&sockaddr6,
            &len );
    GLE = WSAGetLastError();
    ok( ret == 0 || (ret == SOCKET_ERROR && GLE == WSAEINVAL),
        "WSAStringToAddressW() failed for IPv6 address: %d\n", GLE);

    len = sizeof(sockaddr6);
    memset(&sockaddr6, 0, len);
    sockaddr6.sin6_family = AF_INET6;

    ret = WSAStringToAddressW( address7, AF_INET6, NULL, (SOCKADDR*)&sockaddr6,
            &len );
    GLE = WSAGetLastError();
    ok( ret == 0 || (ret == SOCKET_ERROR && GLE == WSAEINVAL),
        "WSAStringToAddressW() failed for IPv6 address: %d\n", GLE);

    len = sizeof(sockaddr6);
    memset(&sockaddr6, 0, len);
    sockaddr6.sin6_family = AF_INET6;

    ret = WSAStringToAddressW( address8, AF_INET6, NULL, (SOCKADDR*)&sockaddr6,
            &len );
    GLE = WSAGetLastError();
    ok( (ret == 0 && sockaddr6.sin6_port == 0xffff) ||
        (ret == SOCKET_ERROR && GLE == WSAEINVAL),
        "WSAStringToAddressW() failed for IPv6 address: %d\n", GLE);

}

static DWORD WINAPI SelectReadThread(void *param)
{
    select_thread_params *par = param;
    fd_set readfds;
    int ret;
    struct sockaddr_in addr;
    struct timeval select_timeout;

    FD_ZERO(&readfds);
    FD_SET(par->s, &readfds);
    select_timeout.tv_sec=5;
    select_timeout.tv_usec=0;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(SERVERIP);
    addr.sin_port = htons(SERVERPORT);

    do_bind(par->s, (struct sockaddr *)&addr, sizeof(addr));
    wsa_ok(listen(par->s, SOMAXCONN ), 0 ==, "SelectReadThread (%x): listen failed: %d\n");

    SetEvent(server_ready);
    ret = select(par->s+1, &readfds, NULL, NULL, &select_timeout);
    par->ReadKilled = (ret == 1);

    return 0;
}

static void test_errors(void)
{
    SOCKET sock;
    SOCKADDR_IN  SockAddr;
    int ret, err;

    WSASetLastError(NO_ERROR);
    sock = socket(PF_INET, SOCK_STREAM, 0);
    ok( (sock != INVALID_SOCKET), "socket failed unexpectedly: %d\n", WSAGetLastError() );
    memset(&SockAddr, 0, sizeof(SockAddr));
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(6924);
    SockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = connect(sock, (PSOCKADDR)&SockAddr, sizeof(SockAddr));
    ok( (ret == SOCKET_ERROR), "expected SOCKET_ERROR, got: %d\n", ret );
    if (ret == SOCKET_ERROR)
    {
        err = WSAGetLastError();
        ok( (err == WSAECONNREFUSED), "expected WSAECONNREFUSED, got: %d\n", err );
    }

    {
        TIMEVAL timeval;
        fd_set set = {1, {sock}};

        timeval.tv_sec = 0;
        timeval.tv_usec = 50000;

        ret = select(1, NULL, &set, NULL, &timeval);
        ok( (ret == 0), "expected 0 (timeout), got: %d\n", ret );
    }

    ret = closesocket(sock);
    ok ( (ret == 0), "closesocket failed unexpectedly: %d\n", WSAGetLastError());
}

static void test_select(void)
{
    SOCKET fdRead, fdWrite;
    fd_set readfds, writefds, exceptfds;
    unsigned int maxfd;
    int ret;
    char buffer;
    struct timeval select_timeout;
    select_thread_params thread_params;
    HANDLE thread_handle;
    DWORD id;

    fdRead = socket(AF_INET, SOCK_STREAM, 0);
    ok( (fdRead != INVALID_SOCKET), "socket failed unexpectedly: %d\n", WSAGetLastError() );
    fdWrite = socket(AF_INET, SOCK_STREAM, 0);
    ok( (fdWrite != INVALID_SOCKET), "socket failed unexpectedly: %d\n", WSAGetLastError() );
 
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    FD_SET(fdRead, &readfds);
    FD_SET(fdWrite, &writefds);
    FD_SET(fdRead, &exceptfds);
    FD_SET(fdWrite, &exceptfds);
    select_timeout.tv_sec=0;
    select_timeout.tv_usec=500;

    maxfd = fdRead;
    if (fdWrite > maxfd)
        maxfd = fdWrite;
       
    todo_wine {
    ret = select(maxfd+1, &readfds, &writefds, &exceptfds, &select_timeout);
    ok ( (ret == 0), "select should not return any socket handles\n");
    ok ( !FD_ISSET(fdRead, &readfds), "FD should not be set\n");
    }
    ok ( !FD_ISSET(fdWrite, &writefds), "FD should not be set\n");

    ok ( !FD_ISSET(fdRead, &exceptfds), "FD should not be set\n");
    ok ( !FD_ISSET(fdWrite, &exceptfds), "FD should not be set\n");
 
    todo_wine {
    ok ((listen(fdWrite, SOMAXCONN) == SOCKET_ERROR), "listen did not fail\n");
    }
    ret = closesocket(fdWrite);
    ok ( (ret == 0), "closesocket failed unexpectedly: %d\n", ret);

    thread_params.s = fdRead;
    thread_params.ReadKilled = FALSE;
    server_ready = CreateEventA(NULL, TRUE, FALSE, NULL);
    thread_handle = CreateThread (NULL, 0, SelectReadThread, &thread_params, 0, &id );
    ok ( (thread_handle != NULL), "CreateThread failed unexpectedly: %d\n", GetLastError());

    WaitForSingleObject (server_ready, INFINITE);
    Sleep(200);
    ret = closesocket(fdRead);
    ok ( (ret == 0), "closesocket failed unexpectedly: %d\n", ret);

    WaitForSingleObject (thread_handle, 1000);
    ok ( (thread_params.ReadKilled) ||
         broken(thread_params.ReadKilled == 0), /*Win98*/
            "closesocket did not wakeup select\n");
    ret = recv(fdRead, &buffer, 1, MSG_PEEK);
    ok( (ret == -1), "peek at closed socket expected -1 got %d\n", ret);

    /* Test selecting invalid handles */
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);

    SetLastError(0);
    ret = select(maxfd+1, 0, 0, 0, &select_timeout);
    ok ( (ret == SOCKET_ERROR), "expected SOCKET_ERROR, got %i\n", ret);
    ok ( WSAGetLastError() == WSAEINVAL, "expected WSAEINVAL, got %i\n", WSAGetLastError());

    SetLastError(0);
    ret = select(maxfd+1, &readfds, &writefds, &exceptfds, &select_timeout);
    ok ( (ret == SOCKET_ERROR), "expected SOCKET_ERROR, got %i\n", ret);
    ok ( WSAGetLastError() == WSAEINVAL, "expected WSAEINVAL, got %i\n", WSAGetLastError());

    FD_SET(INVALID_SOCKET, &readfds);
    SetLastError(0);
    ret = select(maxfd+1, &readfds, &writefds, &exceptfds, &select_timeout);
    ok ( (ret == SOCKET_ERROR), "expected SOCKET_ERROR, got %i\n", ret);
    ok ( WSAGetLastError() == WSAENOTSOCK, "expected WSAENOTSOCK, got %i\n", WSAGetLastError());
    ok ( !FD_ISSET(fdRead, &readfds), "FD should not be set\n");

    FD_ZERO(&readfds);
    FD_SET(INVALID_SOCKET, &writefds);
    SetLastError(0);
    ret = select(maxfd+1, &readfds, &writefds, &exceptfds, &select_timeout);
    ok ( (ret == SOCKET_ERROR), "expected SOCKET_ERROR, got %i\n", ret);
    ok ( WSAGetLastError() == WSAENOTSOCK, "expected WSAENOTSOCK, got %i\n", WSAGetLastError());
    ok ( !FD_ISSET(fdRead, &writefds), "FD should not be set\n");

    FD_ZERO(&writefds);
    FD_SET(INVALID_SOCKET, &exceptfds);
    SetLastError(0);
    ret = select(maxfd+1, &readfds, &writefds, &exceptfds, &select_timeout);
    ok ( (ret == SOCKET_ERROR), "expected SOCKET_ERROR, got %i\n", ret);
    ok ( WSAGetLastError() == WSAENOTSOCK, "expected WSAENOTSOCK, got %i\n", WSAGetLastError());
    ok ( !FD_ISSET(fdRead, &exceptfds), "FD should not be set\n");
}

static DWORD WINAPI AcceptKillThread(void *param)
{
    select_thread_params *par = param;
    struct sockaddr_in address;
    int len = sizeof(address);
    SOCKET client_socket;

    SetEvent(server_ready);
    client_socket = accept(par->s, (struct sockaddr*) &address, &len);
    if (client_socket != INVALID_SOCKET)
        closesocket(client_socket);
    par->ReadKilled = (client_socket == INVALID_SOCKET);
    return 0;
}


static int CALLBACK AlwaysDeferConditionFunc(LPWSABUF lpCallerId, LPWSABUF lpCallerData, LPQOS pQos,
                                             LPQOS lpGQOS, LPWSABUF lpCalleeId, LPWSABUF lpCalleeData,
                                             GROUP FAR * g, DWORD_PTR dwCallbackData)
{
    return CF_DEFER;
}

static void test_accept(void)
{
    int ret;
    SOCKET server_socket = INVALID_SOCKET, accepted = INVALID_SOCKET, connector = INVALID_SOCKET;
    struct sockaddr_in address;
    int socklen;
    select_thread_params thread_params;
    HANDLE thread_handle = NULL;
    DWORD id;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET)
    {
        trace("error creating server socket: %d\n", WSAGetLastError());
        goto done;
    }

    memset(&address, 0, sizeof(address));
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_family = AF_INET;
    ret = bind(server_socket, (struct sockaddr*) &address, sizeof(address));
    if (ret != 0)
    {
        trace("error binding server socket: %d\n", WSAGetLastError());
        goto done;
    }

    socklen = sizeof(address);
    ret = getsockname(server_socket, (struct sockaddr*)&address, &socklen);
    if (ret != 0) {
        skip("failed to lookup bind address, error %d\n", WSAGetLastError());
        goto done;
    }

    ret = listen(server_socket, 5);
    if (ret != 0)
    {
        trace("error making server socket listen: %d\n", WSAGetLastError());
        goto done;
    }

    trace("Blocking accept next\n");

    connector = socket(AF_INET, SOCK_STREAM, 0);
    ok(connector != INVALID_SOCKET, "Failed to create connector socket, error %d\n", WSAGetLastError());

    ret = connect(connector, (struct sockaddr*)&address, sizeof(address));
    ok(ret == 0, "connecting to accepting socket failed, error %d\n", WSAGetLastError());

    accepted = WSAAccept(server_socket, NULL, NULL, AlwaysDeferConditionFunc, 0);
    ok(accepted == INVALID_SOCKET && WSAGetLastError() == WSATRY_AGAIN, "Failed to defer connection, %d\n", WSAGetLastError());

    accepted = accept(server_socket, NULL, 0);
    ok(accepted != INVALID_SOCKET, "Failed to accept deferred connection, error %d\n", WSAGetLastError());

    server_ready = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (server_ready == INVALID_HANDLE_VALUE)
    {
        trace("error creating event: %d\n", GetLastError());
        goto done;
    }

    thread_params.s = server_socket;
    thread_params.ReadKilled = FALSE;
    thread_handle = CreateThread(NULL, 0, AcceptKillThread, &thread_params, 0, &id);
    if (thread_handle == NULL)
    {
        trace("error creating thread: %d\n", GetLastError());
        goto done;
    }

    WaitForSingleObject(server_ready, INFINITE);
    Sleep(200);
    ret = closesocket(server_socket);
    if (ret != 0)
    {
        trace("closesocket failed: %d\n", WSAGetLastError());
        goto done;
    }

    WaitForSingleObject(thread_handle, 1000);
    ok(thread_params.ReadKilled || broken(!thread_params.ReadKilled) /* Win98/ME, after accept */,
       "closesocket did not wakeup accept\n");

done:
    if (accepted != INVALID_SOCKET)
        closesocket(accepted);
    if (connector != INVALID_SOCKET)
        closesocket(connector);
    if (thread_handle != NULL)
        CloseHandle(thread_handle);
    if (server_ready != INVALID_HANDLE_VALUE)
        CloseHandle(server_ready);
    if (server_socket != INVALID_SOCKET)
        closesocket(server_socket);
}

static void test_extendedSocketOptions(void)
{
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in sa;
    int sa_len = sizeof(struct sockaddr_in);
    int optval, optlen = sizeof(int), ret;
    BOOL bool_opt_val;
    LINGER linger_val;

    if(WSAStartup(MAKEWORD(2,0), &wsa)){
        trace("Winsock failed: %d. Aborting test\n", WSAGetLastError());
        return;
    }

    memset(&sa, 0, sa_len);

    sa.sin_family = AF_INET;
    sa.sin_port = htons(0);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP)) == INVALID_SOCKET) {
        trace("Creating the socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    if(bind(sock, (struct sockaddr *) &sa, sa_len) < 0){
        trace("Failed to bind socket: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return;
    }

    ret = getsockopt(sock, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *)&optval, &optlen);

    ok(ret == 0, "getsockopt failed to query SO_MAX_MSG_SIZE, return value is 0x%08x\n", ret);
    ok((optval == 65507) || (optval == 65527),
            "SO_MAX_MSG_SIZE reported %d, expected 65507 or 65527\n", optval);

    /* IE 3 use 0xffffffff instead of SOL_SOCKET (0xffff) */
    SetLastError(0xdeadbeef);
    optval = 0xdeadbeef;
    optlen = sizeof(int);
    ret = getsockopt(sock, 0xffffffff, SO_MAX_MSG_SIZE, (char *)&optval, &optlen);
    ok( (ret == SOCKET_ERROR) && (WSAGetLastError() == WSAEINVAL),
        "got %d with %d and optval: 0x%x/%d (expected SOCKET_ERROR with WSAEINVAL)\n",
        ret, WSAGetLastError(), optval, optval);

    /* more invalid values for level */
    SetLastError(0xdeadbeef);
    optval = 0xdeadbeef;
    optlen = sizeof(int);
    ret = getsockopt(sock, 0x1234ffff, SO_MAX_MSG_SIZE, (char *)&optval, &optlen);
    ok( (ret == SOCKET_ERROR) && (WSAGetLastError() == WSAEINVAL),
        "got %d with %d and optval: 0x%x/%d (expected SOCKET_ERROR with WSAEINVAL)\n",
        ret, WSAGetLastError(), optval, optval);

    SetLastError(0xdeadbeef);
    optval = 0xdeadbeef;
    optlen = sizeof(int);
    ret = getsockopt(sock, 0x8000ffff, SO_MAX_MSG_SIZE, (char *)&optval, &optlen);
    ok( (ret == SOCKET_ERROR) && (WSAGetLastError() == WSAEINVAL),
        "got %d with %d and optval: 0x%x/%d (expected SOCKET_ERROR with WSAEINVAL)\n",
        ret, WSAGetLastError(), optval, optval);

    SetLastError(0xdeadbeef);
    optval = 0xdeadbeef;
    optlen = sizeof(int);
    ret = getsockopt(sock, 0x00008000, SO_MAX_MSG_SIZE, (char *)&optval, &optlen);
    ok( (ret == SOCKET_ERROR) && (WSAGetLastError() == WSAEINVAL),
        "got %d with %d and optval: 0x%x/%d (expected SOCKET_ERROR with WSAEINVAL)\n",
        ret, WSAGetLastError(), optval, optval);

    SetLastError(0xdeadbeef);
    optval = 0xdeadbeef;
    optlen = sizeof(int);
    ret = getsockopt(sock, 0x00000800, SO_MAX_MSG_SIZE, (char *)&optval, &optlen);
    ok( (ret == SOCKET_ERROR) && (WSAGetLastError() == WSAEINVAL),
        "got %d with %d and optval: 0x%x/%d (expected SOCKET_ERROR with WSAEINVAL)\n",
        ret, WSAGetLastError(), optval, optval);

    SetLastError(0xdeadbeef);
    optlen = sizeof(LINGER);
    ret = getsockopt(sock, SOL_SOCKET, SO_LINGER, (char *)&linger_val, &optlen);
    ok( (ret == SOCKET_ERROR) && (WSAGetLastError() == WSAENOPROTOOPT), 
        "getsockopt should fail for UDP sockets setting last error to WSAENOPROTOOPT, got %d with %d\n", 
        ret, WSAGetLastError());
    closesocket(sock);

    if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_IP)) == INVALID_SOCKET) {
        trace("Creating the socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    if(bind(sock, (struct sockaddr *) &sa, sa_len) < 0){
        trace("Failed to bind socket: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return;
    }

    ret = getsockopt(sock, SOL_SOCKET, SO_LINGER, (char *)&linger_val, &optlen);
    ok(ret == 0, "getsockopt failed to query SO_LINGER, return value is 0x%08x\n", ret);

    optlen = sizeof(BOOL);
    ret = getsockopt(sock, SOL_SOCKET, SO_DONTLINGER, (char *)&bool_opt_val, &optlen);
    ok(ret == 0, "getsockopt failed to query SO_DONTLINGER, return value is 0x%08x\n", ret);
    ok((linger_val.l_onoff && !bool_opt_val) || (!linger_val.l_onoff && bool_opt_val),
            "Return value of SO_DONTLINGER is %d, but SO_LINGER returned l_onoff == %d.\n",
            bool_opt_val, linger_val.l_onoff);

    closesocket(sock);
    WSACleanup();
}

static void test_getsockname(void)
{
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in sa_set, sa_get;
    int sa_set_len = sizeof(struct sockaddr_in);
    int sa_get_len = sa_set_len;
    static const unsigned char null_padding[] = {0,0,0,0,0,0,0,0};
    int ret;

    if(WSAStartup(MAKEWORD(2,0), &wsa)){
        trace("Winsock failed: %d. Aborting test\n", WSAGetLastError());
        return;
    }

    memset(&sa_set, 0, sa_set_len);

    sa_set.sin_family = AF_INET;
    sa_set.sin_port = htons(0);
    sa_set.sin_addr.s_addr = htonl(INADDR_ANY);

    if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_IP)) == INVALID_SOCKET) {
        trace("Creating the socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    memcpy(&sa_get, &sa_set, sizeof(sa_set));
    if (getsockname(sock, (struct sockaddr*) &sa_get, &sa_get_len) == 0)
        ok(0, "getsockname on unbound socket should fail\n");
    else {
        ok(WSAGetLastError() == WSAEINVAL, "getsockname on unbound socket "
            "failed with %d, expected %d\n", WSAGetLastError(), WSAEINVAL);
        ok(memcmp(&sa_get, &sa_set, sizeof(sa_get)) == 0,
            "failed getsockname modified sockaddr when it shouldn't\n");
    }

    if(bind(sock, (struct sockaddr *) &sa_set, sa_set_len) < 0){
        trace("Failed to bind socket: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return;
    }

    if(getsockname(sock, (struct sockaddr *) &sa_get, &sa_get_len) != 0){
        trace("Failed to call getsockname: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return;
    }

    ret = memcmp(sa_get.sin_zero, null_padding, 8);
    ok(ret == 0 || broken(ret != 0), /* NT4 */
            "getsockname did not zero the sockaddr_in structure\n");

    closesocket(sock);
    WSACleanup();
}

static void test_dns(void)
{
    struct hostent *h;
    union memaddress
    {
        char *chr;
        void *mem;
    } addr;
    char **ptr;
    int acount;

    h = gethostbyname("");
    ok(h != NULL, "gethostbyname(\"\") failed with %d\n", h_errno);

    /* Use an address with valid alias names if possible */
    h = gethostbyname("source.winehq.org");
    if(!h)
    {
        skip("Can't test the hostent structure because gethostbyname failed\n");
        return;
    }

    /* The returned struct must be allocated in a very strict way. First we need to
     * count how many aliases there are because they must be located right after
     * the struct hostent size. Knowing the amount of aliases we know the exact
     * location of the first IP returned. Rule valid for >= XP, for older OS's
     * it's somewhat the opposite. */
    addr.mem = h + 1;
    if(h->h_addr_list == addr.mem) /* <= W2K */
    {
        skip("Skipping hostent tests since this OS is unsupported\n");
        return;
    }

    todo_wine ok(h->h_aliases == addr.mem,
       "hostent->h_aliases should be in %p, it is in %p\n", addr.mem, h->h_aliases);

    for(ptr = h->h_aliases, acount = 1; *ptr; ptr++) acount++;
    addr.chr += sizeof(*ptr) * acount;
    todo_wine ok(h->h_addr_list == addr.mem,
       "hostent->h_addr_list should be in %p, it is in %p\n", addr.mem, h->h_addr_list);

    for(ptr = h->h_addr_list, acount = 1; *ptr; ptr++) acount++;

    addr.chr += sizeof(*ptr) * acount;
    todo_wine ok(h->h_addr_list[0] == addr.mem,
       "hostent->h_addr_list[0] should be in %p, it is in %p\n", addr.mem, h->h_addr_list[0]);
}

#ifndef __REACTOS__
/* Our winsock headers don't define gethostname because it conflicts with the
 * definition in unistd.h. Define it here to get rid of the warning. */

int WINAPI gethostname(char *name, int namelen);
#endif

static void test_gethostbyname_hack(void)
{
    struct hostent *he;
    char name[256];
    static BYTE loopback[] = {127, 0, 0, 1};
    static BYTE magic_loopback[] = {127, 12, 34, 56};
    int ret;

    ret = gethostname(name, 256);
    ok(ret == 0, "gethostname() call failed: %d\n", WSAGetLastError());

    he = gethostbyname("localhost");
    ok(he != NULL, "gethostbyname(\"localhost\") failed: %d\n", h_errno);
    if(he)
    {
        if(he->h_length != 4)
        {
            skip("h_length is %d, not IPv4, skipping test.\n", he->h_length);
            return;
        }

        ok(memcmp(he->h_addr_list[0], loopback, he->h_length) == 0,
           "gethostbyname(\"localhost\") returned %d.%d.%d.%d\n",
           he->h_addr_list[0][0], he->h_addr_list[0][1], he->h_addr_list[0][2],
           he->h_addr_list[0][3]);
    }

    if(strcmp(name, "localhost") == 0)
    {
        skip("hostname seems to be \"localhost\", skipping test.\n");
        return;
    }

    he = NULL;
    he = gethostbyname(name);
    ok(he != NULL, "gethostbyname(\"%s\") failed: %d\n", name, h_errno);
    if(he)
    {
        if(he->h_length != 4)
        {
            skip("h_length is %d, not IPv4, skipping test.\n", he->h_length);
            return;
        }

        if (he->h_addr_list[0][0] == 127)
        {
            ok(memcmp(he->h_addr_list[0], magic_loopback, he->h_length) == 0,
               "gethostbyname(\"%s\") returned %d.%d.%d.%d not 127.12.34.56\n",
               name, he->h_addr_list[0][0], he->h_addr_list[0][1],
               he->h_addr_list[0][2], he->h_addr_list[0][3]);
        }
    }

    gethostbyname("nonexistent.winehq.org");
    /* Don't check for the return value, as some braindead ISPs will kindly
     * resolve nonexistent host names to addresses of the ISP's spam pages. */
}

static void test_inet_addr(void)
{
    u_long addr;

    addr = inet_addr(NULL);
    ok(addr == INADDR_NONE, "inet_addr succeeded unexpectedly\n");
}

static void test_addr_to_print(void)
{
    char dst[16];
    char dst6[64];
    const char * pdst;
    struct in_addr in;
    struct in6_addr in6;

    u_long addr0_Num = 0x00000000;
    PCSTR addr0_Str = "0.0.0.0";
    u_long addr1_Num = 0x20201015;
    PCSTR addr1_Str = "21.16.32.32";
    u_char addr2_Num[16] = {0,0,0,0,0,0,0,0,0,0,0xff,0xfe,0xcC,0x98,0xbd,0x74};
    PCSTR addr2_Str = "::fffe:cc98:bd74";
    u_char addr3_Num[16] = {0x20,0x30,0xa4,0xb1};
    PCSTR addr3_Str = "2030:a4b1::";
    u_char addr4_Num[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0xcC,0x98,0xbd,0x74};
    PCSTR addr4_Str = "::204.152.189.116";

    /* Test IPv4 addresses */
    in.s_addr = addr0_Num;

    pdst = inet_ntoa(*((struct in_addr*)&in.s_addr));
    ok(pdst != NULL, "inet_ntoa failed %s\n", dst);
    ok(!strcmp(pdst, addr0_Str),"Address %s != %s\n", pdst, addr0_Str);

    /* Test that inet_ntoa and inet_ntop return the same value */
    in.S_un.S_addr = addr1_Num;
    pdst = inet_ntoa(*((struct in_addr*)&in.s_addr));
    ok(pdst != NULL, "inet_ntoa failed %s\n", dst);
    ok(!strcmp(pdst, addr1_Str),"Address %s != %s\n", pdst, addr1_Str);

    /* InetNtop became available in Vista and Win2008 */
    if (!pInetNtop)
    {
        win_skip("InetNtop not present, not executing tests\n");
        return;
    }

    /* Second part of test */
    pdst = pInetNtop(AF_INET,(void*)&in.s_addr, dst, sizeof(dst));
    ok(pdst != NULL, "InetNtop failed %s\n", dst);
    ok(!strcmp(pdst, addr1_Str),"Address %s != %s\n", pdst, addr1_Str);

    /* Test invalid parm conditions */
    pdst = pInetNtop(1, (void*)&in.s_addr, dst, sizeof(dst));
    ok(pdst == NULL, "The pointer should not be returned (%p)\n", pdst);
    ok(WSAGetLastError() == WSAEAFNOSUPPORT, "Should be WSAEAFNOSUPPORT\n");

    /* Test Null destination */
    pdst = NULL;
    pdst = pInetNtop(AF_INET, (void*)&in.s_addr, NULL, sizeof(dst));
    ok(pdst == NULL, "The pointer should not be returned (%p)\n", pdst);
    ok(WSAGetLastError() == STATUS_INVALID_PARAMETER || WSAGetLastError() == WSAEINVAL /* Win7 */,
       "Should be STATUS_INVALID_PARAMETER or WSAEINVAL not 0x%x\n", WSAGetLastError());

    /* Test zero length passed */
    WSASetLastError(0);
    pdst = NULL;
    pdst = pInetNtop(AF_INET, (void*)&in.s_addr, dst, 0);
    ok(pdst == NULL, "The pointer should not be returned (%p)\n", pdst);
    ok(WSAGetLastError() == STATUS_INVALID_PARAMETER || WSAGetLastError() == WSAEINVAL /* Win7 */,
       "Should be STATUS_INVALID_PARAMETER or WSAEINVAL not 0x%x\n", WSAGetLastError());

    /* Test length one shorter than the address length */
    WSASetLastError(0);
    pdst = NULL;
    pdst = pInetNtop(AF_INET, (void*)&in.s_addr, dst, 6);
    ok(pdst == NULL, "The pointer should not be returned (%p)\n", pdst);
    ok(WSAGetLastError() == STATUS_INVALID_PARAMETER || WSAGetLastError() == WSAEINVAL /* Win7 */,
       "Should be STATUS_INVALID_PARAMETER or WSAEINVAL not 0x%x\n", WSAGetLastError());

    /* Test longer length is ok */
    WSASetLastError(0);
    pdst = NULL;
    pdst = pInetNtop(AF_INET, (void*)&in.s_addr, dst, sizeof(dst)+1);
    ok(pdst != NULL, "The pointer should  be returned (%p)\n", pdst);
    ok(!strcmp(pdst, addr1_Str),"Address %s != %s\n", pdst, addr1_Str);

    /* Test the IPv6 addresses */

    /* Test an zero prefixed IPV6 address */
    memcpy(in6.u.Byte, addr2_Num, sizeof(addr2_Num));
    pdst = pInetNtop(AF_INET6,(void*)&in6.s6_addr, dst6, sizeof(dst6));
    ok(pdst != NULL, "InetNtop failed %s\n", dst6);
    ok(!strcmp(pdst, addr2_Str),"Address %s != %s\n", pdst, addr2_Str);

    /* Test an zero suffixed IPV6 address */
    memcpy(in6.s6_addr, addr3_Num, sizeof(addr3_Num));
    pdst = pInetNtop(AF_INET6,(void*)&in6.s6_addr, dst6, sizeof(dst6));
    ok(pdst != NULL, "InetNtop failed %s\n", dst6);
    ok(!strcmp(pdst, addr3_Str),"Address %s != %s\n", pdst, addr3_Str);

    /* Test the IPv6 address contains the IPv4 address in IPv4 notation */
    memcpy(in6.s6_addr, addr4_Num, sizeof(addr4_Num));
    pdst = pInetNtop(AF_INET6, (void*)&in6.s6_addr, dst6, sizeof(dst6));
    ok(pdst != NULL, "InetNtop failed %s\n", dst6);
    ok(!strcmp(pdst, addr4_Str),"Address %s != %s\n", pdst, addr4_Str);

    /* Test invalid parm conditions */
    memcpy(in6.u.Byte, addr2_Num, sizeof(addr2_Num));

    /* Test Null destination */
    pdst = NULL;
    pdst = pInetNtop(AF_INET6, (void*)&in6.s6_addr, NULL, sizeof(dst6));
    ok(pdst == NULL, "The pointer should not be returned (%p)\n", pdst);
    ok(WSAGetLastError() == STATUS_INVALID_PARAMETER || WSAGetLastError() == WSAEINVAL /* Win7 */,
       "Should be STATUS_INVALID_PARAMETER or WSAEINVAL not 0x%x\n", WSAGetLastError());

    /* Test zero length passed */
    WSASetLastError(0);
    pdst = NULL;
    pdst = pInetNtop(AF_INET6, (void*)&in6.s6_addr, dst6, 0);
    ok(pdst == NULL, "The pointer should not be returned (%p)\n", pdst);
    ok(WSAGetLastError() == STATUS_INVALID_PARAMETER || WSAGetLastError() == WSAEINVAL /* Win7 */,
       "Should be STATUS_INVALID_PARAMETER or WSAEINVAL not 0x%x\n", WSAGetLastError());

    /* Test length one shorter than the address length */
    WSASetLastError(0);
    pdst = NULL;
    pdst = pInetNtop(AF_INET6, (void*)&in6.s6_addr, dst6, 16);
    ok(pdst == NULL, "The pointer should not be returned (%p)\n", pdst);
    ok(WSAGetLastError() == STATUS_INVALID_PARAMETER || WSAGetLastError() == WSAEINVAL /* Win7 */,
       "Should be STATUS_INVALID_PARAMETER or WSAEINVAL not 0x%x\n", WSAGetLastError());

    /* Test longer length is ok */
    WSASetLastError(0);
    pdst = NULL;
    pdst = pInetNtop(AF_INET6, (void*)&in6.s6_addr, dst6, 18);
    ok(pdst != NULL, "The pointer should be returned (%p)\n", pdst);
}

static void test_ioctlsocket(void)
{
    SOCKET sock;
    struct tcp_keepalive kalive;
    int ret, optval;
    static const LONG cmds[] = {FIONBIO, FIONREAD, SIOCATMARK};
    UINT i;
    u_long arg = 0;

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ok(sock != INVALID_SOCKET, "Creating the socket failed: %d\n", WSAGetLastError());
    if(sock == INVALID_SOCKET)
    {
        skip("Can't continue without a socket.\n");
        return;
    }

    for(i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++)
    {
        /* broken apps like defcon pass the argp value directly instead of a pointer to it */
        ret = ioctlsocket(sock, cmds[i], (u_long *)1);
        ok(ret == SOCKET_ERROR, "ioctlsocket succeeded unexpectedly\n");
        ret = WSAGetLastError();
        ok(ret == WSAEFAULT, "expected WSAEFAULT, got %d instead\n", ret);
    }

    /* A fresh and not connected socket has no urgent data, this test shows
     * that normal(not urgent) data returns a non-zero value for SIOCATMARK. */

    ret = ioctlsocket(sock, SIOCATMARK, &arg);
    ok(ret != SOCKET_ERROR, "ioctlsocket failed unexpectedly\n");
    ok(arg, "SIOCATMARK expected a non-zero value\n");

    /* when SO_OOBINLINE is set SIOCATMARK must always return TRUE */
    optval = 1;
    ret = setsockopt(sock, SOL_SOCKET, SO_OOBINLINE, (void *)&optval, sizeof(optval));
    ok(ret != SOCKET_ERROR, "setsockopt failed unexpectedly\n");
    arg = 0;
    ret = ioctlsocket(sock, SIOCATMARK, &arg);
    ok(ret != SOCKET_ERROR, "ioctlsocket failed unexpectedly\n");
    ok(arg, "SIOCATMARK expected a non-zero value\n");

    /* disable SO_OOBINLINE and get the same old bahavior */
    optval = 0;
    ret = setsockopt(sock, SOL_SOCKET, SO_OOBINLINE, (void *)&optval, sizeof(optval));
    ok(ret != SOCKET_ERROR, "setsockopt failed unexpectedly\n");
    arg = 0;
    ret = ioctlsocket(sock, SIOCATMARK, &arg);
    ok(arg, "SIOCATMARK expected a non-zero value\n");

    ret = WSAIoctl(sock, SIO_KEEPALIVE_VALS, &arg, 0, NULL, 0, &arg, NULL, NULL);
    ok(ret == SOCKET_ERROR, "WSAIoctl succeeded unexpectedly\n");
    ret = WSAGetLastError();
    ok(ret == WSAEFAULT || broken(ret == WSAEINVAL), "expected WSAEFAULT, got %d instead\n", ret);

    ret = WSAIoctl(sock, SIO_KEEPALIVE_VALS, NULL, sizeof(struct tcp_keepalive), NULL, 0, &arg, NULL, NULL);
    ok(ret == SOCKET_ERROR, "WSAIoctl succeeded unexpectedly\n");
    ret = WSAGetLastError();
    ok(ret == WSAEFAULT || broken(ret == WSAEINVAL), "expected WSAEFAULT, got %d instead\n", ret);

    /* broken used to catch W95, W98, NT4 */
    make_keepalive(kalive, 0, 0, 0);
    ret = WSAIoctl(sock, SIO_KEEPALIVE_VALS, &kalive, sizeof(struct tcp_keepalive), NULL, 0, &arg, NULL, NULL);
    ok(ret == 0 || broken(ret == SOCKET_ERROR), "WSAIoctl failed unexpectedly\n");

    make_keepalive(kalive, 1, 0, 0);
    ret = WSAIoctl(sock, SIO_KEEPALIVE_VALS, &kalive, sizeof(struct tcp_keepalive), NULL, 0, &arg, NULL, NULL);
    ok(ret == 0 || broken(ret == SOCKET_ERROR), "WSAIoctl failed unexpectedly\n");

    make_keepalive(kalive, 1, 1000, 1000);
    ret = WSAIoctl(sock, SIO_KEEPALIVE_VALS, &kalive, sizeof(struct tcp_keepalive), NULL, 0, &arg, NULL, NULL);
    ok(ret == 0 || broken(ret == SOCKET_ERROR), "WSAIoctl failed unexpectedly\n");

    make_keepalive(kalive, 1, 10000, 10000);
    ret = WSAIoctl(sock, SIO_KEEPALIVE_VALS, &kalive, sizeof(struct tcp_keepalive), NULL, 0, &arg, NULL, NULL);
    ok(ret == 0 || broken(ret == SOCKET_ERROR), "WSAIoctl failed unexpectedly\n");

    make_keepalive(kalive, 1, 100, 100);
    ret = WSAIoctl(sock, SIO_KEEPALIVE_VALS, &kalive, sizeof(struct tcp_keepalive), NULL, 0, &arg, NULL, NULL);
    ok(ret == 0 || broken(ret == SOCKET_ERROR), "WSAIoctl failed unexpectedly\n");

    make_keepalive(kalive, 0, 100, 100);
    ret = WSAIoctl(sock, SIO_KEEPALIVE_VALS, &kalive, sizeof(struct tcp_keepalive), NULL, 0, &arg, NULL, NULL);
    ok(ret == 0 || broken(ret == SOCKET_ERROR), "WSAIoctl failed unexpectedly\n");

    closesocket(sock);
}

static int drain_pause=0;
static DWORD WINAPI drain_socket_thread(LPVOID arg)
{
    char buffer[1024];
    SOCKET sock = *(SOCKET*)arg;
    int ret;

    while ((ret = recv(sock, buffer, sizeof(buffer), 0)) != 0)
    {
        if (ret < 0)
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
                fd_set readset;
                FD_ZERO(&readset);
                FD_SET(sock, &readset);
                select(sock+1, &readset, NULL, NULL, NULL);
                while (drain_pause)
                    Sleep(100);
            }
            else
                break;
        }
    }
    return 0;
}

static void test_send(void)
{
    SOCKET src = INVALID_SOCKET;
    SOCKET dst = INVALID_SOCKET;
    HANDLE hThread = NULL;
    const int buflen = 1024*1024;
    char *buffer = NULL;
    int ret, i, zero = 0;
    WSABUF buf;
    OVERLAPPED ov;
    BOOL bret;
    DWORD id, bytes_sent, dwRet;

    memset(&ov, 0, sizeof(ov));

    if (tcp_socketpair(&src, &dst) != 0)
    {
        ok(0, "creating socket pair failed, skipping test\n");
        return;
    }

    set_blocking(dst, FALSE);
    /* force disable buffering so we can get a pending overlapped request */
    ret = setsockopt(dst, SOL_SOCKET, SO_SNDBUF, (char *) &zero, sizeof(zero));
    ok(!ret, "setsockopt SO_SNDBUF failed: %d - %d\n", ret, GetLastError());

    hThread = CreateThread(NULL, 0, drain_socket_thread, &dst, 0, &id);
    if (hThread == NULL)
    {
        ok(0, "CreateThread failed, error %d\n", GetLastError());
        goto end;
    }

    buffer = HeapAlloc(GetProcessHeap(), 0, buflen);
    if (buffer == NULL)
    {
        ok(0, "HeapAlloc failed, error %d\n", GetLastError());
        goto end;
    }

    /* fill the buffer with some nonsense */
    for (i = 0; i < buflen; ++i)
    {
        buffer[i] = (char) i;
    }

    ret = send(src, buffer, buflen, 0);
    if (ret >= 0)
        ok(ret == buflen, "send should have sent %d bytes, but it only sent %d\n", buflen, ret);
    else
        ok(0, "send failed, error %d\n", WSAGetLastError());

    buf.buf = buffer;
    buf.len = buflen;

    ov.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    ok(ov.hEvent != NULL, "could not create event object, errno = %d\n", GetLastError());
    if (!ov.hEvent)
        goto end;

    bytes_sent = 0;
    WSASetLastError(12345);
    ret = WSASend(dst, &buf, 1, &bytes_sent, 0, &ov, NULL);
    ok((ret == SOCKET_ERROR && WSAGetLastError() == ERROR_IO_PENDING) || broken(bytes_sent == buflen),
       "Failed to start overlapped send %d - %d - %d/%d\n", ret, WSAGetLastError(), bytes_sent, buflen);

    /* don't check for completion yet, we may need to drain the buffer while still sending */
    set_blocking(src, FALSE);
    for (i = 0; i < buflen; ++i)
    {
        int j = 0;

        ret = recv(src, buffer, 1, 0);
        while (ret == SOCKET_ERROR && GetLastError() == WSAEWOULDBLOCK && j < 100)
        {
            j++;
            Sleep(50);
            ret = recv(src, buffer, 1, 0);
        }

        ok(ret == 1, "Failed to receive data %d - %d (got %d/%d)\n", ret, GetLastError(), i, buflen);
        if (ret != 1)
            break;

        ok(buffer[0] == (char) i, "Received bad data at position %d\n", i);
    }

    dwRet = WaitForSingleObject(ov.hEvent, 1000);
    ok(dwRet == WAIT_OBJECT_0, "Failed to wait for recv message: %d - %d\n", dwRet, GetLastError());
    if (dwRet == WAIT_OBJECT_0)
    {
        bret = GetOverlappedResult((HANDLE)dst, &ov, &bytes_sent, FALSE);
        ok((bret && bytes_sent == buflen) || broken(!bret && GetLastError() == ERROR_IO_INCOMPLETE) /* win9x */,
           "Got %d instead of %d (%d - %d)\n", bytes_sent, buflen, bret, GetLastError());
    }

    WSASetLastError(12345);
    ret = WSASend(INVALID_SOCKET, &buf, 1, NULL, 0, &ov, NULL);
    ok(ret == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK,
       "WSASend failed %d - %d\n", ret, WSAGetLastError());

    WSASetLastError(12345);
    ret = WSASend(dst, &buf, 1, NULL, 0, &ov, NULL);
    ok(ret == SOCKET_ERROR && WSAGetLastError() == ERROR_IO_PENDING,
       "Failed to start overlapped send %d - %d\n", ret, WSAGetLastError());

end:
    if (src != INVALID_SOCKET)
        closesocket(src);
    if (dst != INVALID_SOCKET)
        closesocket(dst);
    if (hThread != NULL)
        CloseHandle(hThread);
    if (ov.hEvent)
        CloseHandle(ov.hEvent);
    HeapFree(GetProcessHeap(), 0, buffer);
}

typedef struct async_message
{
    SOCKET socket;
    LPARAM lparam;
    struct async_message *next;
} async_message;

static struct async_message *messages_received;

#define WM_SOCKET (WM_USER+100)
static LRESULT CALLBACK ws2_test_WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    struct async_message *message;

    switch (msg)
    {
    case WM_SOCKET:
        message = HeapAlloc(GetProcessHeap(), 0, sizeof(*message));
        message->socket = (SOCKET) wparam;
        message->lparam = lparam;
        message->next = NULL;

        if (messages_received)
        {
            struct async_message *last = messages_received;
            while (last->next) last = last->next;
            last->next = message;
        }
        else
            messages_received = message;
        return 0;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

static void get_event_details(int event, int *bit, char *name)
{
    switch (event)
    {
        case FD_ACCEPT:
            if (bit) *bit = FD_ACCEPT_BIT;
            if (name) strcpy(name, "FD_ACCEPT");
            break;
        case FD_CONNECT:
            if (bit) *bit = FD_CONNECT_BIT;
            if (name) strcpy(name, "FD_CONNECT");
            break;
        case FD_READ:
            if (bit) *bit = FD_READ_BIT;
            if (name) strcpy(name, "FD_READ");
            break;
        case FD_OOB:
            if (bit) *bit = FD_OOB_BIT;
            if (name) strcpy(name, "FD_OOB");
            break;
        case FD_WRITE:
            if (bit) *bit = FD_WRITE_BIT;
            if (name) strcpy(name, "FD_WRITE");
            break;
        case FD_CLOSE:
            if (bit) *bit = FD_CLOSE_BIT;
            if (name) strcpy(name, "FD_CLOSE");
            break;
        default:
            if (bit) *bit = -1;
            if (name) sprintf(name, "bad%x", event);
    }
}

static const char *dbgstr_event_seq(const LPARAM *seq)
{
    static char message[1024];
    char name[12];
    int len = 1;

    message[0] = '[';
    message[1] = 0;
    while (*seq)
    {
        get_event_details(WSAGETSELECTEVENT(*seq), NULL, name);
        len += sprintf(message + len, "%s(%d) ", name, WSAGETSELECTERROR(*seq));
        seq++;
    }
    if (len > 1) len--;
    strcpy( message + len, "]" );
    return message;
}

static char *dbgstr_event_seq_result(SOCKET s, WSANETWORKEVENTS *netEvents)
{
    static char message[1024];
    struct async_message *curr = messages_received;
    int index, error, bit = 0;
    char name[12];
    int len = 1;

    message[0] = '[';
    message[1] = 0;
    while (1)
    {
        if (netEvents)
        {
            if (bit >= FD_MAX_EVENTS) break;
            if ( !(netEvents->lNetworkEvents & (1 << bit)) )
            {
                bit++;
                continue;
            }
            get_event_details(1 << bit, &index, name);
            error = netEvents->iErrorCode[index];
            bit++;
        }
        else
        {
            if (!curr) break;
            if (curr->socket != s)
            {
                curr = curr->next;
                continue;
            }
            get_event_details(WSAGETSELECTEVENT(curr->lparam), NULL, name);
            error = WSAGETSELECTERROR(curr->lparam);
            curr = curr->next;
        }

        len += sprintf(message + len, "%s(%d) ", name, error);
    }
    if (len > 1) len--;
    strcpy( message + len, "]" );
    return message;
}

static void flush_events(SOCKET s, HANDLE hEvent)
{
    WSANETWORKEVENTS netEvents;
    struct async_message *prev = NULL, *curr = messages_received;
    int ret;
    DWORD dwRet;

    if (hEvent != INVALID_HANDLE_VALUE)
    {
        dwRet = WaitForSingleObject(hEvent, 100);
        if (dwRet == WAIT_OBJECT_0)
        {
            ret = WSAEnumNetworkEvents(s, hEvent, &netEvents);
            if (ret)
                ok(0, "WSAEnumNetworkEvents failed, error %d\n", ret);
        }
    }
    else
    {
        while (curr)
        {
            if (curr->socket == s)
            {
                if (prev) prev->next = curr->next;
                else messages_received = curr->next;

                HeapFree(GetProcessHeap(), 0, curr);

                if (prev) curr = prev->next;
                else curr = messages_received;
            }
            else
            {
                prev = curr;
                curr = curr->next;
            }
        }
    }
}

static int match_event_sequence(SOCKET s, WSANETWORKEVENTS *netEvents, const LPARAM *seq)
{
    int event, index, error, events;
    struct async_message *curr;

    if (netEvents)
    {
        events = netEvents->lNetworkEvents;
        while (*seq)
        {
            event = WSAGETSELECTEVENT(*seq);
            error = WSAGETSELECTERROR(*seq);
            get_event_details(event, &index, NULL);

            if (!(events & event) && index != -1)
                return 0;
            if (events & event && index != -1)
            {
                if (netEvents->iErrorCode[index] != error)
                    return 0;
            }
            events &= ~event;
            seq++;
        }
        if (events)
            return 0;
    }
    else
    {
        curr = messages_received;
        while (curr)
        {
            if (curr->socket == s)
            {
                if (!*seq) return 0;
                if (*seq != curr->lparam) return 0;
                seq++;
            }
            curr = curr->next;
        }
        if (*seq)
            return 0;
    }
    return 1;
}

/* checks for a sequence of events, (order only checked if window is used) */
static void ok_event_sequence(SOCKET s, HANDLE hEvent, const LPARAM *seq, const LPARAM **broken_seqs, int completelyBroken)
{
    MSG msg;
    WSANETWORKEVENTS events, *netEvents = NULL;
    int ret;
    DWORD dwRet;

    if (hEvent != INVALID_HANDLE_VALUE)
    {
        netEvents = &events;

        dwRet = WaitForSingleObject(hEvent, 200);
        if (dwRet == WAIT_OBJECT_0)
        {
            ret = WSAEnumNetworkEvents(s, hEvent, netEvents);
            if (ret)
            {
                winetest_ok(0, "WSAEnumNetworkEvents failed, error %d\n", ret);
                return;
            }
        }
        else
            memset(netEvents, 0, sizeof(*netEvents));
    }
    else
    {
        Sleep(200);
        /* Run the message loop a little */
        while (PeekMessageA( &msg, 0, 0, 0, PM_REMOVE ))
        {
            DispatchMessageA(&msg);
        }
    }

    if (match_event_sequence(s, netEvents, seq))
    {
        winetest_ok(1, "Sequence matches expected: %s\n", dbgstr_event_seq(seq));
        flush_events(s, hEvent);
        return;
    }

    if (broken_seqs)
    {
        for (; *broken_seqs; broken_seqs++)
        {
            if (match_event_sequence(s, netEvents, *broken_seqs))
            {
                winetest_ok(broken(1), "Sequence matches broken: %s, expected %s\n", dbgstr_event_seq_result(s, netEvents), dbgstr_event_seq(seq));
                flush_events(s, hEvent);
                return;
            }
        }
    }

    winetest_ok(broken(completelyBroken), "Expected event sequence %s, got %s\n", dbgstr_event_seq(seq),
                dbgstr_event_seq_result(s, netEvents));
    flush_events(s, hEvent);
}

#define ok_event_seq (winetest_set_location(__FILE__, __LINE__), 0) ? (void)0 : ok_event_sequence

static void test_events(int useMessages)
{
    SOCKET server = INVALID_SOCKET;
    SOCKET src = INVALID_SOCKET, src2 = INVALID_SOCKET;
    SOCKET dst = INVALID_SOCKET, dst2 = INVALID_SOCKET;
    struct sockaddr_in addr;
    HANDLE hThread = NULL;
    HANDLE hEvent = INVALID_HANDLE_VALUE, hEvent2 = INVALID_HANDLE_VALUE;
    WNDCLASSEX wndclass;
    HWND hWnd = NULL;
    char *buffer = NULL;
    int bufferSize = 1024*1024;
    WSABUF bufs;
    OVERLAPPED ov, ov2;
    DWORD flags = 0;
    DWORD bytesReturned;
    DWORD id;
    int len;
    int ret;
    DWORD dwRet;
    BOOL bret;
    static char szClassName[] = "wstestclass";
    const LPARAM *broken_seq[3];
    static const LPARAM empty_seq[] = { 0 };
    static const LPARAM close_seq[] = { WSAMAKESELECTREPLY(FD_CLOSE, 0), 0 };
    static const LPARAM write_seq[] = { WSAMAKESELECTREPLY(FD_WRITE, 0), 0 };
    static const LPARAM read_seq[] = { WSAMAKESELECTREPLY(FD_READ, 0), 0 };
    static const LPARAM oob_seq[] = { WSAMAKESELECTREPLY(FD_OOB, 0), 0 };
    static const LPARAM connect_seq[] = { WSAMAKESELECTREPLY(FD_CONNECT, 0),
                                          WSAMAKESELECTREPLY(FD_WRITE, 0), 0 };
    static const LPARAM read_read_seq[] = { WSAMAKESELECTREPLY(FD_READ, 0),
                                            WSAMAKESELECTREPLY(FD_READ, 0), 0 };
    static const LPARAM read_write_seq[] = { WSAMAKESELECTREPLY(FD_READ, 0),
                                             WSAMAKESELECTREPLY(FD_WRITE, 0), 0 };
    static const LPARAM read_close_seq[] = { WSAMAKESELECTREPLY(FD_READ, 0),
                                             WSAMAKESELECTREPLY(FD_CLOSE, 0), 0 };

    memset(&ov, 0, sizeof(ov));
    memset(&ov2, 0, sizeof(ov2));

    /* don't use socketpair, we want connection event */
    src = socket(AF_INET, SOCK_STREAM, 0);
    if (src == INVALID_SOCKET)
    {
        ok(0, "creating socket pair failed (%d), skipping test\n", GetLastError());
        goto end;
    }

    src2 = socket(AF_INET, SOCK_STREAM, 0);
    if (src2 == INVALID_SOCKET)
    {
        ok(0, "creating socket pair failed (%d), skipping test\n", GetLastError());
        goto end;
    }

    len = sizeof(BOOL);
    if (getsockopt(src, SOL_SOCKET, SO_OOBINLINE, (void *)&bret, &len) == SOCKET_ERROR)
    {
        ok(0, "failed to get oobinline status, %d\n", GetLastError());
        goto end;
    }
    ok(bret == FALSE, "OOB not inline\n");

    if (useMessages)
    {
        trace("Event test using messages\n");

        wndclass.cbSize         = sizeof(wndclass);
        wndclass.style          = CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc    = ws2_test_WndProc;
        wndclass.cbClsExtra     = 0;
        wndclass.cbWndExtra     = 0;
        wndclass.hInstance      = GetModuleHandle(NULL);
        wndclass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
        wndclass.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);
        wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wndclass.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
        wndclass.lpszClassName  = szClassName;
        wndclass.lpszMenuName   = NULL;
        RegisterClassEx(&wndclass);

        hWnd = CreateWindow(szClassName, "WS2Test", WS_OVERLAPPEDWINDOW, 0, 0, 500, 500, NULL, NULL, GetModuleHandle(NULL), NULL);
        if (!hWnd)
        {
            ok(0, "failed to create window: %d\n", GetLastError());
            return;
        }

        ret = WSAAsyncSelect(src, hWnd, WM_SOCKET, FD_CONNECT | FD_READ | FD_OOB | FD_WRITE | FD_CLOSE);
        if (ret)
        {
            ok(0, "WSAAsyncSelect failed, error %d\n", ret);
            goto end;
        }

        ret = WSAAsyncSelect(src2, hWnd, WM_SOCKET, FD_CONNECT | FD_READ | FD_OOB | FD_WRITE | FD_CLOSE);
        if (ret)
        {
            ok(0, "WSAAsyncSelect failed, error %d\n", ret);
            goto end;
        }
    }
    else
    {
        trace("Event test using events\n");

        hEvent = WSACreateEvent();
        if (hEvent == INVALID_HANDLE_VALUE)
        {
            ok(0, "WSACreateEvent failed, error %d\n", GetLastError());
            goto end;
        }

        hEvent2 = WSACreateEvent();
        if (hEvent2 == INVALID_HANDLE_VALUE)
        {
            ok(0, "WSACreateEvent failed, error %d\n", GetLastError());
            goto end;
        }

        ret = WSAEventSelect(src, hEvent, FD_CONNECT | FD_READ | FD_OOB | FD_WRITE | FD_CLOSE);
        if (ret)
        {
            ok(0, "WSAEventSelect failed, error %d\n", ret);
            goto end;
        }

        ret = WSAEventSelect(src2, hEvent2, FD_CONNECT | FD_READ | FD_OOB | FD_WRITE | FD_CLOSE);
        if (ret)
        {
            ok(0, "WSAEventSelect failed, error %d\n", ret);
            goto end;
        }
    }

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET)
    {
        ok(0, "creating socket pair failed (%d), skipping test\n", GetLastError());
        goto end;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    ret = bind(server, (struct sockaddr*)&addr, sizeof(addr));
    if (ret != 0)
    {
        ok(0, "creating socket pair failed (%d), skipping test\n", GetLastError());
        goto end;
    }

    len = sizeof(addr);
    ret = getsockname(server, (struct sockaddr*)&addr, &len);
    if (ret != 0)
    {
        ok(0, "creating socket pair failed (%d), skipping test\n", GetLastError());
        goto end;
    }

    ret = listen(server, 2);
    if (ret != 0)
    {
        ok(0, "creating socket pair failed (%d), skipping test\n", GetLastError());
        goto end;
    }

    ret = connect(src, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
    {
        ok(0, "creating socket pair failed (%d), skipping test\n", GetLastError());
        goto end;
    }

    ret = connect(src2, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
    {
        ok(0, "creating socket pair failed (%d), skipping test\n", GetLastError());
        goto end;
    }

    len = sizeof(addr);
    dst = accept(server, (struct sockaddr*)&addr, &len);
    if (dst == INVALID_SOCKET)
    {
        ok(0, "creating socket pair failed (%d), skipping test\n", GetLastError());
        goto end;
    }

    len = sizeof(addr);
    dst2 = accept(server, (struct sockaddr*)&addr, &len);
    if (dst2 == INVALID_SOCKET)
    {
        ok(0, "creating socket pair failed (%d), skipping test\n", GetLastError());
        goto end;
    }

    closesocket(server);
    server = INVALID_SOCKET;

    /* On Windows it seems when a non-blocking socket sends to a
       blocking socket on the same host, the send() is BLOCKING,
       so make both sockets non-blocking. src is already non-blocking
       from the async select */

    if (set_blocking(dst, FALSE))
    {
        ok(0, "ioctlsocket failed, error %d\n", WSAGetLastError());
        goto end;
    }

    buffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferSize);
    if (buffer == NULL)
    {
        ok(0, "could not allocate memory for test\n");
        goto end;
    }

    ov.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (ov.hEvent == NULL)
    {
        ok(0, "could not create event object, errno = %d\n", GetLastError());
        goto end;
    }

    ov2.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (ov2.hEvent == NULL)
    {
        ok(0, "could not create event object, errno = %d\n", GetLastError());
        goto end;
    }

    /* FD_WRITE should be set initially, and allow us to send at least 1 byte */
    ok_event_seq(src, hEvent, connect_seq, NULL, 1);
    ok_event_seq(src2, hEvent2, connect_seq, NULL, 1);
    /* broken on all windows - FD_CONNECT error is garbage */

    /* Test simple send/recv */
    ret = send(dst, buffer, 100, 0);
    ok(ret == 100, "Failed to send buffer %d err %d\n", ret, GetLastError());
    ok_event_seq(src, hEvent, read_seq, NULL, 0);

    ret = recv(src, buffer, 1, MSG_PEEK);
    ok(ret == 1, "Failed to peek at recv buffer %d err %d\n", ret, GetLastError());
    ok_event_seq(src, hEvent, read_seq, NULL, 0);

    ret = recv(src, buffer, 50, 0);
    ok(ret == 50, "Failed to recv buffer %d err %d\n", ret, GetLastError());
    ok_event_seq(src, hEvent, read_seq, NULL, 0);

    ret = recv(src, buffer, 50, 0);
    ok(ret == 50, "Failed to recv buffer %d err %d\n", ret, GetLastError());
    ok_event_seq(src, hEvent, empty_seq, NULL, 0);

    /* fun fact - events are reenabled even on failure, but only for messages */
    ret = send(dst, "1", 1, 0);
    ok(ret == 1, "Failed to send buffer %d err %d\n", ret, GetLastError());
    ok_event_seq(src, hEvent, read_seq, NULL, 0);

    ret = recv(src, buffer, -1, 0);
    ok(ret == SOCKET_ERROR && (GetLastError() == WSAEFAULT || GetLastError() == WSAENOBUFS),
       "Failed to recv buffer %d err %d\n", ret, GetLastError());
    if (useMessages)
    {
        broken_seq[0] = empty_seq; /* win9x */
        broken_seq[1] = NULL;
        todo_wine ok_event_seq(src, hEvent, read_seq, broken_seq, 0);
    }
    else
        ok_event_seq(src, hEvent, empty_seq, NULL, 0);

    ret = recv(src, buffer, 1, 0);
    ok(ret == 1, "Failed to recv buffer %d err %d\n", ret, GetLastError());
    ok_event_seq(src, hEvent, empty_seq, NULL, 0);

    /* Interaction with overlapped */
    bufs.len = sizeof(char);
    bufs.buf = buffer;
    ret = WSARecv(src, &bufs, 1, &bytesReturned, &flags, &ov, NULL);
    ok(ret == SOCKET_ERROR && GetLastError() == ERROR_IO_PENDING,
       "WSARecv failed - %d error %d\n", ret, GetLastError());

    bufs.len = sizeof(char);
    bufs.buf = buffer+1;
    ret = WSARecv(src, &bufs, 1, &bytesReturned, &flags, &ov2, NULL);
    ok(ret == SOCKET_ERROR && GetLastError() == ERROR_IO_PENDING,
       "WSARecv failed - %d error %d\n", ret, GetLastError());

    ret = send(dst, "12", 2, 0);
    ok(ret == 2, "Failed to send buffer %d err %d\n", ret, GetLastError());
    broken_seq[0] = read_read_seq; /* win9x */
    broken_seq[1] = NULL;
    ok_event_seq(src, hEvent, empty_seq, broken_seq, 0);

    dwRet = WaitForSingleObject(ov.hEvent, 100);
    ok(dwRet == WAIT_OBJECT_0, "Failed to wait for recv message: %d - %d\n", dwRet, GetLastError());
    if (dwRet == WAIT_OBJECT_0)
    {
        bret = GetOverlappedResult((HANDLE)src, &ov, &bytesReturned, FALSE);
        ok((bret && bytesReturned == 1) || broken(!bret && GetLastError() == ERROR_IO_INCOMPLETE) /* win9x */,
           "Got %d instead of 1 (%d - %d)\n", bytesReturned, bret, GetLastError());
        ok(buffer[0] == '1', "Got %c instead of 1\n", buffer[0]);
    }

    dwRet = WaitForSingleObject(ov2.hEvent, 100);
    ok(dwRet == WAIT_OBJECT_0, "Failed to wait for recv message: %d - %d\n", dwRet, GetLastError());
    if (dwRet == WAIT_OBJECT_0)
    {
        bret = GetOverlappedResult((HANDLE)src, &ov2, &bytesReturned, FALSE);
        ok((bret && bytesReturned == 1) || broken(!bret && GetLastError() == ERROR_IO_INCOMPLETE) /* win9x */,
           "Got %d instead of 1 (%d - %d)\n", bytesReturned, bret, GetLastError());
        ok(buffer[1] == '2', "Got %c instead of 2\n", buffer[1]);
    }

    ret = send(dst, "1", 1, 0);
    ok(ret == 1, "Failed to send buffer %d err %d\n", ret, GetLastError());
    ok_event_seq(src, hEvent, read_seq, NULL, 0);

    ret = recv(src, buffer, 1, 0);
    ok(ret == 1, "Failed to empty buffer: %d - %d\n", ret, GetLastError());
    ok_event_seq(src, hEvent, empty_seq, NULL, 0);

    /* Notifications are delivered as soon as possible, blocked only on
     * async requests on the same type */
    bufs.len = sizeof(char);
    bufs.buf = buffer;
    ret = WSARecv(src, &bufs, 1, &bytesReturned, &flags, &ov, NULL);
    ok(ret == SOCKET_ERROR && GetLastError() == ERROR_IO_PENDING,
       "WSARecv failed - %d error %d\n", ret, GetLastError());

    if (0) {
    ret = send(dst, "1", 1, MSG_OOB);
    ok(ret == 1, "Failed to send buffer %d err %d\n", ret, GetLastError());
    ok_event_seq(src, hEvent, oob_seq, NULL, 0);
    }

    dwRet = WaitForSingleObject(ov.hEvent, 100);
    ok(dwRet == WAIT_TIMEOUT, "OOB message activated read?: %d - %d\n", dwRet, GetLastError());

    ret = send(dst, "2", 1, 0);
    ok(ret == 1, "Failed to send buffer %d err %d\n", ret, GetLastError());
    broken_seq[0] = read_seq;  /* win98 */
    broken_seq[1] = NULL;
    ok_event_seq(src, hEvent, empty_seq, broken_seq, 0);

    dwRet = WaitForSingleObject(ov.hEvent, 100);
    ok(dwRet == WAIT_OBJECT_0 || broken(dwRet == WAIT_TIMEOUT),
       "Failed to wait for recv message: %d - %d\n", dwRet, GetLastError());
    if (dwRet == WAIT_OBJECT_0)
    {
        bret = GetOverlappedResult((HANDLE)src, &ov, &bytesReturned, FALSE);
        ok((bret && bytesReturned == 1) || broken(!bret && GetLastError() == ERROR_IO_INCOMPLETE) /* win9x */,
           "Got %d instead of 1 (%d - %d)\n", bytesReturned, bret, GetLastError());
        ok(buffer[0] == '2', "Got %c instead of 2\n", buffer[0]);
    }
    else if (dwRet == WAIT_TIMEOUT)
    {
        /* this happens on win98. We get an FD_READ later on the next test */
        CancelIo((HANDLE) src);
    }

    if (0) {
    ret = recv(src, buffer, 1, MSG_OOB);
    todo_wine ok(ret == 1, "Failed to empty buffer: %d - %d\n", ret, GetLastError());
    /* We get OOB notification, but no data on wine */
    ok_event_seq(src, hEvent, empty_seq, NULL, 0);
    }

    /* Flood the send queue */
    hThread = CreateThread(NULL, 0, drain_socket_thread, &dst, 0, &id);
    if (hThread == NULL)
    {
        ok(0, "CreateThread failed, error %d\n", GetLastError());
        goto end;
    }

    /* Now FD_WRITE should not be set, because the socket send buffer isn't full yet */
    ok_event_seq(src, hEvent, empty_seq, NULL, 0);

    /* Now if we send a ton of data and the 'server' does not drain it fast
     * enough (set drain_pause to be sure), the socket send buffer will only
     * take some of it, and we will get a short write. This will trigger
     * another FD_WRITE event as soon as data is sent and more space becomes
     * available, but not any earlier. */
    drain_pause=1;
    do
    {
        ret = send(src, buffer, bufferSize, 0);
    } while (ret == bufferSize);
    drain_pause=0;
    if (ret >= 0 || WSAGetLastError() == WSAEWOULDBLOCK)
    {
        Sleep(400); /* win9x */
        broken_seq[0] = read_write_seq;
        broken_seq[1] = NULL;
        ok_event_seq(src, hEvent, write_seq, broken_seq, 0);
    }
    else
    {
        ok(0, "sending a lot of data failed with error %d\n", WSAGetLastError());
    }

    /* Test how FD_CLOSE is handled */
    ret = send(dst, "12", 2, 0);
    ok(ret == 2, "Failed to send buffer %d err %d\n", ret, GetLastError());

    /* Wait a little and let the send complete */
    Sleep(100);
    closesocket(dst);
    dst = INVALID_SOCKET;
    Sleep(100);

    /* We can never implement this in wine, best we can hope for is
       sending FD_CLOSE after the reads complete */
    broken_seq[0] = read_seq;  /* win9x */
    broken_seq[1] = NULL;
    todo_wine ok_event_seq(src, hEvent, read_close_seq, broken_seq, 0);

    ret = recv(src, buffer, 1, 0);
    ok(ret == 1, "Failed to empty buffer: %d - %d\n", ret, GetLastError());
    ok_event_seq(src, hEvent, read_seq, NULL, 0);

    ret = recv(src, buffer, 1, 0);
    ok(ret == 1, "Failed to empty buffer: %d - %d\n", ret, GetLastError());
    /* want it? it's here, but you can't have it */
    broken_seq[0] = close_seq;  /* win9x */
    broken_seq[1] = NULL;
    todo_wine ok_event_seq(src, hEvent, empty_seq, /* wine sends FD_CLOSE here */
                           broken_seq, 0);

    /* Test how FD_CLOSE is handled */
    ret = send(dst2, "12", 2, 0);
    ok(ret == 2, "Failed to send buffer %d err %d\n", ret, GetLastError());

    Sleep(200);
    shutdown(dst2, SD_SEND);
    Sleep(200);

    /* Some of the below are technically todo_wine, but our event sequence is still valid, so to prevent
       regressions, don't mark them as todo_wine, and mark windows as broken */
    broken_seq[0] = read_close_seq;
    broken_seq[1] = close_seq;
    broken_seq[2] = NULL;
    ok_event_seq(src2, hEvent2, read_seq, broken_seq, 0);

    ret = recv(src2, buffer, 1, 0);
    ok(ret == 1 || broken(!ret), "Failed to empty buffer: %d - %d\n", ret, GetLastError());
    broken_seq[0] = close_seq;  /* win98 */
    broken_seq[1] = NULL;
    ok_event_seq(src2, hEvent2, read_seq, broken_seq, 0);

    ret = recv(src2, buffer, 1, 0);
    ok(ret == 1 || broken(!ret), "Failed to empty buffer: %d - %d\n", ret, GetLastError());
    broken_seq[0] = empty_seq;
    broken_seq[1] = NULL;
    ok_event_seq(src2, hEvent2, close_seq, broken_seq, 0);

    ret = send(src2, "1", 1, 0);
    ok(ret == 1, "Sending to half-closed socket failed %d err %d\n", ret, GetLastError());
    ok_event_seq(src2, hEvent2, empty_seq, NULL, 0);

    ret = send(src2, "1", 1, 0);
    ok(ret == 1, "Sending to half-closed socket failed %d err %d\n", ret, GetLastError());
    ok_event_seq(src2, hEvent2, empty_seq, NULL, 0);

end:
    if (src != INVALID_SOCKET)
    {
        flush_events(src, hEvent);
        closesocket(src);
    }
    if (src2 != INVALID_SOCKET)
    {
        flush_events(src2, hEvent2);
        closesocket(src2);
    }
    HeapFree(GetProcessHeap(), 0, buffer);
    if (server != INVALID_SOCKET)
        closesocket(server);
    if (dst != INVALID_SOCKET)
        closesocket(dst);
    if (dst2 != INVALID_SOCKET)
        closesocket(dst2);
    if (hThread != NULL)
        CloseHandle(hThread);
    if (hWnd != NULL)
        DestroyWindow(hWnd);
    if (hEvent != NULL)
        CloseHandle(hEvent);
    if (hEvent2 != NULL)
        CloseHandle(hEvent2);
    if (ov.hEvent != NULL)
        CloseHandle(ov.hEvent);
    if (ov2.hEvent != NULL)
        CloseHandle(ov2.hEvent);
}

static void test_ipv6only(void)
{
    SOCKET v4 = INVALID_SOCKET,
           v6 = INVALID_SOCKET;
    struct sockaddr_in sin4;
    struct sockaddr_in6 sin6;
    int ret;

    memset(&sin4, 0, sizeof(sin4));
    sin4.sin_family = AF_INET;
    sin4.sin_port = htons(SERVERPORT);

    memset(&sin6, 0, sizeof(sin6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_port = htons(SERVERPORT);

    v6 = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (v6 == INVALID_SOCKET) {
        skip("Could not create IPv6 socket (LastError: %d; %d expected if IPv6 not available).\n",
            WSAGetLastError(), WSAEAFNOSUPPORT);
        goto end;
    }
    ret = bind(v6, (struct sockaddr*)&sin6, sizeof(sin6));
    if (ret) {
        skip("Could not bind IPv6 address (LastError: %d).\n",
            WSAGetLastError());
        goto end;
    }

    v4 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (v4 == INVALID_SOCKET) {
        skip("Could not create IPv4 socket (LastError: %d).\n",
            WSAGetLastError());
        goto end;
    }
    ret = bind(v4, (struct sockaddr*)&sin4, sizeof(sin4));
    ok(!ret, "Could not bind IPv4 address (LastError: %d; %d expected if IPv6 binds to IPv4 as well).\n",
        WSAGetLastError(), WSAEADDRINUSE);

end:
    if (v4 != INVALID_SOCKET)
        closesocket(v4);
    if (v6 != INVALID_SOCKET)
        closesocket(v6);
}

static void test_WSASendTo(void)
{
    SOCKET s;
    struct sockaddr_in addr;
    char buf[12] = "hello world";
    WSABUF data_buf;
    DWORD bytesSent;
    int ret;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(139);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    data_buf.len = sizeof(buf);
    data_buf.buf = buf;

    if( (s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        ok(0, "socket() failed error: %d\n", WSAGetLastError());
        return;
    }

    WSASetLastError(12345);
    ret = WSASendTo(INVALID_SOCKET, &data_buf, 1, NULL, 0, (struct sockaddr*)&addr, sizeof(addr), NULL, NULL);
    ok(ret == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK,
       "WSASendTo() failed: %d/%d\n", ret, WSAGetLastError());

    WSASetLastError(12345);
    ret = WSASendTo(s, &data_buf, 1, NULL, 0, (struct sockaddr*)&addr, sizeof(addr), NULL, NULL);
    ok(ret == SOCKET_ERROR && WSAGetLastError() == WSAEFAULT,
       "WSASendTo() failed: %d/%d\n", ret, WSAGetLastError());

    WSASetLastError(12345);
    if(WSASendTo(s, &data_buf, 1, &bytesSent, 0, (struct sockaddr*)&addr, sizeof(addr), NULL, NULL)) {
        ok(0, "WSASendTo() failed error: %d\n", WSAGetLastError());
        return;
    }
    ok(!WSAGetLastError(), "WSAGetLastError() should return zero after "
            "a successful call to WSASendTo()\n");
}

static DWORD WINAPI recv_thread(LPVOID arg)
{
    SOCKET sock = *(SOCKET *)arg;
    char buffer[32];
    WSABUF wsa;
    WSAOVERLAPPED ov;
    DWORD flags = 0;

    wsa.buf = buffer;
    wsa.len = sizeof(buffer);
    ov.hEvent = WSACreateEvent();
    WSARecv(sock, &wsa, 1, NULL, &flags, &ov, NULL);

    WaitForSingleObject(ov.hEvent, 1000);
    WSACloseEvent(ov.hEvent);
    return 0;
}

static void test_WSARecv(void)
{
    SOCKET src, dest, server = INVALID_SOCKET;
    char buf[20];
    WSABUF bufs;
    WSAOVERLAPPED ov;
    DWORD bytesReturned, flags, id;
    struct linger ling;
    struct sockaddr_in addr;
    int iret, len;
    DWORD dwret;
    BOOL bret;
    HANDLE thread;

    memset(&ov, 0, sizeof(ov));

    tcp_socketpair(&src, &dest);
    if (src == INVALID_SOCKET || dest == INVALID_SOCKET)
    {
        skip("failed to create sockets\n");
        goto end;
    }

    bufs.len = sizeof(buf);
    bufs.buf = buf;
    flags = 0;

    ov.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    ok(ov.hEvent != NULL, "could not create event object, errno = %d\n", GetLastError());
    if (!ov.hEvent)
        goto end;

    ling.l_onoff = 1;
    ling.l_linger = 0;
    iret = setsockopt (src, SOL_SOCKET, SO_LINGER, (char *) &ling, sizeof(ling));
    ok(!iret, "Failed to set linger %d\n", GetLastError());

    iret = WSARecv(dest, &bufs, 1, NULL, &flags, &ov, NULL);
    ok(iret == SOCKET_ERROR && GetLastError() == ERROR_IO_PENDING, "WSARecv failed - %d error %d\n", iret, GetLastError());

    iret = WSARecv(dest, &bufs, 1, &bytesReturned, &flags, &ov, NULL);
    ok(iret == SOCKET_ERROR && GetLastError() == ERROR_IO_PENDING, "WSARecv failed - %d error %d\n", iret, GetLastError());

    closesocket(src);
    src = INVALID_SOCKET;

    dwret = WaitForSingleObject(ov.hEvent, 1000);
    ok(dwret == WAIT_OBJECT_0, "Waiting for disconnect event failed with %d + errno %d\n", dwret, GetLastError());

    bret = GetOverlappedResult((HANDLE)dest, &ov, &bytesReturned, FALSE);
    todo_wine ok(!bret && (GetLastError() == ERROR_NETNAME_DELETED || broken(GetLastError() == ERROR_IO_INCOMPLETE) /* win9x */),
        "Did not get disconnect event: %d, error %d\n", bret, GetLastError());
    ok(bytesReturned == 0, "Bytes received is %d\n", bytesReturned);

    src = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    ok(src != INVALID_SOCKET, "failed to create socket %d\n", WSAGetLastError());
    if (src == INVALID_SOCKET) goto end;

    server = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    ok(server != INVALID_SOCKET, "failed to create socket %d\n", WSAGetLastError());
    if (server == INVALID_SOCKET) goto end;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    iret = bind(server, (struct sockaddr *)&addr, sizeof(addr));
    if (iret) goto end;

    len = sizeof(addr);
    iret = getsockname(server, (struct sockaddr *)&addr, &len);
    if (iret) goto end;

    iret = listen(server, 1);
    if (iret) goto end;

    iret = connect(src, (struct sockaddr *)&addr, sizeof(addr));
    if (iret) goto end;

    len = sizeof(addr);
    dest = accept(server, (struct sockaddr *)&addr, &len);
    ok(dest != INVALID_SOCKET, "failed to create socket %d\n", WSAGetLastError());
    if (dest == INVALID_SOCKET) goto end;

    send(src, "test message", sizeof("test message"), 0);
    thread = CreateThread(NULL, 0, recv_thread, &dest, 0, &id);
    CloseHandle(thread);

end:
    if (server != INVALID_SOCKET)
        closesocket(server);
    if (dest != INVALID_SOCKET)
        closesocket(dest);
    if (src != INVALID_SOCKET)
        closesocket(src);
    if (ov.hEvent)
        WSACloseEvent(ov.hEvent);
}

static void test_GetAddrInfoW(void)
{
    static const WCHAR port[] = {'8','0',0};
    static const WCHAR empty[] = {0};
    static const WCHAR localhost[] = {'l','o','c','a','l','h','o','s','t',0};
    static const WCHAR zero[] = {'0',0};
    int ret;
    ADDRINFOW *result, hint;

    if (!pGetAddrInfoW || !pFreeAddrInfoW)
    {
        win_skip("GetAddrInfoW and/or FreeAddrInfoW not present\n");
        return;
    }
    memset(&hint, 0, sizeof(ADDRINFOW));

    ret = pGetAddrInfoW(NULL, NULL, NULL, &result);
    ok(ret == WSAHOST_NOT_FOUND, "got %d expected WSAHOST_NOT_FOUND\n", ret);

    result = NULL;
    ret = pGetAddrInfoW(empty, NULL, NULL, &result);
    ok(!ret, "GetAddrInfoW failed with %d\n", WSAGetLastError());
    ok(result != NULL, "GetAddrInfoW failed\n");
    pFreeAddrInfoW(result);

    result = NULL;
    ret = pGetAddrInfoW(NULL, zero, NULL, &result);
    ok(!ret, "GetAddrInfoW failed with %d\n", WSAGetLastError());
    ok(result != NULL, "GetAddrInfoW failed\n");
    pFreeAddrInfoW(result);

    result = NULL;
    ret = pGetAddrInfoW(empty, zero, NULL, &result);
    ok(!ret, "GetAddrInfoW failed with %d\n", WSAGetLastError());
    ok(result != NULL, "GetAddrInfoW failed\n");
    pFreeAddrInfoW(result);

    result = NULL;
    ret = pGetAddrInfoW(localhost, NULL, NULL, &result);
    ok(!ret, "GetAddrInfoW failed with %d\n", WSAGetLastError());
    pFreeAddrInfoW(result);

    result = NULL;
    ret = pGetAddrInfoW(localhost, empty, NULL, &result);
    ok(!ret, "GetAddrInfoW failed with %d\n", WSAGetLastError());
    pFreeAddrInfoW(result);

    result = NULL;
    ret = pGetAddrInfoW(localhost, zero, NULL, &result);
    ok(!ret, "GetAddrInfoW failed with %d\n", WSAGetLastError());
    pFreeAddrInfoW(result);

    result = NULL;
    ret = pGetAddrInfoW(localhost, port, NULL, &result);
    ok(!ret, "GetAddrInfoW failed with %d\n", WSAGetLastError());
    pFreeAddrInfoW(result);

    result = NULL;
    ret = pGetAddrInfoW(localhost, port, &hint, &result);
    ok(!ret, "GetAddrInfoW failed with %d\n", WSAGetLastError());
    pFreeAddrInfoW(result);
}

static void test_getaddrinfo(void)
{
    int ret;
    ADDRINFOA *result, hint;

    if (!pgetaddrinfo || !pfreeaddrinfo)
    {
        win_skip("getaddrinfo and/or freeaddrinfo not present\n");
        return;
    }
    memset(&hint, 0, sizeof(ADDRINFOA));

    ret = pgetaddrinfo(NULL, NULL, NULL, &result);
    ok(ret == WSAHOST_NOT_FOUND, "got %d expected WSAHOST_NOT_FOUND\n", ret);

    result = NULL;
    ret = pgetaddrinfo("", NULL, NULL, &result);
    ok(!ret, "getaddrinfo failed with %d\n", WSAGetLastError());
    ok(result != NULL, "getaddrinfo failed\n");
    pfreeaddrinfo(result);

    result = NULL;
    ret = pgetaddrinfo(NULL, "0", NULL, &result);
    ok(!ret, "getaddrinfo failed with %d\n", WSAGetLastError());
    ok(result != NULL, "getaddrinfo failed\n");
    pfreeaddrinfo(result);

    result = NULL;
    ret = pgetaddrinfo("", "0", NULL, &result);
    ok(!ret, "getaddrinfo failed with %d\n", WSAGetLastError());
    ok(result != NULL, "getaddrinfo failed\n");
    pfreeaddrinfo(result);

    result = NULL;
    ret = pgetaddrinfo("localhost", NULL, NULL, &result);
    ok(!ret, "getaddrinfo failed with %d\n", WSAGetLastError());
    pfreeaddrinfo(result);

    result = NULL;
    ret = pgetaddrinfo("localhost", "", NULL, &result);
    ok(!ret, "getaddrinfo failed with %d\n", WSAGetLastError());
    pfreeaddrinfo(result);

    result = NULL;
    ret = pgetaddrinfo("localhost", "0", NULL, &result);
    ok(!ret, "getaddrinfo failed with %d\n", WSAGetLastError());
    pfreeaddrinfo(result);

    result = NULL;
    ret = pgetaddrinfo("localhost", "80", NULL, &result);
    ok(!ret, "getaddrinfo failed with %d\n", WSAGetLastError());
    pfreeaddrinfo(result);

    result = NULL;
    ret = pgetaddrinfo("localhost", "80", &hint, &result);
    ok(!ret, "getaddrinfo failed with %d\n", WSAGetLastError());
    pfreeaddrinfo(result);
}

static void test_ConnectEx(void)
{
    SOCKET listener = INVALID_SOCKET;
    SOCKET acceptor = INVALID_SOCKET;
    SOCKET connector = INVALID_SOCKET;
    struct sockaddr_in address, conaddress;
    int addrlen;
    OVERLAPPED overlapped;
    LPFN_CONNECTEX pConnectEx;
    GUID connectExGuid = WSAID_CONNECTEX;
    DWORD bytesReturned;
    char buffer[1024];
    BOOL bret;
    DWORD dwret;
    int iret;

    memset(&overlapped, 0, sizeof(overlapped));

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == INVALID_SOCKET) {
        skip("could not create listener socket, error %d\n", WSAGetLastError());
        goto end;
    }

    connector = socket(AF_INET, SOCK_STREAM, 0);
    if (connector == INVALID_SOCKET) {
        skip("could not create connector socket, error %d\n", WSAGetLastError());
        goto end;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    iret = bind(listener, (struct sockaddr*)&address, sizeof(address));
    if (iret != 0) {
        skip("failed to bind, error %d\n", WSAGetLastError());
        goto end;
    }

    addrlen = sizeof(address);
    iret = getsockname(listener, (struct sockaddr*)&address, &addrlen);
    if (iret != 0) {
        skip("failed to lookup bind address, error %d\n", WSAGetLastError());
        goto end;
    }

    if (set_blocking(listener, TRUE)) {
        skip("couldn't make socket non-blocking, error %d\n", WSAGetLastError());
        goto end;
    }

    iret = WSAIoctl(connector, SIO_GET_EXTENSION_FUNCTION_POINTER, &connectExGuid, sizeof(connectExGuid),
        &pConnectEx, sizeof(pConnectEx), &bytesReturned, NULL, NULL);
    if (iret) {
        win_skip("WSAIoctl failed to get ConnectEx with ret %d + errno %d\n", iret, WSAGetLastError());
        goto end;
    }

    bret = pConnectEx(INVALID_SOCKET, (struct sockaddr*)&address, addrlen, NULL, 0, &bytesReturned, &overlapped);
    ok(bret == FALSE && WSAGetLastError() == WSAENOTSOCK, "ConnectEx on invalid socket "
        "returned %d + errno %d\n", bret, WSAGetLastError());

    bret = pConnectEx(connector, (struct sockaddr*)&address, addrlen, NULL, 0, &bytesReturned, &overlapped);
    todo_wine ok(bret == FALSE && WSAGetLastError() == WSAEINVAL, "ConnectEx on a unbound socket "
        "returned %d + errno %d\n", bret, WSAGetLastError());
    if (bret == TRUE || WSAGetLastError() != WSAEINVAL)
    {
        acceptor = accept(listener, NULL, NULL);
        if (acceptor != INVALID_SOCKET) {
            closesocket(acceptor);
            acceptor = INVALID_SOCKET;
        }

        closesocket(connector);
        connector = socket(AF_INET, SOCK_STREAM, 0);
        if (connector == INVALID_SOCKET) {
            skip("could not create connector socket, error %d\n", WSAGetLastError());
            goto end;
        }
    }

    /* ConnectEx needs a bound socket */
    memset(&conaddress, 0, sizeof(conaddress));
    conaddress.sin_family = AF_INET;
    conaddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    iret = bind(connector, (struct sockaddr*)&conaddress, sizeof(conaddress));
    if (iret != 0) {
        skip("failed to bind, error %d\n", WSAGetLastError());
        goto end;
    }

    bret = pConnectEx(connector, (struct sockaddr*)&address, addrlen, NULL, 0, &bytesReturned, NULL);
    ok(bret == FALSE && WSAGetLastError() == ERROR_INVALID_PARAMETER, "ConnectEx on a NULL overlapped "
        "returned %d + errno %d\n", bret, WSAGetLastError());

    overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (overlapped.hEvent == NULL) {
        skip("could not create event object, errno = %d\n", GetLastError());
        goto end;
    }

    iret = listen(listener, 1);
    if (iret != 0) {
        skip("listening failed, errno = %d\n", WSAGetLastError());
        goto end;
    }

    bret = pConnectEx(connector, (struct sockaddr*)&address, addrlen, NULL, 0, &bytesReturned, &overlapped);
    ok(bret == FALSE && WSAGetLastError() == ERROR_IO_PENDING, "ConnectEx failed: "
        "returned %d + errno %d\n", bret, WSAGetLastError());
    dwret = WaitForSingleObject(overlapped.hEvent, 15000);
    ok(dwret == WAIT_OBJECT_0, "Waiting for connect event failed with %d + errno %d\n", dwret, GetLastError());

    bret = GetOverlappedResult((HANDLE)connector, &overlapped, &bytesReturned, FALSE);
    ok(bret, "Connecting failed, error %d\n", GetLastError());
    ok(bytesReturned == 0, "Bytes sent is %d\n", bytesReturned);

    closesocket(connector);
    connector = socket(AF_INET, SOCK_STREAM, 0);
    if (connector == INVALID_SOCKET) {
        skip("could not create connector socket, error %d\n", WSAGetLastError());
        goto end;
    }
    /* ConnectEx needs a bound socket */
    memset(&conaddress, 0, sizeof(conaddress));
    conaddress.sin_family = AF_INET;
    conaddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    iret = bind(connector, (struct sockaddr*)&conaddress, sizeof(conaddress));
    if (iret != 0) {
        skip("failed to bind, error %d\n", WSAGetLastError());
        goto end;
    }

    acceptor = accept(listener, NULL, NULL);
    if (acceptor != INVALID_SOCKET) {
        closesocket(acceptor);
    }

    buffer[0] = '1';
    buffer[1] = '2';
    buffer[2] = '3';
    bret = pConnectEx(connector, (struct sockaddr*)&address, addrlen, buffer, 3, &bytesReturned, &overlapped);
    ok(bret == FALSE && WSAGetLastError() == ERROR_IO_PENDING, "ConnectEx failed: "
        "returned %d + errno %d\n", bret, WSAGetLastError());
    dwret = WaitForSingleObject(overlapped.hEvent, 15000);
    ok(dwret == WAIT_OBJECT_0, "Waiting for connect event failed with %d + errno %d\n", dwret, GetLastError());

    bret = GetOverlappedResult((HANDLE)connector, &overlapped, &bytesReturned, FALSE);
    ok(bret, "Connecting failed, error %d\n", GetLastError());
    ok(bytesReturned == 3, "Bytes sent is %d\n", bytesReturned);

    acceptor = accept(listener, NULL, NULL);
    ok(acceptor != INVALID_SOCKET, "could not accept socket error %d\n", WSAGetLastError());

    bytesReturned = recv(acceptor, buffer, 3, 0);
    buffer[4] = 0;
    ok(bytesReturned == 3, "Didn't get all sent data, got only %d\n", bytesReturned);
    ok(buffer[0] == '1' && buffer[1] == '2' && buffer[2] == '3',
       "Failed to get the right data, expected '123', got '%s'\n", buffer);

    closesocket(connector);
    connector = socket(AF_INET, SOCK_STREAM, 0);
    if (connector == INVALID_SOCKET) {
        skip("could not create connector socket, error %d\n", WSAGetLastError());
        goto end;
    }
    /* ConnectEx needs a bound socket */
    memset(&conaddress, 0, sizeof(conaddress));
    conaddress.sin_family = AF_INET;
    conaddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    iret = bind(connector, (struct sockaddr*)&conaddress, sizeof(conaddress));
    if (iret != 0) {
        skip("failed to bind, error %d\n", WSAGetLastError());
        goto end;
    }

    if (acceptor != INVALID_SOCKET) {
        closesocket(acceptor);
        acceptor = INVALID_SOCKET;
    }

    /* Connect with error */
    closesocket(listener);
    listener = INVALID_SOCKET;

    address.sin_port = htons(1);

    bret = pConnectEx(connector, (struct sockaddr*)&address, addrlen, NULL, 0, &bytesReturned, &overlapped);
    ok(bret == FALSE && GetLastError(), "ConnectEx to bad destination failed: "
        "returned %d + errno %d\n", bret, GetLastError());

    if (GetLastError() == ERROR_IO_PENDING)
    {
        dwret = WaitForSingleObject(overlapped.hEvent, 15000);
        ok(dwret == WAIT_OBJECT_0, "Waiting for connect event failed with %d + errno %d\n", dwret, GetLastError());

        bret = GetOverlappedResult((HANDLE)connector, &overlapped, &bytesReturned, FALSE);
        ok(bret == FALSE && GetLastError() == ERROR_CONNECTION_REFUSED,
           "Connecting to a disconnected host returned error %d - %d\n", bret, WSAGetLastError());
    }
    else {
        ok(GetLastError() == WSAECONNREFUSED,
           "Connecting to a disconnected host returned error %d - %d\n", bret, WSAGetLastError());
    }

end:
    if (overlapped.hEvent)
        WSACloseEvent(overlapped.hEvent);
    if (listener != INVALID_SOCKET)
        closesocket(listener);
    if (acceptor != INVALID_SOCKET)
        closesocket(acceptor);
    if (connector != INVALID_SOCKET)
        closesocket(connector);
}

static void test_AcceptEx(void)
{
    SOCKET listener = INVALID_SOCKET;
    SOCKET acceptor = INVALID_SOCKET;
    SOCKET connector = INVALID_SOCKET;
    SOCKET connector2 = INVALID_SOCKET;
    struct sockaddr_in bindAddress, peerAddress, *readBindAddress, *readRemoteAddress;
    int socklen, optlen;
    GUID acceptExGuid = WSAID_ACCEPTEX, getAcceptExGuid = WSAID_GETACCEPTEXSOCKADDRS;
    LPFN_ACCEPTEX pAcceptEx = NULL;
    LPFN_GETACCEPTEXSOCKADDRS pGetAcceptExSockaddrs = NULL;
    fd_set fds_accept, fds_send;
    struct timeval timeout = {0,10}; /* wait for 10 milliseconds */
    int got, conn1, i;
    DWORD bytesReturned, connect_time;
    char buffer[1024], ipbuffer[32];
    OVERLAPPED overlapped;
    int iret, localSize = sizeof(struct sockaddr_in), remoteSize = localSize;
    BOOL bret;
    DWORD dwret;

    memset(&overlapped, 0, sizeof(overlapped));

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == INVALID_SOCKET) {
        skip("could not create listener socket, error %d\n", WSAGetLastError());
        goto end;
    }

    acceptor = socket(AF_INET, SOCK_STREAM, 0);
    if (acceptor == INVALID_SOCKET) {
        skip("could not create acceptor socket, error %d\n", WSAGetLastError());
        goto end;
    }

    connector = socket(AF_INET, SOCK_STREAM, 0);
    if (connector == INVALID_SOCKET) {
        skip("could not create connector socket, error %d\n", WSAGetLastError());
        goto end;
    }

    memset(&bindAddress, 0, sizeof(bindAddress));
    bindAddress.sin_family = AF_INET;
    bindAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    iret = bind(listener, (struct sockaddr*)&bindAddress, sizeof(bindAddress));
    if (iret != 0) {
        skip("failed to bind, error %d\n", WSAGetLastError());
        goto end;
    }

    socklen = sizeof(bindAddress);
    iret = getsockname(listener, (struct sockaddr*)&bindAddress, &socklen);
    if (iret != 0) {
        skip("failed to lookup bind address, error %d\n", WSAGetLastError());
        goto end;
    }

    if (set_blocking(listener, FALSE)) {
        skip("couldn't make socket non-blocking, error %d\n", WSAGetLastError());
        goto end;
    }

    iret = WSAIoctl(listener, SIO_GET_EXTENSION_FUNCTION_POINTER, &acceptExGuid, sizeof(acceptExGuid),
        &pAcceptEx, sizeof(pAcceptEx), &bytesReturned, NULL, NULL);
    if (iret) {
        skip("WSAIoctl failed to get AcceptEx with ret %d + errno %d\n", iret, WSAGetLastError());
        goto end;
    }

    iret = WSAIoctl(listener, SIO_GET_EXTENSION_FUNCTION_POINTER, &getAcceptExGuid, sizeof(getAcceptExGuid),
        &pGetAcceptExSockaddrs, sizeof(pGetAcceptExSockaddrs), &bytesReturned, NULL, NULL);
    if (iret) {
        skip("WSAIoctl failed to get GetAcceptExSockaddrs with ret %d + errno %d\n", iret, WSAGetLastError());
        goto end;
    }

    bret = pAcceptEx(INVALID_SOCKET, acceptor, buffer, sizeof(buffer) - 2*(sizeof(struct sockaddr_in) + 16),
        sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
        &bytesReturned, &overlapped);
    ok(bret == FALSE && WSAGetLastError() == WSAENOTSOCK, "AcceptEx on invalid listening socket "
        "returned %d + errno %d\n", bret, WSAGetLastError());

    bret = pAcceptEx(listener, INVALID_SOCKET, buffer, sizeof(buffer) - 2*(sizeof(struct sockaddr_in) + 16),
        sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
        &bytesReturned, &overlapped);
    ok(bret == FALSE && WSAGetLastError() == WSAEINVAL, "AcceptEx on invalid accepting socket "
        "returned %d + errno %d\n", bret, WSAGetLastError());

    bret = pAcceptEx(listener, acceptor, NULL, sizeof(buffer) - 2*(sizeof(struct sockaddr_in) + 16),
        sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
        &bytesReturned, &overlapped);
    ok(bret == FALSE &&
        (WSAGetLastError() == WSAEINVAL ||
         broken(WSAGetLastError() == WSAEFAULT)), /* NT4 */
        "AcceptEx on NULL buffer returned %d + errno %d\n", bret, WSAGetLastError());

    bret = pAcceptEx(listener, acceptor, buffer, 0, 0, sizeof(struct sockaddr_in) + 16,
        &bytesReturned, &overlapped);
    todo_wine ok(bret == FALSE && WSAGetLastError() == WSAEINVAL, "AcceptEx on too small local address size "
        "returned %d + errno %d\n", bret, WSAGetLastError());

    bret = pAcceptEx(listener, acceptor, buffer, 0, sizeof(struct sockaddr_in) + 16, 0,
        &bytesReturned, &overlapped);
    todo_wine ok(bret == FALSE && WSAGetLastError() == WSAEINVAL, "AcceptEx on too small remote address size "
        "returned %d + errno %d\n", bret, WSAGetLastError());

    bret = pAcceptEx(listener, acceptor, buffer, 0,
        sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
        &bytesReturned, NULL);
    ok(bret == FALSE && WSAGetLastError() == ERROR_INVALID_PARAMETER, "AcceptEx on a NULL overlapped "
        "returned %d + errno %d\n", bret, WSAGetLastError());

    bret = pAcceptEx(listener, acceptor, buffer, sizeof(buffer) - 2*(sizeof(struct sockaddr_in) + 16),
        sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
        &bytesReturned, &overlapped);
    todo_wine ok(bret == FALSE && WSAGetLastError() == WSAEINVAL, "AcceptEx on a non-listening socket "
        "returned %d + errno %d\n", bret, WSAGetLastError());

    iret = listen(listener, 5);
    if (iret != 0) {
        skip("listening failed, errno = %d\n", WSAGetLastError());
        goto end;
    }

    overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (overlapped.hEvent == NULL) {
        skip("could not create event object, errno = %d\n", GetLastError());
        goto end;
    }

    bret = pAcceptEx(listener, acceptor, buffer, 0,
        sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
        &bytesReturned, &overlapped);
    ok(bret == FALSE && WSAGetLastError() == ERROR_IO_PENDING, "AcceptEx returned %d + errno %d\n", bret, WSAGetLastError());

    bret = pAcceptEx(listener, acceptor, buffer, 0,
        sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
        &bytesReturned, &overlapped);
    todo_wine ok((bret == FALSE && WSAGetLastError() == WSAEINVAL) || broken(bret == FALSE && WSAGetLastError() == ERROR_IO_PENDING) /* NT4 */,
       "AcceptEx on already pending socket returned %d + errno %d\n", bret, WSAGetLastError());
    if (bret == FALSE && WSAGetLastError() == ERROR_IO_PENDING) {
        /* We need to cancel this call, otherwise things fail */
        bret = CancelIo((HANDLE) listener);
        ok(bret, "Failed to cancel failed test. Bailing...\n");
        if (!bret) return;
        WaitForSingleObject(overlapped.hEvent, 0);

        bret = pAcceptEx(listener, acceptor, buffer, 0,
            sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
            &bytesReturned, &overlapped);
        ok(bret == FALSE && WSAGetLastError() == ERROR_IO_PENDING, "AcceptEx returned %d + errno %d\n", bret, WSAGetLastError());
    }

    iret = connect(acceptor,  (struct sockaddr*)&bindAddress, sizeof(bindAddress));
    todo_wine ok((iret == SOCKET_ERROR && WSAGetLastError() == WSAEINVAL) || broken(!iret) /* NT4 */,
       "connecting to acceptex acceptor succeeded? return %d + errno %d\n", iret, WSAGetLastError());
    if (!iret || (iret == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)) {
        /* We need to cancel this call, otherwise things fail */
        closesocket(acceptor);
        acceptor = socket(AF_INET, SOCK_STREAM, 0);
        if (acceptor == INVALID_SOCKET) {
            skip("could not create acceptor socket, error %d\n", WSAGetLastError());
            goto end;
        }

        bret = CancelIo((HANDLE) listener);
        ok(bret, "Failed to cancel failed test. Bailing...\n");
        if (!bret) return;

        bret = pAcceptEx(listener, acceptor, buffer, 0,
            sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
            &bytesReturned, &overlapped);
        ok(bret == FALSE && WSAGetLastError() == ERROR_IO_PENDING, "AcceptEx returned %d + errno %d\n", bret, WSAGetLastError());
    }

    iret = connect(connector, (struct sockaddr*)&bindAddress, sizeof(bindAddress));
    ok(iret == 0, "connecting to accepting socket failed, error %d\n", WSAGetLastError());

    dwret = WaitForSingleObject(overlapped.hEvent, INFINITE);
    ok(dwret == WAIT_OBJECT_0, "Waiting for accept event failed with %d + errno %d\n", dwret, GetLastError());

    bret = GetOverlappedResult((HANDLE)listener, &overlapped, &bytesReturned, FALSE);
    ok(bret, "GetOverlappedResult failed, error %d\n", GetLastError());
    ok(bytesReturned == 0, "bytesReturned isn't supposed to be %d\n", bytesReturned);

    closesocket(connector);
    connector = INVALID_SOCKET;
    closesocket(acceptor);

    /* Test short reads */

    acceptor = socket(AF_INET, SOCK_STREAM, 0);
    if (acceptor == INVALID_SOCKET) {
        skip("could not create acceptor socket, error %d\n", WSAGetLastError());
        goto end;
    }
    connector = socket(AF_INET, SOCK_STREAM, 0);
    if (connector == INVALID_SOCKET) {
        skip("could not create connector socket, error %d\n", WSAGetLastError());
        goto end;
    }
    bret = pAcceptEx(listener, acceptor, buffer, 2,
        sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
        &bytesReturned, &overlapped);
    ok(bret == FALSE && WSAGetLastError() == ERROR_IO_PENDING, "AcceptEx returned %d + errno %d\n", bret, WSAGetLastError());

    connect_time = 0xdeadbeef;
    optlen = sizeof(connect_time);
    iret = getsockopt(connector, SOL_SOCKET, SO_CONNECT_TIME, (char *)&connect_time, &optlen);
    ok(!iret, "getsockopt failed %d\n", WSAGetLastError());
    ok(connect_time == ~0u, "unexpected connect time %u\n", connect_time);

    iret = connect(connector, (struct sockaddr*)&bindAddress, sizeof(bindAddress));
    ok(iret == 0, "connecting to accepting socket failed, error %d\n", WSAGetLastError());

    connect_time = 0xdeadbeef;
    optlen = sizeof(connect_time);
    iret = getsockopt(connector, SOL_SOCKET, SO_CONNECT_TIME, (char *)&connect_time, &optlen);
    ok(!iret, "getsockopt failed %d\n", WSAGetLastError());
    ok(connect_time < 0xdeadbeef, "unexpected connect time %u\n", connect_time);

    dwret = WaitForSingleObject(overlapped.hEvent, 0);
    ok(dwret == WAIT_TIMEOUT, "Waiting for accept event timeout failed with %d + errno %d\n", dwret, GetLastError());

    iret = getsockname( connector, (struct sockaddr *)&peerAddress, &remoteSize);
    ok( !iret, "getsockname failed.\n");

    /* Check if the buffer from AcceptEx is decoded correclty */
    pGetAcceptExSockaddrs(buffer, 2, sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
                          (struct sockaddr **)&readBindAddress, &localSize,
                          (struct sockaddr **)&readRemoteAddress, &remoteSize);
    strcpy( ipbuffer, inet_ntoa(readBindAddress->sin_addr));
    ok( readBindAddress->sin_addr.s_addr == bindAddress.sin_addr.s_addr,
            "Local socket address is different %s != %s\n",
            ipbuffer, inet_ntoa(bindAddress.sin_addr));
    ok( readBindAddress->sin_port == bindAddress.sin_port,
            "Local socket port is different: %d != %d\n",
            readBindAddress->sin_port, bindAddress.sin_port);
    strcpy( ipbuffer, inet_ntoa(readRemoteAddress->sin_addr));
    ok( readRemoteAddress->sin_addr.s_addr == peerAddress.sin_addr.s_addr,
            "Remote socket address is different %s != %s\n",
            ipbuffer, inet_ntoa(peerAddress.sin_addr));
    ok( readRemoteAddress->sin_port == peerAddress.sin_port,
            "Remote socket port is different: %d != %d\n",
            readRemoteAddress->sin_port, peerAddress.sin_port);

    iret = send(connector, buffer, 1, 0);
    ok(iret == 1, "could not send 1 byte: send %d errno %d\n", iret, WSAGetLastError());

    dwret = WaitForSingleObject(overlapped.hEvent, 1000);
    ok(dwret == WAIT_OBJECT_0, "Waiting for accept event failed with %d + errno %d\n", dwret, GetLastError());

    bret = GetOverlappedResult((HANDLE)listener, &overlapped, &bytesReturned, FALSE);
    ok(bret, "GetOverlappedResult failed, error %d\n", GetLastError());
    ok(bytesReturned == 1, "bytesReturned isn't supposed to be %d\n", bytesReturned);

    closesocket(connector);
    connector = INVALID_SOCKET;
    closesocket(acceptor);

    /* Test CF_DEFER & AcceptEx interaction */

    acceptor = socket(AF_INET, SOCK_STREAM, 0);
    if (acceptor == INVALID_SOCKET) {
        skip("could not create acceptor socket, error %d\n", WSAGetLastError());
        goto end;
    }
    connector = socket(AF_INET, SOCK_STREAM, 0);
    if (connector == INVALID_SOCKET) {
        skip("could not create connector socket, error %d\n", WSAGetLastError());
        goto end;
    }
    connector2 = socket(AF_INET, SOCK_STREAM, 0);
    if (connector == INVALID_SOCKET) {
        skip("could not create connector socket, error %d\n", WSAGetLastError());
        goto end;
    }

    if (set_blocking(connector, FALSE)) {
        skip("couldn't make socket non-blocking, error %d\n", WSAGetLastError());
        goto end;
    }

    if (set_blocking(connector2, FALSE)) {
        skip("couldn't make socket non-blocking, error %d\n", WSAGetLastError());
        goto end;
    }

    /* Connect socket #1 */
    iret = connect(connector, (struct sockaddr*)&bindAddress, sizeof(bindAddress));
    ok(iret == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK, "connecting to accepting socket failed, error %d\n", WSAGetLastError());

    FD_ZERO ( &fds_accept );
    FD_ZERO ( &fds_send );

    FD_SET ( listener, &fds_accept );
    FD_SET ( connector, &fds_send );

    buffer[0] = '0';
    got = 0;
    conn1 = 0;

    for (i = 0; i < 4000; ++i)
    {
        fd_set fds_openaccept = fds_accept, fds_opensend = fds_send;

        wsa_ok ( ( select ( 0, &fds_openaccept, &fds_opensend, NULL, &timeout ) ), SOCKET_ERROR !=,
            "acceptex test(%d): could not select on socket, errno %d\n" );

        /* check for incoming requests */
        if ( FD_ISSET ( listener, &fds_openaccept ) ) {
            got++;
            if (got == 1) {
                SOCKET tmp = WSAAccept(listener, NULL, NULL, (LPCONDITIONPROC) AlwaysDeferConditionFunc, 0);
                ok(tmp == INVALID_SOCKET && WSAGetLastError() == WSATRY_AGAIN, "Failed to defer connection, %d\n", WSAGetLastError());
                bret = pAcceptEx(listener, acceptor, buffer, 0,
                                    sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
                                    &bytesReturned, &overlapped);
                ok(bret == FALSE && WSAGetLastError() == ERROR_IO_PENDING, "AcceptEx returned %d + errno %d\n", bret, WSAGetLastError());
            }
            else if (got == 2) {
                /* this should be socket #2 */
                SOCKET tmp = accept(listener, NULL, NULL);
                ok(tmp != INVALID_SOCKET, "accept failed %d\n", WSAGetLastError());
                closesocket(tmp);
            }
            else {
                ok(FALSE, "Got more than 2 connections?\n");
            }
        }
        if ( conn1 && FD_ISSET ( connector2, &fds_opensend ) ) {
            /* Send data on second socket, and stop */
            send(connector2, "2", 1, 0);
            FD_CLR ( connector2, &fds_send );

            break;
        }
        if ( FD_ISSET ( connector, &fds_opensend ) ) {
            /* Once #1 is connected, allow #2 to connect */
            conn1 = 1;

            send(connector, "1", 1, 0);
            FD_CLR ( connector, &fds_send );

            iret = connect(connector2, (struct sockaddr*)&bindAddress, sizeof(bindAddress));
            ok(iret == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK, "connecting to accepting socket failed, error %d\n", WSAGetLastError());
            FD_SET ( connector2, &fds_send );
        }
    }

    ok (got == 2 || broken(got == 1) /* NT4 */,
            "Did not get both connections, got %d\n", got);

    dwret = WaitForSingleObject(overlapped.hEvent, 0);
    ok(dwret == WAIT_OBJECT_0, "Waiting for accept event failed with %d + errno %d\n", dwret, GetLastError());

    bret = GetOverlappedResult((HANDLE)listener, &overlapped, &bytesReturned, FALSE);
    ok(bret, "GetOverlappedResult failed, error %d\n", GetLastError());
    ok(bytesReturned == 0, "bytesReturned isn't supposed to be %d\n", bytesReturned);

    set_blocking(acceptor, TRUE);
    iret = recv( acceptor, buffer, 2, 0);
    ok(iret == 1, "Failed to get data, %d, errno: %d\n", iret, WSAGetLastError());

    ok(buffer[0] == '1', "The wrong first client was accepted by acceptex: %c != 1\n", buffer[0]);

    closesocket(connector);
    connector = INVALID_SOCKET;
    closesocket(acceptor);

    /* clean up in case of failures */
    while ((acceptor = accept(listener, NULL, NULL)) != INVALID_SOCKET)
        closesocket(acceptor);

    /* Disconnect during receive? */

    acceptor = socket(AF_INET, SOCK_STREAM, 0);
    if (acceptor == INVALID_SOCKET) {
        skip("could not create acceptor socket, error %d\n", WSAGetLastError());
        goto end;
    }
    connector = socket(AF_INET, SOCK_STREAM, 0);
    if (connector == INVALID_SOCKET) {
        skip("could not create connector socket, error %d\n", WSAGetLastError());
        goto end;
    }
    bret = pAcceptEx(listener, acceptor, buffer, sizeof(buffer) - 2*(sizeof(struct sockaddr_in) + 16),
        sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
        &bytesReturned, &overlapped);
    ok(bret == FALSE && WSAGetLastError() == ERROR_IO_PENDING, "AcceptEx returned %d + errno %d\n", bret, WSAGetLastError());

    iret = connect(connector, (struct sockaddr*)&bindAddress, sizeof(bindAddress));
    ok(iret == 0, "connecting to accepting socket failed, error %d\n", WSAGetLastError());

    closesocket(connector);
    connector = INVALID_SOCKET;

    dwret = WaitForSingleObject(overlapped.hEvent, 1000);
    ok(dwret == WAIT_OBJECT_0, "Waiting for accept event failed with %d + errno %d\n", dwret, GetLastError());

    bret = GetOverlappedResult((HANDLE)listener, &overlapped, &bytesReturned, FALSE);
    ok(bret, "GetOverlappedResult failed, error %d\n", GetLastError());
    ok(bytesReturned == 0, "bytesReturned isn't supposed to be %d\n", bytesReturned);

    closesocket(acceptor);

    /* Test closing with pending requests */

    acceptor = socket(AF_INET, SOCK_STREAM, 0);
    if (acceptor == INVALID_SOCKET) {
        skip("could not create acceptor socket, error %d\n", WSAGetLastError());
        goto end;
    }
    bret = pAcceptEx(listener, acceptor, buffer, sizeof(buffer) - 2*(sizeof(struct sockaddr_in) + 16),
        sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
        &bytesReturned, &overlapped);
    ok(bret == FALSE && WSAGetLastError() == ERROR_IO_PENDING, "AcceptEx returned %d + errno %d\n", bret, WSAGetLastError());

    closesocket(acceptor);

    dwret = WaitForSingleObject(overlapped.hEvent, 1000);
    todo_wine ok(dwret == WAIT_OBJECT_0 || broken(dwret == WAIT_TIMEOUT) /* NT4/2000 */,
       "Waiting for accept event failed with %d + errno %d\n", dwret, GetLastError());

    if (dwret != WAIT_TIMEOUT) {
        bret = GetOverlappedResult((HANDLE)listener, &overlapped, &bytesReturned, FALSE);
        ok(!bret && GetLastError() == ERROR_OPERATION_ABORTED, "GetOverlappedResult failed, error %d\n", GetLastError());
    }
    else {
        bret = CancelIo((HANDLE) listener);
        ok(bret, "Failed to cancel failed test. Bailing...\n");
        if (!bret) return;
        WaitForSingleObject(overlapped.hEvent, 0);
    }

    acceptor = socket(AF_INET, SOCK_STREAM, 0);
    if (acceptor == INVALID_SOCKET) {
        skip("could not create acceptor socket, error %d\n", WSAGetLastError());
        goto end;
    }
    bret = pAcceptEx(listener, acceptor, buffer, sizeof(buffer) - 2*(sizeof(struct sockaddr_in) + 16),
        sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
        &bytesReturned, &overlapped);
    ok(bret == FALSE && WSAGetLastError() == ERROR_IO_PENDING, "AcceptEx returned %d + errno %d\n", bret, WSAGetLastError());

    CancelIo((HANDLE) acceptor);

    dwret = WaitForSingleObject(overlapped.hEvent, 1000);
    ok(dwret == WAIT_TIMEOUT, "Waiting for timeout failed with %d + errno %d\n", dwret, GetLastError());

    closesocket(acceptor);

    acceptor = socket(AF_INET, SOCK_STREAM, 0);
    if (acceptor == INVALID_SOCKET) {
        skip("could not create acceptor socket, error %d\n", WSAGetLastError());
        goto end;
    }
    bret = pAcceptEx(listener, acceptor, buffer, sizeof(buffer) - 2*(sizeof(struct sockaddr_in) + 16),
        sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
        &bytesReturned, &overlapped);
    ok(bret == FALSE && WSAGetLastError() == ERROR_IO_PENDING, "AcceptEx returned %d + errno %d\n", bret, WSAGetLastError());

    closesocket(listener);
    listener = INVALID_SOCKET;

    dwret = WaitForSingleObject(overlapped.hEvent, 1000);
    ok(dwret == WAIT_OBJECT_0, "Waiting for accept event failed with %d + errno %d\n", dwret, GetLastError());

    bret = GetOverlappedResult((HANDLE)listener, &overlapped, &bytesReturned, FALSE);
    ok(!bret && GetLastError() == ERROR_OPERATION_ABORTED, "GetOverlappedResult failed, error %d\n", GetLastError());

end:
    if (overlapped.hEvent)
        WSACloseEvent(overlapped.hEvent);
    if (listener != INVALID_SOCKET)
        closesocket(listener);
    if (acceptor != INVALID_SOCKET)
        closesocket(acceptor);
    if (connector != INVALID_SOCKET)
        closesocket(connector);
    if (connector2 != INVALID_SOCKET)
        closesocket(connector2);
}

static void test_getpeername(void)
{
    SOCKET sock;
    struct sockaddr_in sa, sa_out;
    int sa_len;
    const char buf[] = "hello world";
    int ret;

    /* Test the parameter validation order. */
    ret = getpeername(INVALID_SOCKET, NULL, NULL);
    ok(ret == SOCKET_ERROR, "Expected getpeername to return SOCKET_ERROR, got %d\n", ret);
    ok(WSAGetLastError() == WSAENOTSOCK,
       "Expected WSAGetLastError() to return WSAENOTSOCK, got %d\n", WSAGetLastError());

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    ok(sock != INVALID_SOCKET, "Expected socket to return a valid socket\n");
    if (sock == INVALID_SOCKET)
    {
        skip("Socket creation failed with %d\n", WSAGetLastError());
        return;
    }

    ret = getpeername(sock, NULL, NULL);
    ok(ret == SOCKET_ERROR, "Expected getpeername to return SOCKET_ERROR, got %d\n", ret);
    ok(WSAGetLastError() == WSAENOTCONN ||
       broken(WSAGetLastError() == WSAEFAULT), /* Win9x and WinMe */
       "Expected WSAGetLastError() to return WSAENOTCONN, got %d\n", WSAGetLastError());

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(139);
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* sendto does not change a socket's connection state. */
    ret = sendto(sock, buf, sizeof(buf), 0, (struct sockaddr*)&sa, sizeof(sa));
    ok(ret != SOCKET_ERROR,
       "Expected sendto to succeed, WSAGetLastError() = %d\n", WSAGetLastError());

    ret = getpeername(sock, NULL, NULL);
    ok(ret == SOCKET_ERROR, "Expected getpeername to return SOCKET_ERROR, got %d\n", ret);
    ok(WSAGetLastError() == WSAENOTCONN ||
       broken(WSAGetLastError() == WSAEFAULT), /* Win9x and WinMe */
       "Expected WSAGetLastError() to return WSAENOTCONN, got %d\n", WSAGetLastError());

    ret = connect(sock, (struct sockaddr*)&sa, sizeof(sa));
    ok(ret == 0,
       "Expected connect to succeed, WSAGetLastError() = %d\n", WSAGetLastError());

    ret = getpeername(sock, NULL, NULL);
    ok(ret == SOCKET_ERROR, "Expected getpeername to return SOCKET_ERROR, got %d\n", ret);
    ok(WSAGetLastError() == WSAEFAULT,
       "Expected WSAGetLastError() to return WSAEFAULT, got %d\n", WSAGetLastError());

    /* Test crashes on Wine. */
    if (0)
    {
        ret = getpeername(sock, (void*)0xdeadbeef, (void*)0xcafebabe);
        ok(ret == SOCKET_ERROR, "Expected getpeername to return SOCKET_ERROR, got %d\n", ret);
        ok(WSAGetLastError() == WSAEFAULT,
           "Expected WSAGetLastError() to return WSAEFAULT, got %d\n", WSAGetLastError());
    }

    sa_len = 0;
    ret = getpeername(sock, (struct sockaddr*)&sa_out, &sa_len);
    ok(ret == SOCKET_ERROR, "Expected getpeername to return 0, got %d\n", ret);
    ok(WSAGetLastError() == WSAEFAULT,
       "Expected WSAGetLastError() to return WSAEFAULT, got %d\n", WSAGetLastError());

    sa_len = sizeof(sa_out);
    ret = getpeername(sock, (struct sockaddr*)&sa_out, &sa_len);
    ok(ret == 0, "Expected getpeername to return 0, got %d\n", ret);
    ok(!memcmp(&sa, &sa_out, sizeof(sa)),
       "Expected the returned structure to be identical to the connect structure\n");

    closesocket(sock);
}

static void test_sioRoutingInterfaceQuery(void)
{
    int ret;
    SOCKET sock;
    SOCKADDR_IN sin = { 0 }, sout = { 0 };
    DWORD bytesReturned;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    ok(sock != INVALID_SOCKET, "Expected socket to return a valid socket\n");
    if (sock == INVALID_SOCKET)
    {
        skip("Socket creation failed with %d\n", WSAGetLastError());
        return;
    }
    ret = WSAIoctl(sock, SIO_ROUTING_INTERFACE_QUERY, NULL, 0, NULL, 0, NULL,
                   NULL, NULL);
    ok(ret == SOCKET_ERROR && WSAGetLastError() == WSAEFAULT,
       "expected WSAEFAULT, got %d\n", WSAGetLastError());
    ret = WSAIoctl(sock, SIO_ROUTING_INTERFACE_QUERY, &sin, sizeof(sin),
                   NULL, 0, NULL, NULL, NULL);
    ok(ret == SOCKET_ERROR && WSAGetLastError() == WSAEFAULT,
       "expected WSAEFAULT, got %d\n", WSAGetLastError());
    ret = WSAIoctl(sock, SIO_ROUTING_INTERFACE_QUERY, &sin, sizeof(sin),
                   NULL, 0, &bytesReturned, NULL, NULL);
    ok(ret == SOCKET_ERROR &&
       (WSAGetLastError() == WSAEFAULT /* Win98 */ ||
        WSAGetLastError() == WSAEINVAL /* NT4 */||
        WSAGetLastError() == WSAEAFNOSUPPORT),
       "expected WSAEFAULT or WSAEINVAL or WSAEAFNOSUPPORT, got %d\n",
       WSAGetLastError());
    sin.sin_family = AF_INET;
    ret = WSAIoctl(sock, SIO_ROUTING_INTERFACE_QUERY, &sin, sizeof(sin),
                   NULL, 0, &bytesReturned, NULL, NULL);
    ok(ret == SOCKET_ERROR &&
       (WSAGetLastError() == WSAEFAULT /* Win98 */ ||
        WSAGetLastError() == WSAEINVAL),
       "expected WSAEFAULT or WSAEINVAL, got %d\n", WSAGetLastError());
    sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ret = WSAIoctl(sock, SIO_ROUTING_INTERFACE_QUERY, &sin, sizeof(sin),
                   NULL, 0, &bytesReturned, NULL, NULL);
    ok(ret == SOCKET_ERROR &&
       (WSAGetLastError() == WSAEINVAL /* NT4 */ ||
        WSAGetLastError() == WSAEFAULT),
       "expected WSAEINVAL or WSAEFAULT, got %d\n", WSAGetLastError());
    ret = WSAIoctl(sock, SIO_ROUTING_INTERFACE_QUERY, &sin, sizeof(sin),
                   &sout, sizeof(sout), &bytesReturned, NULL, NULL);
    ok(!ret || broken(WSAGetLastError() == WSAEINVAL /* NT4 */),
       "WSAIoctl failed: %d\n", WSAGetLastError());
    if (!ret)
    {
        ok(sout.sin_family == AF_INET, "expected AF_INET, got %d\n",
           sout.sin_family);
        /* We expect the source address to be INADDR_LOOPBACK as well, but
         * there's no guarantee that a route to the loopback address exists,
         * so rather than introduce spurious test failures we do not test the
         * source address.
         */
    }
    closesocket(sock);
}

static void test_synchronous_WSAIoctl(void)
{
    HANDLE previous_port, io_port;
    WSAOVERLAPPED overlapped, *olp;
    SOCKET socket;
    ULONG on;
    ULONG_PTR key;
    DWORD num_bytes;
    BOOL ret;
    int res;

    previous_port = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );
    ok( previous_port != NULL, "failed to create completion port %u\n", GetLastError() );

    socket = WSASocketW( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED );
    ok( socket != INVALID_SOCKET, "failed to create socket %d\n", WSAGetLastError() );

    io_port = CreateIoCompletionPort( (HANDLE)socket, previous_port, 0, 0 );
    ok( io_port != NULL, "failed to create completion port %u\n", GetLastError() );

    on = 1;
    memset( &overlapped, 0, sizeof(overlapped) );
    res = WSAIoctl( socket, FIONBIO, &on, sizeof(on), NULL, 0, &num_bytes, &overlapped, NULL );
    ok( !res, "WSAIoctl failed %d\n", WSAGetLastError() );

    ret = GetQueuedCompletionStatus( io_port, &num_bytes, &key, &olp, 10000 );
    ok( ret, "failed to get completion status %u\n", GetLastError() );

    CloseHandle( io_port );
    closesocket( socket );
    CloseHandle( previous_port );
}

#define WM_ASYNCCOMPLETE (WM_USER + 100)
static HWND create_async_message_window(void)
{
    static const char class_name[] = "ws2_32 async message window class";

    WNDCLASSEXA wndclass;
    HWND hWnd;

    wndclass.cbSize         = sizeof(wndclass);
    wndclass.style          = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc    = DefWindowProcA;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = GetModuleHandleA(NULL);
    wndclass.hIcon          = LoadIconA(NULL, IDI_APPLICATION);
    wndclass.hIconSm        = LoadIconA(NULL, IDI_APPLICATION);
    wndclass.hCursor        = LoadCursorA(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wndclass.lpszClassName  = class_name;
    wndclass.lpszMenuName   = NULL;

    RegisterClassExA(&wndclass);

    hWnd = CreateWindow(class_name, "ws2_32 async message window", WS_OVERLAPPEDWINDOW,
                        0, 0, 500, 500, NULL, NULL, GetModuleHandleA(NULL), NULL);
    if (!hWnd)
    {
        ok(0, "failed to create window: %u\n", GetLastError());
        return NULL;
    }

    return hWnd;
}

static void test_WSAAsyncGetServByPort(void)
{
    HWND hwnd = create_async_message_window();
    HANDLE ret;
    char buffer[MAXGETHOSTSTRUCT];

    if (!hwnd)
        return;

    /* FIXME: The asynchronous window messages should be tested. */

    /* Parameters are not checked when initiating the asynchronous operation.  */
    ret = WSAAsyncGetServByPort(NULL, 0, 0, NULL, NULL, 0);
    ok(ret != NULL, "WSAAsyncGetServByPort returned NULL\n");

    ret = WSAAsyncGetServByPort(hwnd, WM_ASYNCCOMPLETE, 0, NULL, NULL, 0);
    ok(ret != NULL, "WSAAsyncGetServByPort returned NULL\n");

    ret = WSAAsyncGetServByPort(hwnd, WM_ASYNCCOMPLETE, htons(80), NULL, NULL, 0);
    ok(ret != NULL, "WSAAsyncGetServByPort returned NULL\n");

    ret = WSAAsyncGetServByPort(hwnd, WM_ASYNCCOMPLETE, htons(80), NULL, buffer, MAXGETHOSTSTRUCT);
    ok(ret != NULL, "WSAAsyncGetServByPort returned NULL\n");

    DestroyWindow(hwnd);
}

static void test_WSAAsyncGetServByName(void)
{
    HWND hwnd = create_async_message_window();
    HANDLE ret;
    char buffer[MAXGETHOSTSTRUCT];

    if (!hwnd)
        return;

    /* FIXME: The asynchronous window messages should be tested. */

    /* Parameters are not checked when initiating the asynchronous operation.  */
    ret = WSAAsyncGetServByName(hwnd, WM_ASYNCCOMPLETE, "", NULL, NULL, 0);
    ok(ret != NULL, "WSAAsyncGetServByName returned NULL\n");

    ret = WSAAsyncGetServByName(hwnd, WM_ASYNCCOMPLETE, "", "", buffer, MAXGETHOSTSTRUCT);
    ok(ret != NULL, "WSAAsyncGetServByName returned NULL\n");

    ret = WSAAsyncGetServByName(hwnd, WM_ASYNCCOMPLETE, "http", NULL, NULL, 0);
    ok(ret != NULL, "WSAAsyncGetServByName returned NULL\n");

    ret = WSAAsyncGetServByName(hwnd, WM_ASYNCCOMPLETE, "http", "tcp", buffer, MAXGETHOSTSTRUCT);
    ok(ret != NULL, "WSAAsyncGetServByName returned NULL\n");

    DestroyWindow(hwnd);
}

/*
 * Provide consistent initialization for the AcceptEx IOCP tests.
 */
static SOCKET setup_iocp_src(struct sockaddr_in *bindAddress)
{
    SOCKET src, ret = INVALID_SOCKET;
    int iret, socklen;

    src = socket(AF_INET, SOCK_STREAM, 0);
    if (src == INVALID_SOCKET)
    {
        skip("could not create listener socket, error %d\n", WSAGetLastError());
        goto end;
    }

    memset(bindAddress, 0, sizeof(*bindAddress));
    bindAddress->sin_family = AF_INET;
    bindAddress->sin_addr.s_addr = inet_addr("127.0.0.1");
    iret = bind(src, (struct sockaddr*)bindAddress, sizeof(*bindAddress));
    if (iret != 0)
    {
        skip("failed to bind, error %d\n", WSAGetLastError());
        goto end;
    }

    socklen = sizeof(*bindAddress);
    iret = getsockname(src, (struct sockaddr*)bindAddress, &socklen);
    if (iret != 0) {
        skip("failed to lookup bind address, error %d\n", WSAGetLastError());
        goto end;
    }

    if (set_blocking(src, FALSE))
    {
        skip("couldn't make socket non-blocking, error %d\n", WSAGetLastError());
        goto end;
    }

    iret = listen(src, 5);
    if (iret != 0)
    {
        skip("listening failed, errno = %d\n", WSAGetLastError());
        goto end;
    }

    ret = src;
end:
    if (src != ret && ret == INVALID_SOCKET)
        closesocket(src);
    return ret;
}

static void test_completion_port(void)
{
    HANDLE previous_port, io_port;
    WSAOVERLAPPED ov, *olp;
    SOCKET src, dest, dup, connector = INVALID_SOCKET;
    WSAPROTOCOL_INFOA info;
    char buf[1024];
    WSABUF bufs;
    DWORD num_bytes, flags;
    struct linger ling;
    int iret;
    BOOL bret;
    ULONG_PTR key;
    struct sockaddr_in bindAddress;
    GUID acceptExGuid = WSAID_ACCEPTEX;
    LPFN_ACCEPTEX pAcceptEx = NULL;

    previous_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    ok( previous_port != NULL, "Failed to create completion port %u\n", GetLastError());

    memset(&ov, 0, sizeof(ov));

    tcp_socketpair(&src, &dest);
    if (src == INVALID_SOCKET || dest == INVALID_SOCKET)
    {
        skip("failed to create sockets\n");
        goto end;
    }

    bufs.len = sizeof(buf);
    bufs.buf = buf;
    flags = 0;

    ling.l_onoff = 1;
    ling.l_linger = 0;
    iret = setsockopt (src, SOL_SOCKET, SO_LINGER, (char *) &ling, sizeof(ling));
    ok(!iret, "Failed to set linger %d\n", GetLastError());

    io_port = CreateIoCompletionPort( (HANDLE)dest, previous_port, 125, 0 );
    ok(io_port != NULL, "Failed to create completion port %u\n", GetLastError());

    SetLastError(0xdeadbeef);

    iret = WSARecv(dest, &bufs, 1, &num_bytes, &flags, &ov, NULL);
    ok(iret == SOCKET_ERROR, "WSARecv returned %d\n", iret);
    ok(GetLastError() == ERROR_IO_PENDING, "Last error was %d\n", GetLastError());

    closesocket(src);
    src = INVALID_SOCKET;

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;

    bret = GetQueuedCompletionStatus(io_port, &num_bytes, &key, &olp, 100);
    todo_wine ok(bret == FALSE, "GetQueuedCompletionStatus returned %d\n", bret);
    todo_wine ok(GetLastError() == ERROR_NETNAME_DELETED, "Last error was %d\n", GetLastError());
    ok(key == 125, "Key is %lu\n", key);
    ok(num_bytes == 0, "Number of bytes received is %u\n", num_bytes);
    ok(olp == &ov, "Overlapped structure is at %p\n", olp);

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;

    bret = GetQueuedCompletionStatus(io_port, &num_bytes, &key, &olp, 100);
    ok(bret == FALSE, "GetQueuedCompletionStatus returned %d\n", bret );
    ok(GetLastError() == WAIT_TIMEOUT, "Last error was %d\n", GetLastError());
    ok(key == 0xdeadbeef, "Key is %lu\n", key);
    ok(num_bytes == 0xdeadbeef, "Number of bytes transferred is %u\n", num_bytes);
    ok(!olp, "Overlapped structure is at %p\n", olp);

    if (dest != INVALID_SOCKET)
        closesocket(dest);
    if (src != INVALID_SOCKET)
        closesocket(src);

    memset(&ov, 0, sizeof(ov));

    tcp_socketpair(&src, &dest);
    if (src == INVALID_SOCKET || dest == INVALID_SOCKET)
    {
        skip("failed to create sockets\n");
        goto end;
    }

    bufs.len = sizeof(buf);
    bufs.buf = buf;
    flags = 0;

    ling.l_onoff = 1;
    ling.l_linger = 0;
    iret = setsockopt (src, SOL_SOCKET, SO_LINGER, (char *) &ling, sizeof(ling));
    ok(!iret, "Failed to set linger %d\n", GetLastError());

    io_port = CreateIoCompletionPort((HANDLE)dest, previous_port, 125, 0);
    ok(io_port != NULL, "failed to create completion port %u\n", GetLastError());

    set_blocking(dest, FALSE);

    closesocket(src);
    src = INVALID_SOCKET;
    num_bytes = 0xdeadbeef;
    SetLastError(0xdeadbeef);

    iret = WSASend(dest, &bufs, 1, &num_bytes, 0, &ov, NULL);
    ok(iret == SOCKET_ERROR, "WSASend failed - %d\n", iret);
    ok(GetLastError() == WSAECONNRESET, "Last error was %d\n", GetLastError());
    ok(num_bytes == 0xdeadbeef, "Managed to send %d\n", num_bytes);

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;

    bret = GetQueuedCompletionStatus( io_port, &num_bytes, &key, &olp, 200 );
    ok(bret == FALSE, "GetQueuedCompletionStatus returned %u\n", bret );
    ok(GetLastError() == WAIT_TIMEOUT, "Last error was %d\n", GetLastError());
    ok(key == 0xdeadbeef, "Key is %lu\n", key);
    ok(num_bytes == 0xdeadbeef, "Number of bytes transferred is %u\n", num_bytes);
    ok(!olp, "Overlapped structure is at %p\n", olp);

    if (dest != INVALID_SOCKET)
        closesocket(dest);

    dest = socket(AF_INET, SOCK_STREAM, 0);
    if (dest == INVALID_SOCKET)
    {
        skip("could not create acceptor socket, error %d\n", WSAGetLastError());
        goto end;
    }

    iret = WSAIoctl(dest, SIO_GET_EXTENSION_FUNCTION_POINTER, &acceptExGuid, sizeof(acceptExGuid),
            &pAcceptEx, sizeof(pAcceptEx), &num_bytes, NULL, NULL);
    if (iret)
    {
        skip("WSAIoctl failed to get AcceptEx with ret %d + errno %d\n", iret, WSAGetLastError());
        goto end;
    }

    /* Test IOCP response on socket close (IOCP created after AcceptEx) */

    if ((src = setup_iocp_src(&bindAddress)) == INVALID_SOCKET)
        goto end;

    SetLastError(0xdeadbeef);

    bret = pAcceptEx(src, dest, buf, sizeof(buf) - 2*(sizeof(struct sockaddr_in) + 16),
            sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
            &num_bytes, &ov);
    ok(bret == FALSE, "AcceptEx returned %d\n", bret);
    ok(GetLastError() == ERROR_IO_PENDING, "Last error was %d\n", GetLastError());

    io_port = CreateIoCompletionPort((HANDLE)src, previous_port, 125, 0);
    ok(io_port != NULL, "failed to create completion port %u\n", GetLastError());

    closesocket(src);
    src = INVALID_SOCKET;

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;

    bret = GetQueuedCompletionStatus(io_port, &num_bytes, &key, &olp, 100);
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    todo_wine ok(GetLastError() == ERROR_OPERATION_ABORTED, "Last error was %d\n", GetLastError());
    todo_wine ok(key == 125, "Key is %lu\n", key);
    todo_wine ok(num_bytes == 0, "Number of bytes transferred is %u\n", num_bytes);
    todo_wine ok(olp == &ov, "Overlapped structure is at %p\n", olp);
    todo_wine ok(olp && (olp->Internal == (ULONG)STATUS_CANCELLED), "Internal status is %lx\n", olp ? olp->Internal : 0);

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;
    bret = GetQueuedCompletionStatus( io_port, &num_bytes, &key, &olp, 200 );
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    ok(GetLastError() == WAIT_TIMEOUT, "Last error was %d\n", GetLastError());
    ok(key == 0xdeadbeef, "Key is %lu\n", key);
    ok(num_bytes == 0xdeadbeef, "Number of bytes transferred is %u\n", num_bytes);
    ok(!olp, "Overlapped structure is at %p\n", olp);

    /* Test IOCP response on socket close (IOCP created before AcceptEx) */

    if ((src = setup_iocp_src(&bindAddress)) == INVALID_SOCKET)
        goto end;

    SetLastError(0xdeadbeef);

    io_port = CreateIoCompletionPort((HANDLE)src, previous_port, 125, 0);
    ok(io_port != NULL, "failed to create completion port %u\n", GetLastError());

    bret = pAcceptEx(src, dest, buf, sizeof(buf) - 2*(sizeof(struct sockaddr_in) + 16),
            sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
            &num_bytes, &ov);
    ok(bret == FALSE, "AcceptEx returned %d\n", bret);
    ok(GetLastError() == ERROR_IO_PENDING, "Last error was %d\n", GetLastError());

    closesocket(src);
    src = INVALID_SOCKET;

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;

    bret = GetQueuedCompletionStatus(io_port, &num_bytes, &key, &olp, 100);
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    todo_wine ok(GetLastError() == ERROR_OPERATION_ABORTED, "Last error was %d\n", GetLastError());
    todo_wine ok(key == 125, "Key is %lu\n", key);
    todo_wine ok(num_bytes == 0, "Number of bytes transferred is %u\n", num_bytes);
    todo_wine ok(olp == &ov, "Overlapped structure is at %p\n", olp);
    todo_wine ok(olp && (olp->Internal == (ULONG)STATUS_CANCELLED), "Internal status is %lx\n", olp ? olp->Internal : 0);

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;
    bret = GetQueuedCompletionStatus( io_port, &num_bytes, &key, &olp, 200 );
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    ok(GetLastError() == WAIT_TIMEOUT, "Last error was %d\n", GetLastError());
    ok(key == 0xdeadbeef, "Key is %lu\n", key);
    ok(num_bytes == 0xdeadbeef, "Number of bytes transferred is %u\n", num_bytes);
    ok(!olp, "Overlapped structure is at %p\n", olp);

    /* Test IOCP with duplicated handle */

    if ((src = setup_iocp_src(&bindAddress)) == INVALID_SOCKET)
        goto end;

    SetLastError(0xdeadbeef);

    io_port = CreateIoCompletionPort((HANDLE)src, previous_port, 125, 0);
    ok(io_port != NULL, "failed to create completion port %u\n", GetLastError());

    WSADuplicateSocket( src, GetCurrentProcessId(), &info );
    dup = WSASocket(AF_INET, SOCK_STREAM, 0, &info, 0, WSA_FLAG_OVERLAPPED);
    ok(dup != INVALID_SOCKET, "failed to duplicate socket!\n");

    bret = pAcceptEx(dup, dest, buf, sizeof(buf) - 2*(sizeof(struct sockaddr_in) + 16),
            sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
            &num_bytes, &ov);
    ok(bret == FALSE, "AcceptEx returned %d\n", bret);
    ok(GetLastError() == ERROR_IO_PENDING, "Last error was %d\n", GetLastError());

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;
    bret = GetQueuedCompletionStatus( io_port, &num_bytes, &key, &olp, 200 );
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    ok(GetLastError() == WAIT_TIMEOUT, "Last error was %d\n", GetLastError());
    ok(key == 0xdeadbeef, "Key is %lu\n", key);
    ok(num_bytes == 0xdeadbeef, "Number of bytes transferred is %u\n", num_bytes);
    ok(!olp, "Overlapped structure is at %p\n", olp);

    closesocket(src);
    src = INVALID_SOCKET;
    closesocket(dup);
    dup = INVALID_SOCKET;

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;
    bret = GetQueuedCompletionStatus(io_port, &num_bytes, &key, &olp, 100);
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    todo_wine ok(GetLastError() == ERROR_OPERATION_ABORTED, "Last error was %d\n", GetLastError());
    todo_wine ok(key == 125, "Key is %lu\n", key);
    todo_wine ok(num_bytes == 0, "Number of bytes transferred is %u\n", num_bytes);
    todo_wine ok(olp == &ov, "Overlapped structure is at %p\n", olp);
    todo_wine ok(olp && olp->Internal == (ULONG)STATUS_CANCELLED, "Internal status is %lx\n", olp ? olp->Internal : 0);

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;
    bret = GetQueuedCompletionStatus( io_port, &num_bytes, &key, &olp, 200 );
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    ok(GetLastError() == WAIT_TIMEOUT, "Last error was %d\n", GetLastError());
    ok(key == 0xdeadbeef, "Key is %lu\n", key);
    ok(num_bytes == 0xdeadbeef, "Number of bytes transferred is %u\n", num_bytes);
    ok(!olp, "Overlapped structure is at %p\n", olp);

    /* Test IOCP with duplicated handle (closing duplicated handle) */

    if ((src = setup_iocp_src(&bindAddress)) == INVALID_SOCKET)
        goto end;

    SetLastError(0xdeadbeef);

    io_port = CreateIoCompletionPort((HANDLE)src, previous_port, 125, 0);
    ok(io_port != NULL, "failed to create completion port %u\n", GetLastError());

    WSADuplicateSocket( src, GetCurrentProcessId(), &info );
    dup = WSASocket(AF_INET, SOCK_STREAM, 0, &info, 0, WSA_FLAG_OVERLAPPED);
    ok(dup != INVALID_SOCKET, "failed to duplicate socket!\n");

    bret = pAcceptEx(dup, dest, buf, sizeof(buf) - 2*(sizeof(struct sockaddr_in) + 16),
            sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
            &num_bytes, &ov);
    ok(bret == FALSE, "AcceptEx returned %d\n", bret);
    ok(GetLastError() == ERROR_IO_PENDING, "Last error was %d\n", GetLastError());

    closesocket(dup);
    dup = INVALID_SOCKET;

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;
    bret = GetQueuedCompletionStatus( io_port, &num_bytes, &key, &olp, 200 );
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    ok(GetLastError() == WAIT_TIMEOUT, "Last error was %d\n", GetLastError());
    ok(key == 0xdeadbeef, "Key is %lu\n", key);
    ok(num_bytes == 0xdeadbeef, "Number of bytes transferred is %u\n", num_bytes);
    ok(!olp, "Overlapped structure is at %p\n", olp);

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;
    bret = GetQueuedCompletionStatus( io_port, &num_bytes, &key, &olp, 200 );
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    ok(GetLastError() == WAIT_TIMEOUT, "Last error was %d\n", GetLastError());
    ok(key == 0xdeadbeef, "Key is %lu\n", key);
    ok(num_bytes == 0xdeadbeef, "Number of bytes transferred is %u\n", num_bytes);
    ok(!olp, "Overlapped structure is at %p\n", olp);

    closesocket(src);
    src = INVALID_SOCKET;

    bret = GetQueuedCompletionStatus(io_port, &num_bytes, &key, &olp, 100);
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    todo_wine ok(GetLastError() == ERROR_OPERATION_ABORTED, "Last error was %d\n", GetLastError());
    todo_wine ok(key == 125, "Key is %lu\n", key);
    todo_wine ok(num_bytes == 0, "Number of bytes transferred is %u\n", num_bytes);
    todo_wine ok(olp == &ov, "Overlapped structure is at %p\n", olp);
    todo_wine ok(olp && (olp->Internal == (ULONG)STATUS_CANCELLED), "Internal status is %lx\n", olp ? olp->Internal : 0);

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;
    bret = GetQueuedCompletionStatus( io_port, &num_bytes, &key, &olp, 200 );
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    ok(GetLastError() == WAIT_TIMEOUT, "Last error was %d\n", GetLastError());
    ok(key == 0xdeadbeef, "Key is %lu\n", key);
    ok(num_bytes == 0xdeadbeef, "Number of bytes transferred is %u\n", num_bytes);
    ok(!olp, "Overlapped structure is at %p\n", olp);

    /* Test IOCP with duplicated handle (closing original handle) */

    if ((src = setup_iocp_src(&bindAddress)) == INVALID_SOCKET)
        goto end;

    SetLastError(0xdeadbeef);

    io_port = CreateIoCompletionPort((HANDLE)src, previous_port, 125, 0);
    ok(io_port != NULL, "failed to create completion port %u\n", GetLastError());

    WSADuplicateSocket( src, GetCurrentProcessId(), &info );
    dup = WSASocket(AF_INET, SOCK_STREAM, 0, &info, 0, WSA_FLAG_OVERLAPPED);
    ok(dup != INVALID_SOCKET, "failed to duplicate socket!\n");

    bret = pAcceptEx(dup, dest, buf, sizeof(buf) - 2*(sizeof(struct sockaddr_in) + 16),
            sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
            &num_bytes, &ov);
    ok(bret == FALSE, "AcceptEx returned %d\n", bret);
    ok(GetLastError() == ERROR_IO_PENDING, "Last error was %d\n", GetLastError());

    closesocket(src);
    src = INVALID_SOCKET;

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;
    bret = GetQueuedCompletionStatus( io_port, &num_bytes, &key, &olp, 200 );
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    ok(GetLastError() == WAIT_TIMEOUT, "Last error was %d\n", GetLastError());
    ok(key == 0xdeadbeef, "Key is %lu\n", key);
    ok(num_bytes == 0xdeadbeef, "Number of bytes transferred is %u\n", num_bytes);
    ok(!olp, "Overlapped structure is at %p\n", olp);

    closesocket(dup);
    dup = INVALID_SOCKET;

    bret = GetQueuedCompletionStatus(io_port, &num_bytes, &key, &olp, 100);
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    todo_wine ok(GetLastError() == ERROR_OPERATION_ABORTED, "Last error was %d\n", GetLastError());
    todo_wine ok(key == 125, "Key is %lu\n", key);
    todo_wine ok(num_bytes == 0, "Number of bytes transferred is %u\n", num_bytes);
    todo_wine ok(olp == &ov, "Overlapped structure is at %p\n", olp);
    todo_wine ok(olp && (olp->Internal == (ULONG)STATUS_CANCELLED), "Internal status is %lx\n", olp ? olp->Internal : 0);

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;
    bret = GetQueuedCompletionStatus( io_port, &num_bytes, &key, &olp, 200 );
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    ok(GetLastError() == WAIT_TIMEOUT, "Last error was %d\n", GetLastError());
    ok(key == 0xdeadbeef, "Key is %lu\n", key);
    ok(num_bytes == 0xdeadbeef, "Number of bytes transferred is %u\n", num_bytes);
    ok(!olp, "Overlapped structure is at %p\n", olp);

    /* Test IOCP without AcceptEx */

    if ((src = setup_iocp_src(&bindAddress)) == INVALID_SOCKET)
        goto end;

    SetLastError(0xdeadbeef);

    io_port = CreateIoCompletionPort((HANDLE)src, previous_port, 125, 0);
    ok(io_port != NULL, "failed to create completion port %u\n", GetLastError());

    closesocket(src);
    src = INVALID_SOCKET;

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;
    bret = GetQueuedCompletionStatus( io_port, &num_bytes, &key, &olp, 200 );
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    ok(GetLastError() == WAIT_TIMEOUT, "Last error was %d\n", GetLastError());
    ok(key == 0xdeadbeef, "Key is %lu\n", key);
    ok(num_bytes == 0xdeadbeef, "Number of bytes transferred is %u\n", num_bytes);
    ok(!olp, "Overlapped structure is at %p\n", olp);

    /* */

    if ((src = setup_iocp_src(&bindAddress)) == INVALID_SOCKET)
        goto end;

    connector = socket(AF_INET, SOCK_STREAM, 0);
    if (connector == INVALID_SOCKET) {
        skip("could not create connector socket, error %d\n", WSAGetLastError());
        goto end;
    }

    io_port = CreateIoCompletionPort((HANDLE)src, previous_port, 125, 0);
    ok(io_port != NULL, "failed to create completion port %u\n", GetLastError());

    io_port = CreateIoCompletionPort((HANDLE)dest, previous_port, 236, 0);
    ok(io_port != NULL, "failed to create completion port %u\n", GetLastError());

    bret = pAcceptEx(src, dest, buf, sizeof(buf) - 2*(sizeof(struct sockaddr_in) + 16),
            sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
            &num_bytes, &ov);
    ok(bret == FALSE, "AcceptEx returned %d\n", bret);
    ok(GetLastError() == ERROR_IO_PENDING, "Last error was %d\n", GetLastError());

    iret = connect(connector, (struct sockaddr*)&bindAddress, sizeof(bindAddress));
    ok(iret == 0, "connecting to accepting socket failed, error %d\n", GetLastError());

    closesocket(connector);
    connector = INVALID_SOCKET;

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;

    bret = GetQueuedCompletionStatus(io_port, &num_bytes, &key, &olp, 100);
    ok(bret == TRUE, "failed to get completion status %u\n", bret);
    ok(GetLastError() == 0xdeadbeef, "Last error was %d\n", GetLastError());
    ok(key == 125, "Key is %lu\n", key);
    ok(num_bytes == 0, "Number of bytes transferred is %u\n", num_bytes);
    ok(olp == &ov, "Overlapped structure is at %p\n", olp);
    ok(olp && (olp->Internal == (ULONG)STATUS_SUCCESS), "Internal status is %lx\n", olp ? olp->Internal : 0);

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;
    bret = GetQueuedCompletionStatus( io_port, &num_bytes, &key, &olp, 200 );
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    ok(GetLastError() == WAIT_TIMEOUT, "Last error was %d\n", GetLastError());
    ok(key == 0xdeadbeef, "Key is %lu\n", key);
    ok(num_bytes == 0xdeadbeef, "Number of bytes transferred is %u\n", num_bytes);
    ok(!olp, "Overlapped structure is at %p\n", olp);

    if (dest != INVALID_SOCKET)
        closesocket(dest);
    if (src != INVALID_SOCKET)
        closesocket(dest);

    /* */

    if ((src = setup_iocp_src(&bindAddress)) == INVALID_SOCKET)
        goto end;

    dest = socket(AF_INET, SOCK_STREAM, 0);
    if (dest == INVALID_SOCKET)
    {
        skip("could not create acceptor socket, error %d\n", WSAGetLastError());
        goto end;
    }

    connector = socket(AF_INET, SOCK_STREAM, 0);
    if (connector == INVALID_SOCKET) {
        skip("could not create connector socket, error %d\n", WSAGetLastError());
        goto end;
    }

    io_port = CreateIoCompletionPort((HANDLE)src, previous_port, 125, 0);
    ok(io_port != NULL, "failed to create completion port %u\n", GetLastError());

    io_port = CreateIoCompletionPort((HANDLE)dest, previous_port, 236, 0);
    ok(io_port != NULL, "failed to create completion port %u\n", GetLastError());

    bret = pAcceptEx(src, dest, buf, sizeof(buf) - 2*(sizeof(struct sockaddr_in) + 16),
            sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
            &num_bytes, &ov);
    ok(bret == FALSE, "AcceptEx returned %d\n", bret);
    ok(GetLastError() == ERROR_IO_PENDING, "Last error was %d\n", GetLastError());

    iret = connect(connector, (struct sockaddr*)&bindAddress, sizeof(bindAddress));
    ok(iret == 0, "connecting to accepting socket failed, error %d\n", GetLastError());

    iret = send(connector, buf, 1, 0);
    ok(iret == 1, "could not send 1 byte: send %d errno %d\n", iret, WSAGetLastError());

    closesocket(dest);
    dest = INVALID_SOCKET;

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;

    bret = GetQueuedCompletionStatus(io_port, &num_bytes, &key, &olp, 100);
    ok(bret == TRUE, "failed to get completion status %u\n", bret);
    ok(GetLastError() == 0xdeadbeef, "Last error was %d\n", GetLastError());
    ok(key == 125, "Key is %lu\n", key);
    ok(num_bytes == 1, "Number of bytes transferred is %u\n", num_bytes);
    ok(olp == &ov, "Overlapped structure is at %p\n", olp);
    ok(olp && (olp->Internal == (ULONG)STATUS_SUCCESS), "Internal status is %lx\n", olp ? olp->Internal : 0);

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;
    bret = GetQueuedCompletionStatus( io_port, &num_bytes, &key, &olp, 200 );
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    ok(GetLastError() == WAIT_TIMEOUT, "Last error was %d\n", GetLastError());
    ok(key == 0xdeadbeef, "Key is %lu\n", key);
    ok(num_bytes == 0xdeadbeef, "Number of bytes transferred is %u\n", num_bytes);
    ok(!olp, "Overlapped structure is at %p\n", olp);

    if (src != INVALID_SOCKET)
        closesocket(src);
    if (connector != INVALID_SOCKET)
        closesocket(connector);

    /* */

    if ((src = setup_iocp_src(&bindAddress)) == INVALID_SOCKET)
        goto end;

    dest = socket(AF_INET, SOCK_STREAM, 0);
    if (dest == INVALID_SOCKET)
    {
        skip("could not create acceptor socket, error %d\n", WSAGetLastError());
        goto end;
    }

    connector = socket(AF_INET, SOCK_STREAM, 0);
    if (connector == INVALID_SOCKET) {
        skip("could not create connector socket, error %d\n", WSAGetLastError());
        goto end;
    }

    io_port = CreateIoCompletionPort((HANDLE)src, previous_port, 125, 0);
    ok(io_port != NULL, "failed to create completion port %u\n", GetLastError());

    io_port = CreateIoCompletionPort((HANDLE)dest, previous_port, 236, 0);
    ok(io_port != NULL, "failed to create completion port %u\n", GetLastError());

    bret = pAcceptEx(src, dest, buf, sizeof(buf) - 2*(sizeof(struct sockaddr_in) + 16),
            sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
            &num_bytes, &ov);
    ok(bret == FALSE, "AcceptEx returned %d\n", bret);
    ok(GetLastError() == ERROR_IO_PENDING, "Last error was %d\n", GetLastError());

    iret = connect(connector, (struct sockaddr*)&bindAddress, sizeof(bindAddress));
    ok(iret == 0, "connecting to accepting socket failed, error %d\n", GetLastError());

    closesocket(dest);
    dest = INVALID_SOCKET;

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;

    bret = GetQueuedCompletionStatus(io_port, &num_bytes, &key, &olp, 100);
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    todo_wine ok((GetLastError() == ERROR_NETNAME_DELETED) || (GetLastError() == ERROR_CONNECTION_ABORTED), "Last error was %d\n", GetLastError());
    ok(key == 125, "Key is %lu\n", key);
    ok(num_bytes == 0, "Number of bytes transferred is %u\n", num_bytes);
    ok(olp == &ov, "Overlapped structure is at %p\n", olp);
    todo_wine ok(olp && ((olp->Internal == (ULONG)STATUS_LOCAL_DISCONNECT)
                      || (olp->Internal == (ULONG)STATUS_CONNECTION_ABORTED)), "Internal status is %lx\n", olp ? olp->Internal : 0);

    SetLastError(0xdeadbeef);
    key = 0xdeadbeef;
    num_bytes = 0xdeadbeef;
    olp = (WSAOVERLAPPED *)0xdeadbeef;
    bret = GetQueuedCompletionStatus( io_port, &num_bytes, &key, &olp, 200 );
    ok(bret == FALSE, "failed to get completion status %u\n", bret);
    ok(GetLastError() == WAIT_TIMEOUT, "Last error was %d\n", GetLastError());
    ok(key == 0xdeadbeef, "Key is %lu\n", key);
    ok(num_bytes == 0xdeadbeef, "Number of bytes transferred is %u\n", num_bytes);
    ok(!olp, "Overlapped structure is at %p\n", olp);


    end:
    if (dest != INVALID_SOCKET)
        closesocket(dest);
    if (src != INVALID_SOCKET)
        closesocket(src);
    if (connector != INVALID_SOCKET)
        closesocket(connector);
    CloseHandle(previous_port);
}

/**************** Main program  ***************/

START_TEST( sock )
{
    int i;

/* Leave these tests at the beginning. They depend on WSAStartup not having been
 * called, which is done by Init() below. */
    test_WithoutWSAStartup();
    test_WithWSAStartup();

    Init();

    test_set_getsockopt();
    test_so_reuseaddr();
    test_ip_pktinfo();
    test_extendedSocketOptions();

    for (i = 0; i < sizeof(tests)/sizeof(tests[0]); i++)
    {
        trace ( " **** STARTING TEST %d ****\n", i );
        do_test (  &tests[i] );
        trace ( " **** TEST %d COMPLETE ****\n", i );
    }

    test_UDP();

    test_getservbyname();
    test_WSASocket();

    test_WSAAddressToStringA();
    test_WSAAddressToStringW();

    test_WSAStringToAddressA();
    test_WSAStringToAddressW();

    test_errors();
    test_select();
    test_accept();
    test_getpeername();
    test_getsockname();
    test_inet_addr();
    test_addr_to_print();
    test_ioctlsocket();
    test_dns();
    test_gethostbyname_hack();

    test_WSASendTo();
    test_WSARecv();

    test_events(0);
    test_events(1);

    test_ipv6only();
    test_GetAddrInfoW();
    test_getaddrinfo();
    test_AcceptEx();
    test_ConnectEx();

    test_sioRoutingInterfaceQuery();

    test_WSAAsyncGetServByPort();
    test_WSAAsyncGetServByName();

    test_completion_port();

    /* this is a io heavy test, do it at the end so the kernel doesn't start dropping packets */
    test_send();
    test_synchronous_WSAIoctl();

    Exit();
}
