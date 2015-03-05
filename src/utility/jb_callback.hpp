#ifndef JADEBASE_CALLBACK_HPP
#define JADEBASE_CALLBACK_HPP

/* 
 * jb_callback.hpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/



/******************************************************************************//******************************************************************************/

namespace jade
{
    class callback
    {
    public:
        virtual ~callback() {}
        virtual void call() {}                                                  // Virtual but non-pure virtual call() so this can be used as a default callback
                                                                                // TODO: Change this back to pure virtual after move to C++11
    };
}

/******************************************************************************//******************************************************************************/

#endif


