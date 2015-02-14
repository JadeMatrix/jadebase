/* 
 * jb_exception.cpp
 * 
 * Implements jb_exeption.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_exception.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    exception::exception( std::string message ) throw() : message( message )
    {
        // Do nothing
    }
    exception::exception( const exception& o ) throw() : message( o.message )
    {
        // Do nothing
    }
    exception::~exception() throw()
    {
        // Do nothing
    }
    
    const char* exception::what() const throw()
    {
        return message.c_str();
    }
    
    std::string& exception::operator*()
    {
        return message;
    }
}


