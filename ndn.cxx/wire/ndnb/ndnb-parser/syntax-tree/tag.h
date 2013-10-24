/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2013, Regents of the University of California
 *                     Alexander Afanasyev
 *
 * BSD license, See the LICENSE file for more information
 *
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef _NDNB_PARSER_TAG_H_
#define _NDNB_PARSER_TAG_H_

#include "base-tag.h"
#include <string>

NDN_NAMESPACE_BEGIN

namespace wire {
namespace NdnbParser {

/**
 * \ingroup ndnx-ndnb
 * \brief Class to represent TAG ndnb-encoded node
 *
 * \see http://www.ndnx.org/releases/latest/doc/technical/BinaryEncoding.html
 */
class Tag : public BaseTag
{
public:
  /**
   * \brief Constructor that actually parsed ndnb-encoded TAG block
   *
   * \param start  buffer iterator pointing to the first byte of TAG block name
   * \param length length of TAG name - 1 byte (i.e., minimum tag name is 1 byte)
   *
   * \see http://www.ndnx.org/releases/latest/doc/technical/BinaryEncoding.html
   */
  Tag (InputIterator &start, uint32_t length);

  virtual void accept( VoidNoArguVisitor &v )               { v.visit( *this ); }
  virtual void accept( VoidVisitor &v, boost::any param )   { v.visit( *this, param ); }
  virtual boost::any accept( NoArguVisitor &v )             { return v.visit( *this ); }
  virtual boost::any accept( Visitor &v, boost::any param ) { return v.visit( *this, param ); }

  std::string m_tag; ///< \brief Name of TAG block
};

} // namespace NdnbParser
} // namespace wire

NDN_NAMESPACE_END

#endif // _NDNB_PARSER_TAG_H_
