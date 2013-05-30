/***************************************************************************
 *  $RCSfile: Error.h,v $
 *  
 *  Last modifier: $Author: jgonzale $
 *  Last check in: $Date: 2009/02/27 10:42:21 $
 *  Revision:      $Revision: 1.2 $
 *
 *  Copyright  2006  jgonzale
 *  jgonzale@cepba.upc.edu
 ****************************************************************************/

#ifndef _ERROR_H
#define _ERROR_H

#include <string>
using std::string;

class Error
{
  protected:
    bool   _Error;
    string LastError;
  
  public:
    Error(void) { _Error = false; };
    
    bool GetError(void) { return _Error; };
    void SetError(bool _Error) { this->_Error = _Error; };
    
    string GetLastError(void) { return LastError; };
    
    void SetErrorMessage(string& UserMessage, string& SysMessage)
    {
      LastError = UserMessage + " (" + SysMessage + ")";
      return;
    }
    
    void SetErrorMessage(const char* UserMessage, const char* SysMessage)
    {
      string UserError = UserMessage;
      string SysError  = SysMessage;

      SetErrorMessage(UserError, SysError);
    };
    
    void SetErrorMessage(string& UserMessage, const char* SysMessage)
    {
      string SysError = SysMessage;
      SetErrorMessage(UserMessage, SysError);
    }
    
    void SetErrorMessage(const char* UserMessage, string SysMessage)
    {
      string UserError = UserMessage;
      SetErrorMessage(UserError, SysMessage);
    }
    
    void SetErrorMessage(string UserMessage)
    {
      LastError = UserMessage;
    }
    
    void SetErrorMessage(const char* UserMessage)
    {
      string UserError = UserMessage;
      LastError        = UserError;
    }
};

/* Special prototype of 'error' function needed by R*-Tree. Impemented in
 * 'main.cpp' */
extern void error (char* Message, bool Exit);

#endif /* _ERROR_H */
