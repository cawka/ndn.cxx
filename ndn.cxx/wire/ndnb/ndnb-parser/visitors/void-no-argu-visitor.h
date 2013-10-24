/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2013, Regents of the University of California
 *                     Alexander Afanasyev
 *
 * BSD license, See the LICENSE file for more information
 *
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef _NDNB_PARSER_VOID_NO_ARGU_VISITOR_H_
#define _NDNB_PARSER_VOID_NO_ARGU_VISITOR_H_

#include "../common.h"

NDN_NAMESPACE_BEGIN

namespace wire {
namespace NdnbParser {

/**
 * \ingroup ndn-ndnb
 * \brief Visitor interface that takes no arguments and returns nothing
 *
 * \see http://www.ndnx.org/releases/latest/doc/technical/BinaryEncoding.html
 * for ndnb encoding format help
 */
class VoidNoArguVisitor
{
public:
  virtual void visit (Blob& )=0; ///< \brief Method accepting BLOB block
  virtual void visit (Udata&)=0; ///< \brief Method accepting UDATA block
  virtual void visit (Tag&  )=0; ///< \brief Method accepting TAG block
  virtual void visit (Attr& )=0; ///< \brief Method accepting ATTR block
  virtual void visit (Dtag& )=0; ///< \brief Method accepting DTAG block
  virtual void visit (Dattr&)=0; ///< \brief Method accepting DATTR block
  virtual void visit (Ext&  )=0; ///< \brief Method accepting EXT block

  virtual ~VoidNoArguVisitor () { }
};
  
} // NdnbParser
} // wire

NDN_NAMESPACE_END

#endif // _NDNB_PARSER_VOID_NO_ARGU_VISITOR_H_
