/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2018, Live Networks, Inc.  All rights reserved
// LIVE555 Proxy Server
// main program

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include <string>
#include <vector>
#include <fstream>
#include "..\include\json.hpp" 

using json = nlohmann::json;
using namespace std;


char const* progName;
UsageEnvironment* env;
UserAuthenticationDatabase* authDB = NULL;
UserAuthenticationDatabase* authDBForREGISTER = NULL;

// Default values of command-line parameters:
int verbosityLevel = 0;
Boolean streamRTPOverTCP = False;
portNumBits tunnelOverHTTPPortNum = 0;
portNumBits rtspServerPortNum = 554;
char* username = NULL;
char* password = NULL;
std::string ipServer;
char* ipProxy = NULL;
Boolean proxyREGISTERRequests = False;
char* usernameForREGISTER = NULL;
char* passwordForREGISTER = NULL;

extern netAddressBits ReceivingInterfaceAddr;

static RTSPServer* createRTSPServer(Port port) {
  if (proxyREGISTERRequests) {
    return RTSPServerWithREGISTERProxying::createNew(*env, port, authDB, authDBForREGISTER, 65, streamRTPOverTCP, verbosityLevel, username, password);
  } else {
    return RTSPServer::createNew(*env, port, authDB);
  }
}

void usage(int where = 0) {
  *env << "\nUsage: " << progName
       << " [-c|-C] <config json path>";
  exit(1);
}

class RTSPConfig {
public:
	std::string user;
	std::string password;
	std::string proxiedUrl;
	std::string streamUrl;
protected:
	RTSPConfig(json jd);
public:
	static RTSPConfig* createNew(json jsondata);
};

RTSPConfig::RTSPConfig(json jd)
{
	std::string rtsp_url = jd[0];
	proxiedUrl = rtsp_url;
	string user = jd[1];
	this->user = user;
	string password = jd[2];
	this->password = password;
	string outUrl = jd[3];
	this->streamUrl = outUrl;
}

RTSPConfig* RTSPConfig::createNew(json jsondata)
{
	if(jsondata.size() != 4)
	{
		*env << "\n Error format in rtsp_urls \n";
		exit(1);
	}

	RTSPConfig* cfg = new RTSPConfig(jsondata);
	
	return cfg;
}
vector<RTSPConfig*> vecConfig;

void ReadConfigJson(std::string j3path)
{
	
	std::ifstream i(j3path);
	
	json j3;
	i >> j3;

	//*env << j3.dump().c_str();

	if(j3.find("log") != j3.end())
	{
		verbosityLevel = j3["log"];
	}

	if(j3.find("port") != j3.end())
	{
		rtspServerPortNum = j3["port"];
		*env << "\nServerPort : " << rtspServerPortNum << "\n";
	}
	if(j3.find("serverip") != j3.end())
	{
		std::string ip = j3["serverip"];
		ipServer = ip;
		*env << "ServerIP : " << ipServer .c_str()<< "\n";
	}
	if(j3.find("users") != j3.end())
	{
		json users = j3["users"];
		if(users.size() > 0)
		{
			authDB = new UserAuthenticationDatabase;

			for(int i = 0; i< users.size(); i++)
			{
				json info = users[i];
				std::string username = info[0];
				std::string pwd = info[1];
				authDB->addUserRecord(username.c_str(), pwd.c_str());
			}
		}
		
		//authDB->addUserRecord("username1", "password1"); // replace these with real strings
	}
	if(j3.find("rtsp_urls") != j3.end())
	{
		json rs = j3["rtsp_urls"];
		if(!rs.is_array())
		{
			*env << "Error Json Config Format!";
			exit(1);
		}

		int sz = rs.size();
		

		for(int i =0; i < sz; i++)
		{
			auto cfg = RTSPConfig::createNew(rs[i]);
			vecConfig.push_back(cfg);
		}
	}
	else{
		usage();
		exit(1);
	}
}
//extern void TestFunc(int argc, char** argv);

int main(int argc, char** argv) {

  // Increase the maximum size of video frames that we can 'proxy' without truncation.
  // (Such frames are unreasonably large; the back-end servers should really not be sending frames this large!)
  OutPacketBuffer::maxSize = 100000; // bytes

  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  *env << "Df Wise Proxy Server\n"
       << " Library Version :"
       << LIVEMEDIA_LIBRARY_VERSION_STRING ;
  *env << "\n ver 1.0.0 \n";

  // Check command-line arguments: optional parameters, then one or more rtsp:// URLs (of streams to be proxied):
  progName = argv[0];
  if (argc < 2) usage(1);
  while (argc > 1)
  {
    // Process initial command-line options (beginning with "-"):
    char *const opt = argv[1];
    if (opt[0] != '-')
      break; // the remaining parameters are assumed to be "rtsp://" URLs

    switch (opt[1])
    {
	case 'C':
	case 'c':
	{
		std::string p(argv[2]);
		ReadConfigJson(p);
		++argv;
		--argc;
		break;
	}
    case 'v':
    { // verbose output
      verbosityLevel = 1;
      break;
    }

    case 'V':
    { // more verbose output
      verbosityLevel = 2;
      break;
    }

    case 't':
    {
      // Stream RTP and RTCP over the TCP 'control' connection.
      // (This is for the 'back end' (i.e., proxied) stream only.)
      streamRTPOverTCP = True;
      break;
    }

    case 'T':
    {
      // stream RTP and RTCP over a HTTP connection
      if (argc > 2 && argv[2][0] != '-')
      {
        // The next argument is the HTTP server port number:
        if (sscanf(argv[2], "%hu", &tunnelOverHTTPPortNum) == 1 && tunnelOverHTTPPortNum > 0)
        {
          ++argv;
          --argc;
          break;
        }
      }

      // If we get here, the option was specified incorrectly:
      usage(2);
      break;
    }

	
	
    case 'i':{
      int sz = strlen(opt); 
      *env << "size:" << sz << "\n" << opt;
      if(sz == 3)
      {
        switch(opt[2])
        {
          case 'p':
          {
            ipServer = argv[2];
            //ipProxy = argv[3];
            argv += 1;
            argc -= 1;
            
            // *env << "\nip:" << ip <<"\n";
          } break;
          default:
          {
            // *env << "no p?";
            usage(3);
          }break;
        }
      }
      else
      {
        usage(4);
      }
      
    break;
    } 

    case 'p': {
      // specify a rtsp server port number 
      if (argc > 2 && argv[2][0] != '-') {
        // The next argument is the rtsp server port number:
        if (sscanf(argv[2], "%hu", &rtspServerPortNum) == 1
            && rtspServerPortNum > 0) {
          ++argv; --argc;
          break;
        }
      }

      // If we get here, the option was specified incorrectly:
      usage(5);
      break;
    }
    
    case 'u': { // specify a username and password (to be used if the 'back end' (i.e., proxied) stream requires authentication)
      if (argc < 4) usage(6); // there's no argv[3] (for the "password")
      username = argv[2];
      password = argv[3];
      argv += 2; argc -= 2;
      break;
    }

    case 'U': { // specify a username and password to use to authenticate incoming "REGISTER" commands
      if (argc < 4) usage(7); // there's no argv[3] (for the "password")
      usernameForREGISTER = argv[2];
      passwordForREGISTER = argv[3];

      if (authDBForREGISTER == NULL) authDBForREGISTER = new UserAuthenticationDatabase;
      authDBForREGISTER->addUserRecord(usernameForREGISTER, passwordForREGISTER);
      argv += 2; argc -= 2;
      break;
    }

    case 'R': { // Handle incoming "REGISTER" requests by proxying the specified stream:
      proxyREGISTERRequests = True;
      break;
    }

    default: {
      usage(8);
      break;
    }
    }

    ++argv; --argc;
  }

  //if (argc < 2 && !proxyREGISTERRequests){
  //  *env << "\nthere must be at least one \"rtsp://\" URL at the end \n"; 
  //  usage(9); 
  //} 

  // Make sure that the remaining arguments appear to be "rtsp://" URLs:
 /* int i;
  for (i = 1; i < argc; ++i) {
    if (strncmp(argv[i], "rtsp://", 7) != 0) usage(10);
  }
*/
  // Do some additional checking for invalid command-line argument combinations:
  if (authDBForREGISTER != NULL && !proxyREGISTERRequests) {
    *env << "The '-U <username> <password>' option can be used only with -R\n";
    usage(11);
  }
  if (streamRTPOverTCP) {
    if (tunnelOverHTTPPortNum > 0) {
      *env << "The -t and -T options cannot both be used!\n";
      usage(12);
    } else {
      tunnelOverHTTPPortNum = (portNumBits)(~0); // hack to tell "ProxyServerMediaSession" to stream over TCP, but not using HTTP
    }
  }

#ifdef ACCESS_CONTROL
  // To implement client access control to the RTSP server, do the following:
  authDB = new UserAuthenticationDatabase;
  authDB->addUserRecord("username1", "password1"); // replace these with real strings
      // Repeat this line with each <username>, <password> that you wish to allow access to the server.
#endif

  // Create the RTSP server. Try first with the configured port number,
  // and then with the default port number (554) if different,
  // and then with the alternative port number (8554):
  if(!ipServer.empty())
	ReceivingInterfaceAddr = inet_addr(ipServer.c_str());


  RTSPServer* rtspServer;
  rtspServer = createRTSPServer(rtspServerPortNum);

  if (rtspServer == NULL) {
    if (rtspServerPortNum != 554) {
      *env << "Unable to create a RTSP server with port number " << rtspServerPortNum << ": " << env->getResultMsg() << "\n";
      *env << "Trying instead with the standard port numbers (554 and 8554)...\n";

      rtspServerPortNum = 554;
      rtspServer = createRTSPServer(rtspServerPortNum);
    }
  }

  if (rtspServer == NULL) {
    rtspServerPortNum = 8554;
    rtspServer = createRTSPServer(rtspServerPortNum);
  }

  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }
  ReceivingInterfaceAddr = 0x0; //inet_addr(ipServer);
  // Create a proxy for each "rtsp://" URL specified on the command line:
 
  for (int i = 0; i <  vecConfig.size(); ++i) {
    // = argv[i];

    //char streamName[60];
    //if (argc == 2) {
    //  sprintf(streamName, "%s", "proxyStream"); // there's just one stream; give it this name
    //} else {
    //  sprintf(streamName, "proxyStream-%d", i); // there's more than one stream; distinguish them by name
    //}
	RTSPConfig* cfg = vecConfig[i];
	if(cfg == NULL)
		continue;
    char const* proxiedStreamURL = cfg->proxiedUrl.c_str();
	auto streamUrl = cfg->streamUrl.c_str();
	auto user = cfg->user.c_str();
	auto pwd = cfg->password.c_str();

    ServerMediaSession* sms
      = ProxyServerMediaSession::createNew(*env, rtspServer,
					   proxiedStreamURL, streamUrl,
					   user, pwd, tunnelOverHTTPPortNum, verbosityLevel);

    rtspServer->addServerMediaSession(sms);


    char* proxyStreamURL = rtspServer->rtspURL(sms);
	//char* proxyStreamURL = new char[100];
	//sprintf(proxyStreamURL, "rtsp://%s/%s",ipServer.c_str(),streamUrl);
    *env << "RTSP stream, proxying the stream \"" << proxiedStreamURL << "\"\n";
    *env << "\tPlay this stream using the URL: " << proxyStreamURL << "\n";
    delete[] proxyStreamURL;
  }

  if (proxyREGISTERRequests) {
    *env << "(We handle incoming \"REGISTER\" requests on port " << rtspServerPortNum << ")\n";
  }

  // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
  // Try first with the default HTTP port (80), and then with the alternative HTTP
  // port numbers (8000 and 8080).

  if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
    *env << "\n(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
  } else {
    *env << "\n(RTSP-over-HTTP tunneling is not available.)\n";
  }

  // Now, enter the event loop:
  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}
