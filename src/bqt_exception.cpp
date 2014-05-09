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
        this -> message = message;
    }
    exception::~exception() throw()
    {
        // Do nothing
    }
    
    const char* exception::what() const throw()
    {
        return message.c_str();
    }
}


