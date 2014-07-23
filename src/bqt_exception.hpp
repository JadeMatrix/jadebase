#ifndef BQT_EXCEPTION_HPP
#define BQT_EXCEPTION_HPP

/* 
 * bqt_exception.hpp
 * 
 * BQTDraw internal exceptions
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <iostream>
#include <string>

/******************************************************************************//******************************************************************************/

namespace bqt
{
    // Exceptions can be used like this for advanced message formatting:
    //     exception e;
    //     ff::write( *e, "Message: ", str, ", etc." );
    //     throw e;
    
    class exception : public std::exception
    {
    protected:
        std::string message;
    public:
        exception() throw() {};
        exception( std::string message ) throw();
        exception( const exception& o ) throw();
        ~exception() throw();
        
        const char* what() const throw();
        
        std::string& operator*();
    };
}

/******************************************************************************//******************************************************************************/

#endif


