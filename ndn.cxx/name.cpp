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

#include "name.h"

#include <boost/lexical_cast.hpp>
#include <ctype.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/make_shared.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace std;

namespace ndn
{

///////////////////////////////////////////////////////////////////////////////
//                              CONSTRUCTORS                                 //
///////////////////////////////////////////////////////////////////////////////

Name::Name ()
{
}

Name::Name (const string &name)
{
  stringstream ss(name);
  string compStr;
  bool first = true;
  while(getline(ss, compStr, '/'))
  {
    // discard the first empty comp before the first '/'
    if (first)
    {
      first = false;
      continue;
    }
    Bytes comp(compStr.begin(), compStr.end());
    m_comps.push_back(comp);
  }
}

Name::Name (const vector<Bytes> &comps)
{
  m_comps = comps;
}

Name::Name (const Name &other)
{
  m_comps = other.m_comps;
}

Name::Name (const unsigned char *data, const ccn_indexbuf *comps)
{
  for (unsigned int i = 0; i < comps->n - 1; i++)
  {
    const unsigned char *compPtr;
    size_t size;
    ccn_name_comp_get(data, comps, i, &compPtr, &size);
    Bytes comp;
    readRaw(comp, compPtr, size);
    m_comps.push_back(comp);
  }
}

Name::Name (const void *buf, const size_t length)
{
  ccn_indexbuf *idx = ccn_indexbuf_create();
  const ccn_charbuf namebuf = { length, length, const_cast<unsigned char *> (reinterpret_cast<const unsigned char *> (buf)) };
  ccn_name_split (&namebuf, idx);

  const unsigned char *compPtr = NULL;
  size_t size = 0;
  int i = 0;
  while (ccn_name_comp_get(namebuf.buf, idx, i, &compPtr, &size) == 0)
    {
      Bytes comp;
      readRaw (comp, compPtr, size);
      m_comps.push_back(comp);
      i++;
    }
  ccn_indexbuf_destroy(&idx);
}

Name::Name (const Charbuf &buf)
{
  ccn_indexbuf *idx = ccn_indexbuf_create();
  ccn_name_split (buf.getBuf (), idx);

  const unsigned char *compPtr = NULL;
  size_t size = 0;
  int i = 0;
  while (ccn_name_comp_get(buf.getBuf ()->buf, idx, i, &compPtr, &size) == 0)
    {
      Bytes comp;
      readRaw (comp, compPtr, size);
      m_comps.push_back(comp);
      i++;
    }
  ccn_indexbuf_destroy(&idx);
}

Name::Name (const ccn_charbuf *buf)
{
  ccn_indexbuf *idx = ccn_indexbuf_create();
  ccn_name_split (buf, idx);

  const unsigned char *compPtr = NULL;
  size_t size = 0;
  int i = 0;
  while (ccn_name_comp_get(buf->buf, idx, i, &compPtr, &size) == 0)
    {
      Bytes comp;
      readRaw (comp, compPtr, size);
      m_comps.push_back(comp);
      i++;
    }
  ccn_indexbuf_destroy(&idx);
}

Name &
Name::operator= (const Name &other)
{
  m_comps = other.m_comps;
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
//                                SETTERS                                    //
///////////////////////////////////////////////////////////////////////////////

Name &
Name::append (const Name &comp)
{
  m_comps.insert (m_comps.end (),
                  comp.m_comps.begin (), comp.m_comps.end ());
  return *this;
}

Name &
Name::append (const Bytes &comp)
{
  m_comps.push_back(comp);
  return *this;
}

Name &
Name::append (const string &compStr)
{
  Bytes comp (compStr.begin(), compStr.end());
  return append (comp);
}

Name &
Name::append (const void *buf, size_t size)
{
  Bytes comp (reinterpret_cast<const unsigned char*> (buf), reinterpret_cast<const unsigned char*> (buf) + size);
  return append (comp);
}

Name &
Name::appendNumber (uint64_t number)
{
  Bytes comp;
  while (number > 0)
    {
      comp.push_back (static_cast<unsigned char> (number & 0xFF));
      number >>= 8;
    }
  return append (comp);
}

Name &
Name::appendNumberWithMarker (uint64_t number, unsigned char marker)
{
  Bytes comp;
  comp.push_back (marker);

  while (number > 0)
    {
      comp.push_back (static_cast<unsigned char> (number & 0xFF));
      number >>= 8;
    }
  return append (comp);
}

inline Name &
Name::appendVersion (uint64_t version/* = Name::nversion*/)
{
  if (version != Name::nversion)
    return appendNumberWithMarker (version, 0xFD);
  else
    {
      boost::posix_time::ptime now (boost::posix_time::microsec_clock::universal_time ());
      version = (now.total_seconds () << 12) | (0xFFF & (now.fractional_seconds () / 244 /*( 1000,000 microseconds / 4096.0 resolution = last 12 bits)*/));
      return appendNumberWithMarker (version, 0xFD);
    }
}


///////////////////////////////////////////////////////////////////////////////
//                                GETTERS                                    //
///////////////////////////////////////////////////////////////////////////////

const Bytes &
Name::get (int index) const
{
  if (index < 0)
    {
      index = m_comps.size () - 1 - index;
    }

  if (static_cast<unsigned int> (index) >= m_comps.size ())
    {
      boost::throw_exception (NameException() << error_info_str("Index out of range: " + boost::lexical_cast<string> (index)));
    }
  return m_comps [index];
}

Bytes &
Name::get (int index)
{
  if (index < 0)
    {
      index = m_comps.size () - 1 - index;
    }

  if (static_cast<unsigned int> (index) >= m_comps.size())
    {
      boost::throw_exception(NameException() << error_info_str("Index out of range: " + boost::lexical_cast<string>(index)));
    }
  return m_comps[index];
}


/////
///// Static helpers to convert name component to appropriate value
/////

std::string
Name::asString (const Bytes &comp)
{
  ostringstream ss;
  for (Bytes::const_iterator ch = comp.begin (); ch != comp.end (); ch++)
  {
    if (isprint(*ch))
    {
      ss << static_cast<char> (*ch);
    }
    else
    {
      ss << "%" << hex << setfill('0') << setw(2) << static_cast<unsigned int> (*ch);
    }
  }

  return ss.str();
}

uint64_t
Name::asNumber (const Bytes &comp) const
{
  uint64_t ret = 0;
  for (Bytes::const_reverse_iterator i = comp.rbegin (); i != comp.rend (); i++)
    {
      ret <<= 8;
      ret |= *i;
    }
  return ret;
}

uint64_t
Name::asNumberWithMarker (const Bytes &comp, unsigned char marker) const
{
  if (comp.empty () ||
      *(comp.begin ()) != marker)
    {
      boost::throw_exception (NameException ()
                              << error_info_str("Name component does not have required marker: " + Name::asString (comp)));
    }
  uint64_t ret = 0;
  Bytes::const_reverse_iterator i = comp.rbegin ();
  unsigned char value = *i;
  i++;
  for (; i != comp.rend (); i++)
    {
      ret <<= 8;
      ret |= value;

      value = *i;
    }
  return ret;
}

Name
Name::getSubName (size_t pos/* = 0*/, size_t len/* = Name::npos*/) const
{
  Name retval;

  if (len == npos)
    {
      len = m_comps.size () - pos;
    }
  
  if (pos + len > m_comps.size ())
    {
      boost::throw_exception (NameException() <<
                              error_info_str ("getSubName parameter out of range"));
    }

  for (size_t i = pos; i < pos + len; i++)
    {
      retval.append (get (i));
    }

  return retval;
}


bool
Name::operator==(const Name &name) const
{
  const_iterator i = begin ();
  const_iterator j = name.begin ();

  for (; i != end () && j != name.end (); i++, j++)
    {
      if (*i != *j)
        return false;
    }

  return i == end () && j == name.end ();
}

Name
Name::operator+ (const Name &name) const
{
  Name newName (*this);
  copy (name.m_comps.begin(), name.m_comps.end(), back_inserter (newName));
  return newName;
}


CharbufPtr
Name::toCharbuf () const
{
  CharbufPtr ptr = make_shared<Charbuf> ();

  ccn_charbuf *cbuf = ptr->getBuf();
  ccn_name_init(cbuf);
  int size = m_comps.size();
  for (int i = 0; i < size; i++)
  {
    ccn_name_append(cbuf, head(m_comps[i]), m_comps[i].size());
  }
  return ptr;
}

std::string
Name::toUri () const
{
  return boost::lexical_cast<std::string> (*this)
}

ostream &
operator << (ostream &os, const Name &name)
{
  int size = name.size();
  vector<string> strComps;
  for (int i = 0; i < size; i++)
  {
    strComps.push_back(name.getCompAsString(i));
  }
  string joined = boost::algorithm::join(strComps, "/");
  os << "/" << joined;
  return os;
}

bool
Name::operator !=(const Name &name) const
{
  return !(*this == name);
}

bool
Name::operator < (const Name &name) const
{
  /// @todo implement comparison in the right way, without converting to URI
  // const_iterator i = begin ();
  // const_iterator j = name.begin ();

  // for (; i != end () && j != name.end (); i++, j++)
  //   {
  //     if (*i ??? *j)
  //       return false;
  //   }

  // return i == end () && j == name.end ();

  
  return toUri () < name.toUri ();
}


} // ndn