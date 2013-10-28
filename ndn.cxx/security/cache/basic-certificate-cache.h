/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2013, Regents of the University of California
 *                     Yingdi Yu
 *
 * BSD license, See the LICENSE file for more information
 *
 * Author: Yingdi Yu <yingdi@cs.ucla.edu>
 */

#ifndef NDN_BASIC_CERTIFICATE_CACHE_H
#define NDN_BASIC_CERTIFICATE_CACHE_H

#include "certificate-cache.h"

#include <map>

namespace ndn
{

namespace security
{
  
  class BasicCertificateCache : public CertificateCache
  {
  public:
    
    virtual
    ~BasicCertificateCache() {}

    virtual void
    insertCertificate(Ptr<Certificate> certificate);

    virtual Ptr<Certificate> 
    getCertificate(const Name & certificateName, bool hasVersion=false);

  private:
    std::map<Name, Ptr<Certificate> > m_cache;
  };
  
}//security

}//ndn

#endif
