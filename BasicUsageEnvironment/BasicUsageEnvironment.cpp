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
// Copyright (c) 1996-2018 Live Networks, Inc.  All rights reserved.
// Basic Usage Environment: for a simple, non-scripted, console application
// Implementation

#include "BasicUsageEnvironment.hh"
#include <stdio.h>
#ifdef DEBUG
#include <fstream>
#endif
////////// BasicUsageEnvironment //////////

#if defined(__WIN32__) || defined(_WIN32)
extern "C" int initializeWinsockIfNecessary();
#endif

BasicUsageEnvironment::BasicUsageEnvironment(TaskScheduler& taskScheduler)
: BasicUsageEnvironment0(taskScheduler) {
#if defined(__WIN32__) || defined(_WIN32)
  if (!initializeWinsockIfNecessary()) {
    setResultErrMsg("Failed to initialize 'winsock': ");
    reportBackgroundError();
    internalError();
  }
#endif
}

#ifdef DEBUG
char t_str[100];
std::ofstream _of = std::ofstream("_log.txt");
void _log(char const* str)
{
	_of.write(str,strlen(str));
	_of.flush();
}
#endif

BasicUsageEnvironment::~BasicUsageEnvironment() {
#ifdef DEBUG
	if(_of.is_open())
	{
		_of.close();
	}
#endif
}

BasicUsageEnvironment*
BasicUsageEnvironment::createNew(TaskScheduler& taskScheduler) {
  return new BasicUsageEnvironment(taskScheduler);
}

int BasicUsageEnvironment::getErrno() const {
#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_WCE)
  return WSAGetLastError();
#else
  return errno;
#endif
}


UsageEnvironment& BasicUsageEnvironment::operator<<(char const* str) {
  if (str == NULL) str = "(NULL)"; // sanity check
  fprintf(stderr, "%s", str);
#ifdef DEBUG
  _log(str);
#endif
  return *this;
}

UsageEnvironment& BasicUsageEnvironment::operator<<(int i) {
  fprintf(stderr, "%d", i);
  #ifdef DEBUG
memset(t_str,0,100);
  sprintf_s(t_str,100,"%d",i);
  _log(t_str);
#endif // DEBUG
  return *this;
}

UsageEnvironment& BasicUsageEnvironment::operator<<(unsigned u) {
  fprintf(stderr, "%u", u);
 #ifdef DEBUG
 memset(t_str, 0, 100);
  sprintf_s(t_str, 100, "%u", u);
  _log(t_str);
#endif
  return *this;
}

UsageEnvironment& BasicUsageEnvironment::operator<<(double d) {
  fprintf(stderr, "%f", d);
#ifdef DEBUG
  memset(t_str, 0, 100);
  sprintf_s(t_str, 100, "%f", d);
  _log(t_str);
#endif // DEBUG
  return *this;
}

UsageEnvironment& BasicUsageEnvironment::operator<<(void* p) {
  fprintf(stderr, "%p", p);
#ifdef DEBUG
  memset(t_str, 0, 100);
  sprintf_s(t_str, 100, "%p", p);
  _log(t_str);
#endif // DEBUG
  return *this;
}
