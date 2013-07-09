/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2013, Regents of the University of California
 *                     Yingdi Yu
 *
 * BSD license, See the LICENSE file for more information
 *
 * Author: Yingdi Yu <yingdi@cs.ucla.edu>
 */

#include "regex-patternlist-matcher.h"
#include "regex-repeat-matcher.h"

using namespace std;

namespace ndn
{

namespace regex
{
  
  bool RegexPatternListMatcher::Compile()
  {
    const int len = m_expr.size();
    int index = 0;
    int subHead = index;
    
    while(index < len){
      subHead = index;

      if(!ExtractPattern(subHead, &index))
	return false;
    }
    return true;

  }

  bool RegexPatternListMatcher::ExtractPattern(int index, int* next)
  {
    string errMsg = "Error: RegexPatternListMatcher.ExtractSubPattern(): ";
    
    const int start = index;
    int end = index;
    int indicator = index;
    RegexRepeatMatcher * matcher = NULL;

    switch(m_expr[index]){
    case '(':
      index++;
      index = ExtractSubPattern('(', ')', index);
      indicator = index;
      end = ExtractRepetition(index);
      

    case '<':
      index++;
      index = ExtractSubPattern('<', '>', index);
      indicator = index;
      end = ExtractRepetition(index);

    default:
      throw RegexException("Error: unexpected syntax");
    }

    matcher = new RegexRepeatMatcher(m_expr.substr(start, end), m_backRefManager, indicator);

    if(matcher->Compile()){
      m_matcherList.push_back(matcher);
      *next = end;
      return true;
    }
    else{
      throw RegexException(errMsg + "Cannot compile subpattern " + m_expr);
      return false;
    }
  }
  
  int RegexPatternListMatcher::ExtractSubPattern(const char left, const char right, int index)
  {
    int lcount = 1;
    int rcount = 0;

    while(lcount > rcount){

      if(index >= m_expr.size())
	throw RegexException("Error: parenthesis mismatch");

      if(left == m_expr[index])
        lcount++;

      if(right == m_expr[index])
        rcount++;

      index++;
    }
    return index;
  }

  int RegexPatternListMatcher::ExtractRepetition(int index)
  {
    string errMsg = "Error: RegexPatternListMatcher.ExtractRepetition(): ";

    int exprSize = m_expr.size();

    if(index == exprSize)
      return index;
    
    if('+' == m_expr[index] || '?' == m_expr[index] || '*' == m_expr[index]){
      return ++index;
    }

    if('{' == m_expr[index]){
      while('}' != m_expr[index] && index < exprSize){
        index++;
      }
      if(index == exprSize)
        throw RegexException(errMsg + "Missing right brace bracket");
      else
        return ++index;
    }

    return index;
  }

}//regex

}//ndn