/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2013, Regents of the University of California
 *                     Yingdi Yu
 *
 * BSD license, See the LICENSE file for more information
 *
 * Author: Yingdi Yu <yingdi@cs.ucla.edu>
 */

#include "cert-subject-descrypt-visitor.h"

#include "simple-visitor.h"
#include "../der.h"

#include "ndn.cxx/security/certificate/certificate-data.h"
#include "ndn.cxx/security/certificate/certificate-subdescrpt.h"

namespace ndn
{

namespace der
{
  void
  CertSubDescryptVisitor::visit(DerSequence& derSeq, boost::any param)
  {
    security::CertificateData* certData = boost::any_cast<security::CertificateData*> (param); 
    
    const DerNodePtrList & children = derSeq.getChildren();
    
    SimpleVisitor simpleVisitor;

    OID oid = boost::any_cast<OID>(children[0]->accept(simpleVisitor));
    string value = boost::any_cast<string>(children[1]->accept(simpleVisitor));

    security::CertificateSubDescrypt subDescrypt(oid, value);

    certData->addSubjectDescription(subDescrypt);
  }

}//der

}//ndn
