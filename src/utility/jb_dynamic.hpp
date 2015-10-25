#ifndef JADEBASE_DYNAMIC_HPP
#define JADEBASE_DYNAMIC_HPP

/* 
 * jb_dynamic.hpp
 * 
 * Utilities for temporally, spacially, and semantically dynamic values
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <vector>

/******************************************************************************//******************************************************************************/

namespace jade
{
    class window;
    
    template< typename T > class dynamic
    {
    public:
        dynamic( window& );
        ~dynamic();
        
        void addSubDynamic( dynamic< T >& );
        void changeReference( window& );
        
        const T& operator *();
    protected:
        window* reference;
        std::vector< dynamic< T > > sub_dynamics;
        
        virtual void twist( T& ) = 0;
    };
}

/******************************************************************************//******************************************************************************/

#endif


