/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2013, Regents of the University of California
 *                     Alexander Afanasyev
 *
 * BSD license, See the LICENSE file for more information
 *
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "wire-ndnb.h"

#include <sstream>
#include <boost/foreach.hpp>
#include "ndnb-parser/common.h"
#include "ndnb-parser/visitors/name-visitor.h"
#include "ndnb-parser/syntax-tree/block.h"

NDN_NAMESPACE_BEGIN

namespace wire {

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define NDN_TT_BITS 3
#define NDN_TT_MASK ((1 << NDN_TT_BITS) - 1)
#define NDN_MAX_TINY ((1 << (7-NDN_TT_BITS)) - 1)
#define NDN_TT_HBIT ((unsigned char)(1 << 7))

size_t
Ndnb::AppendBlockHeader (OutputIterator &start, size_t val, uint32_t tt)
{
  unsigned char buf[1+8*((sizeof(val)+6)/7)];
  unsigned char *p = &(buf[sizeof(buf)-1]);
  size_t n = 1;
  p[0] = (NDN_TT_HBIT & ~NdnbParser::NDN_CLOSE) |
    ((val & NDN_MAX_TINY) << NDN_TT_BITS) |
    (NDN_TT_MASK & tt);
  val >>= (7-NDN_TT_BITS);
  while (val != 0) {
    (--p)[0] = (((unsigned char)val) & ~NDN_TT_HBIT) | NdnbParser::NDN_CLOSE;
    n++;
    val >>= 7;
  }
  start.Write (p,n);
  return n;
}

size_t
Ndnb::EstimateBlockHeader (size_t value)
{
  value >>= (7-NDN_TT_BITS);
  size_t n = 1;
  while (value>0)
    {
      value >>= 7;
      n++;
    }
  return n;
}

size_t
Ndnb::AppendNumber (OutputIterator &start, uint32_t number)
{
  std::ostringstream os;
  os << number;

  size_t written = 0;
  written += AppendBlockHeader (start, os.str().size(), NdnbParser::NDN_UDATA);
  written += os.str().size();
  start.Write (reinterpret_cast<const unsigned char*>(os.str().c_str()), os.str().size());

  return written;
}

size_t
Ndnb::EstimateNumber (uint32_t number)
{
  std::ostringstream os;
  os << number;
  return EstimateBlockHeader (os.str ().size ()) + os.str ().size ();
}

size_t
Ndnb::AppendCloser (OutputIterator &start)
{
  start.WriteU8 (NdnbParser::NDN_CLOSE);
  return 1;
}

// size_t
// Ndnb::AppendTimestampBlob (OutputIterator &start, const Time &time)
// {
//   // the original function implements Markers... thought not sure what are these markers for...

//   // Determine miminal number of bytes required to store the timestamp
//   int required_bytes = 2; // 12 bits for fractions of a second, 4 bits left for seconds. Sometimes it is enough
//   intmax_t ts = time.ToInteger (Time::S) >> 4;
//   for (;  required_bytes < 7 && ts != 0; ts >>= 8) // not more than 6 bytes?
//     required_bytes++;

//   size_t len = AppendBlockHeader(start, required_bytes, NdnbParser::NDN_BLOB);

//   // write part with seconds
//   ts = time.ToInteger (Time::S) >> 4;
//   for (int i = 0; i < required_bytes - 2; i++)
//     start.WriteU8 ( ts >> (8 * (required_bytes - 3 - i)) );

//   /* arithmetic contortions are to avoid overflowing 31 bits */
//   ts = ((time.ToInteger (Time::S) & 15) << 12) +
//     (((time.ToInteger (Time::NS) % 1000000000) / 5 * 8 + 195312) / 390625);
//   for (int i = required_bytes - 2; i < required_bytes; i++)
//     start.WriteU8 ( ts >> (8 * (required_bytes - 1 - i)) );

//   return len + required_bytes;
// }

size_t
Ndnb::AppendTimestampBlob (OutputIterator &os, const TimeInterval &time)
{
  // NDNx method function implements some markers, which are not really defined anywhere else...

  // Determine miminal number of bytes required to store the timestamp
  int required_bytes = 2; // 12 bits for fractions of a second, 4 bits left for seconds. Sometimes it is enough
  intmax_t ts = time.total_seconds () >> 4;
  for (;  required_bytes < 7 && ts != 0; ts >>= 8) // not more than 6 bytes?
     required_bytes++;

  size_t len = AppendBlockHeader(os, required_bytes, NdnbParser::NDN_BLOB);

  // write part with seconds
  ts = time.total_seconds () >> 4;
  for (int i = 0; i < required_bytes - 2; i++)
    os.put ( ts >> (8 * (required_bytes - 3 - i)) );

  /* arithmetic contortions are to avoid overflowing 31 bits */
  ts = ((time.total_seconds () & 15) << 12) +
    (((time.total_nanoseconds () % 1000000000) / 5 * 8 + 195312) / 390625);
  for (int i = required_bytes - 2; i < required_bytes; i++)
    os.put ( ts >> (8 * (required_bytes - 1 - i)) );

  return len + required_bytes;
}


// size_t
// Ndnb::EstimateTimestampBlob (const Time &time)
// {
//   int required_bytes = 2; // 12 bits for fractions of a second, 4 bits left for seconds. Sometimes it is enough
//   intmax_t ts = time.ToInteger (Time::S) >> 4;
//   for (;  required_bytes < 7 && ts != 0; ts >>= 8) // not more than 6 bytes?
//     required_bytes++;

//   return EstimateBlockHeader (required_bytes) + required_bytes;
// }

size_t
Ndnb::EstimateTimestampBlob (const TimeInterval &time)
{
  int required_bytes = 2; // 12 bits for fractions of a second, 4 bits left for seconds. Sometimes it is enough
  intmax_t ts = time.total_seconds () >> 4;
  for (;  required_bytes < 7 && ts != 0; ts >>= 8) // not more than 6 bytes?
    required_bytes++;

  return EstimateBlockHeader (required_bytes) + required_bytes;
}

size_t
Ndnb::AppendTaggedBlob (OutputIterator &start, uint32_t dtag,
                        const uint8_t *data, size_t size)
{
  size_t written = AppendBlockHeader (start, dtag, NdnbParser::NDN_DTAG);
  /* 2 */
  if (size>0)
    {
      written += AppendBlockHeader (start, size, NdnbParser::NDN_BLOB);
      start.Write (data, size);
      written += size;
      /* size */
    }
  written += AppendCloser (start);
  /* 1 */

  return written;
}

size_t
Ndnb::AppendTaggedBlobWithPadding (OutputIterator &start, uint32_t dtag,
                                   uint32_t length,
                                   const uint8_t *data, size_t size)
{
  if (size > length)
    {
      // no padding required
      return AppendTaggedBlob (start, dtag, data, size);
    }


  size_t written = AppendBlockHeader (start, dtag, NdnbParser::NDN_DTAG);

  /* 2 */
  if (length>0)
    {
      written += AppendBlockHeader (start, length, NdnbParser::NDN_BLOB);
      start.Write (data, size);
      start.WriteU8 (0, length - size);
      written += length;
      /* size */
    }
  written += AppendCloser (start);
  /* 1 */

  return written;
}

size_t
Ndnb::EstimateTaggedBlob (uint32_t dtag, size_t size)
{
  if (size>0)
    return EstimateBlockHeader (dtag) + EstimateBlockHeader (size) + size + 1;
  else
    return EstimateBlockHeader (dtag) + 1;
}

size_t
Ndnb::AppendString (OutputIterator &start, uint32_t dtag,
                    const std::string &string)
{
  size_t written = AppendBlockHeader (start, dtag, NdnbParser::NDN_DTAG);
  {
    written += AppendBlockHeader (start, string.size (), NdnbParser::NDN_UDATA);
    start.Write (reinterpret_cast<const uint8_t*> (string.c_str ()), string.size ());
    written += string.size ();
  }
  written += AppendCloser (start);

  return written;
}

void
Ndnb::AppendTaggedNumber (OutputIterator &os, uint32_t dtag, uint32_t number)
{
  AppendBlockHeader (os, dtag, NdnbParser::NDN_DTAG);
  {
    AppendNumber (os, number);
  }
  AppendCloser (os);
}

size_t
Ndnb::EstimateString (uint32_t dtag, const std::string &string)
{
  return EstimateBlockHeader (dtag) + EstimateBlockHeader (string.size ()) + string.size () + 1;
}

size_t
Ndnb::SerializeName (OutputIterator &start, const Name &name)
{
  size_t written = 0;
  BOOST_FOREACH (const name::Component &component, name)
    {
      written += AppendTaggedBlob (start, NdnbParser::NDN_DTAG_Component,
                                   reinterpret_cast<const uint8_t*>(component.buf ()), component.size());
    }
  return written;
}

size_t
Ndnb::SerializedSizeName (const Name &name)
{
  size_t written = 0;
  BOOST_FOREACH (const name::Component &component, name)
    {
      written += EstimateTaggedBlob (NdnbParser::NDN_DTAG_Component, component.size ());
    }
  return written;
}

Ptr<Name>
Ndnb::DeserializeName (InputIterator &i)
{
  Ptr<Name> name = Create<Name> ();
  NdnbParser::NameVisitor nameVisitor;

  Ptr<NdnbParser::Block> root = NdnbParser::Block::ParseBlock (i);
  root->accept (nameVisitor, GetPointer (name));

  return name;
}

} // wire

NDN_NAMESPACE_END
