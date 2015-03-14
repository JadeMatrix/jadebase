#ifndef JADEBASE_EXCEPTION_HPP
#define JADEBASE_EXCEPTION_HPP

/* 
 * jb_exception.hpp
 * 
 * jadebase internal exceptions
 * 
 * Exceptions can be used like this for advanced message formatting:
 *     exception e;
 *     ff::write( *e, "Message: ", str, ", etc." );
 *     throw e;
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

/******************************************************************************//******************************************************************************/

namespace jade
{
    class exception
    {
    public:
        exception() throw() {};                                                 // Creates an exception with a blank message string ("")
        exception( std::string ) throw();                                       // Creates an exception with the given message string
        exception( const exception& ) throw();                                  // Copy constructor
        ~exception() throw();
        
        const char* what() const throw();                                       // Retrieve the C-string version of the message
        
        std::string& operator*();                                               // Dereference operator for fancy message formatting
        
    protected:
        std::string message;
    };
}

/******************************************************************************//******************************************************************************/

#endif


