/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2013 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Zhenkai Zhu <zhenkai@cs.ucla.edu>
 *         Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef NDN_WRAPPER_H
#define NDN_WRAPPER_H

#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/thread.hpp>

#include "ndn.cxx/common.h"
#include "ndn.cxx/name.h"
#include "ndn.cxx/interest.h"
#include "ndn.cxx/closure.h"
#include "ndn.cxx/pco.h"

class Executor;
typedef boost::shared_ptr<Executor> ExecutorPtr;

namespace ndn {

struct ndnOperationException : boost::exception, std::exception { };

class Verifier;
class Wrapper
{
public:
  const static int MAX_FRESHNESS = 2147; // max value for ccnx
  const static int DEFAULT_FRESHNESS = 60;
  typedef boost::function<void (InterestPtr)> InterestCallback;

  Wrapper();
  ~Wrapper();

  void
  start (); // called automatically in constructor

  /**
   * @brief Because of uncertainty with executor, in some case it is necessary to call shutdown explicitly (see test-server-and-fetch.cc)
   */
  void
  shutdown (); // called in destructor, but can called manually

  int
  setInterestFilter (const Name &prefix, const InterestCallback &interestCallback, bool record = true);

  void
  clearInterestFilter (const Name &prefix, bool record = true);

  int
  sendInterest (const Interest &interest, const Closure &closure);

  int
  publishData (const Name &name, const unsigned char *buf, size_t len, int freshness = DEFAULT_FRESHNESS, const Name &keyName=Name());

  inline int
  publishData (const Name &name, const Bytes &content, int freshness = DEFAULT_FRESHNESS, const Name &keyName=Name());

  inline int
  publishData (const Name &name, const std::string &content, int freshness = DEFAULT_FRESHNESS, const Name &keyName=Name());

  int
  publishUnsignedData(const Name &name, const unsigned char *buf, size_t len, int freshness = DEFAULT_FRESHNESS);

  inline int
  publishUnsignedData(const Name &name, const Bytes &content, int freshness = DEFAULT_FRESHNESS);

  inline int
  publishUnsignedData(const Name &name, const std::string &content, int freshness = DEFAULT_FRESHNESS);

  static Name
  getLocalPrefix ();

  Bytes
  createContentObject(const Name &name, const void *buf, size_t len, int freshness = DEFAULT_FRESHNESS, const Name &keyNameParam=Name());

  int
  putToCcnd (const Bytes &contentObject);

  bool
  verify(PcoPtr &pco, double maxWait = 1 /*seconds*/);

  PcoPtr
  get (const Interest &interest, double maxWait = 4.0/*seconds*/);

private:
  Wrapper(const Wrapper &other) {}

protected:
  void
  connectCcnd();

  /// @cond include_hidden
  void
  ccnLoop ();

  /// @endcond

protected:
  typedef boost::shared_mutex Lock;
  typedef boost::unique_lock<Lock> WriteLock;
  typedef boost::shared_lock<Lock> ReadLock;

  typedef boost::recursive_mutex RecLock;
  typedef boost::unique_lock<RecLock> UniqueRecLock;

  ccn* m_handle;
  RecLock m_mutex;
  boost::thread m_thread;
  bool m_running;
  bool m_connected;
  std::map<Name, InterestCallback> m_registeredInterests;
  ExecutorPtr m_executor;
  Verifier *m_verifier;
};

typedef boost::shared_ptr<Wrapper> WrapperPtr;

inline int
Wrapper::publishData (const Name &name, const Bytes &content, int freshness, const Name &keyName)
{
  return publishData(name, head(content), content.size(), freshness, keyName);
}

inline int
Wrapper::publishData (const Name &name, const std::string &content, int freshness, const Name &keyName)
{
  return publishData(name, reinterpret_cast<const unsigned char *> (content.c_str ()), content.size (), freshness, keyName);
}

inline int
Wrapper::publishUnsignedData(const Name &name, const Bytes &content, int freshness)
{
  return publishUnsignedData(name, head(content), content.size(), freshness);
}

inline int
Wrapper::publishUnsignedData(const Name &name, const std::string &content, int freshness)
{
  return publishUnsignedData(name, reinterpret_cast<const unsigned char *> (content.c_str ()), content.size (), freshness);
}


} // ndn

#endif
