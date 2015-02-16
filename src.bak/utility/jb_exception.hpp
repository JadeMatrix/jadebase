#ifndef JADEBASE_EXCEPTION_HPP
#define JADEBASE_EXCEPTION_HPP

/* 
 * jb_exception.hpp
 * 
 * jadebase internal exceptions
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

/******************************************************************************//******************************************************************************/

namespace jade
{
    // Exceptions can be used like this for advanced message formatting:
    //     exception e;
    //     ff::write( *e, "Message: ", str, ", etc." );
    //     throw e;
    
    class exception
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


