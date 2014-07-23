/* 
 * bqt_exception.cpp
 * 
 * Implements bqt_exeption.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_exception.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    exception::exception( std::string message ) throw()
    {
        this -> message << message;
    }
    exception::exception( const exception& o ) throw() : message( o.message.str() )
    {
        // Do nothing
    }
    exception::~exception() throw()
    {
        // Do nothing
    }
    
    const char* exception::what() const throw()
    {
        return message.str().c_str();
    }
    
    std::stringstream& exception::operator*()
    {
        return message;
    }
}


