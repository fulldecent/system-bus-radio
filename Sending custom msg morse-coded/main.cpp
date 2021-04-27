#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <sstream>

// SYSTEM BUS RADIO
// https://github.com/fulldecent/system-bus-radio
// Copyright 2016 William Entriken
// C++11 port by Ryou Ezoe
// Wrapper C++ Server by Alberto Sanchez, Pedro J. Martinez, Jorge Rivera

using namespace std;

#define SERVER_PORT htons(50007)

std::mutex m ;
std::condition_variable cv ;
std::chrono::high_resolution_clock::time_point mid ;
std::chrono::high_resolution_clock::time_point reset ;


void boost_song()
{
    using namespace std::chrono ;

    while( true )
    {
        std::unique_lock<std::mutex> lk{m} ;
        cv.wait( lk ) ;

        std::atomic<unsigned> x{0} ;
        while( high_resolution_clock::now() < mid )
        {
            ++x ;
        }
        std::this_thread::sleep_until( reset ) ;
    }
}

void square_am_signal(float time, float frequency)
{
    using namespace std::chrono ;

    std::cout << "Playing / " << time << " seconds / " << frequency << " Hz\n" ;

    seconds const sec{1} ;
    nanoseconds const nsec{ sec } ;
    using rep = nanoseconds::rep ;
    auto nsec_per_sec = nsec.count() ;

    nanoseconds const period( static_cast<rep>( nsec_per_sec / frequency) ) ;

    auto start = high_resolution_clock::now() ;
    auto const end = start + nanoseconds( static_cast<rep>(time * nsec_per_sec) ) ;

    while (high_resolution_clock::now() < end)
    {
        mid = start + period / 2 ;
        reset = start + period ;

        cv.notify_all() ;
        std::this_thread::sleep_until( reset ) ;
        start = reset;
    }
}

vector<string> split(string str, char delimiter) {
  vector<string> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;
  
  while(getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }
  
  return internal;
}

int main(int argc, char* args[]){

    for ( unsigned i = 0 ; i < std::thread::hardware_concurrency() ; ++i )
    {
        std::thread t( boost_song ) ;
        t.detach() ;
    }

    vector<string> v;
    int dur;
    int freq;
    float duration;

    char buffer[1000];
    int n;

    int serverSock=socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = SERVER_PORT;
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    /* bind (this socket, local address, address length)
       bind server socket (serverSock) to server address (serverAddr).  
       Necessary so that server can use a specific port */ 
    bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(struct sockaddr));

    // wait for a client
    /* listen (this socket, request queue length) */
    listen(serverSock,1);
    sockaddr_in clientAddr;
    socklen_t sin_size=sizeof(struct sockaddr_in);
    int clientSock=accept(serverSock,(struct sockaddr*)&clientAddr, &sin_size);

    while (1 == 1) {
            bzero(buffer, 1000);

            //receive a message from a client
            n = read(clientSock, buffer, 500);
            cout << "Confirmation code  " << n << endl;
            cout << "Server received:  " << buffer << endl;

            v = split(buffer, '-');
            dur = std::stoi(v[0]);
            freq = std::stoi(v[1]);
            duration = (float)dur / 1000;

            square_am_signal(duration, freq);
    }

    close(serverSock);
    return 0;
    
}

