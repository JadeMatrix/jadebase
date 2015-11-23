#ifndef JADEBASE_TENSOR_HPP
#define JADEBASE_TENSOR_HPP

/* 
 * jb_tensor.hpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/



/******************************************************************************//******************************************************************************/

namespace jade
{
    template< typename T > class tensor
    {
        friend class tensor_engine< T >;
        
    public:
        virtual ~tensor();
        
        virtual void solve( tensor_engine< T >& );
        
        const T operator*() const;
        
    protected:
        tensor();
        
        T cache;
        
        union
        {
            void* p[ 2 ];
            T d[ 2 * sizeof( void* ) / sizeof( T ) ];
        };
    };
}

/******************************************************************************//******************************************************************************/

#endif


