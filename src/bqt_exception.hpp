#ifndef BQT_EXCEPTION_H
#define BQT_EXCEPTION_H

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
    class exception : public std::exception
    {
    protected:
        std::string message;
    public:
        exception( std::string message ) throw();
        ~exception() throw();
        
        const char* what() const throw();
    };
}

/******************************************************************************//******************************************************************************/

#endif


