#ifndef JADEBASE_CONTAINER_HPP
#define JADEBASE_CONTAINER_HPP

/* 
 * jb_container.hpp
 * 
 * Template class for soft pointers, called containers
 * Containers are kept track of by the object that they reference;  when that
 * object is deleted, it sets its pointer to NULL within each container.
 * 
 * Note that for safety's sake, all containers owned by an object should be
 * cleared no later than the beginning of that object's destructor.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <set>

#include "../threading/jb_mutex.hpp"
#include "../threading/jb_scopedlock.hpp"
#include "../utility/jb_exception.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    // Pre-C++11 friend workaround
    template< typename T > class container;
    template< typename T > void clear( container< T >& );                       // External clear function
    
    template< typename T > class container                                      // T is only required to have T::register_container( container< T >* ) and
                                                                                // T::deregister_container( container< T >* )
    {
        // C++11 allows us to do this:
        // friend T;                                                               // So that the parent type T can call container::clear()
        // But until we use C++11 we have to use this:
        friend void clear< T >( container< T >& );
    public:
        container( T* );
        ~container();
        
        T* acquire();                                                           // Acquiring multiple times on the same thread is illegal
        void release();
    protected:
        mutex container_mutex;
        bool acquired;
        T* parent;
        
        void clear();
    };
    
    template< typename T > class scoped_lock< container< T > >                  // scoped_lock template specialization for container<>s (I guess?)
    // template<> template< typename T > class scoped_lock< container< T > >
    {
    private:
        container< T >& slc;
        T* ptr;
    public:
        scoped_lock( container< T >& c ) : slc( c )
        {
            ptr = slc.acquire();
        }
        ~scoped_lock()
        {
            slc.release();
        }
        T* operator*()
        {
            return ptr;
        }
    };
    
    // WARNING: Not really usable because of destructor chaining:
    // container_tracker's destructor is called AFTER the inheriting class's,
    // potentially invalidating the data we want to protect with container<>.
    // Just use this as an example.
    
    // template< typename T > class container_tracker                              // Utility virtual class for creating classes that can be put in containers
    //                                                                             // (note that it is not required to use this class).
    //                                                                             // Usage: class foo : public container_tracker< foo >
    // {
    //     friend container< T >;                                                  // C++11
    // public:
    //     virtual ~container_tracker();
        
    //     void register_container( container< T >* );                             // Can make this protected/private in C++11
    //     void deregister_container( container< T >* );                           // Can make this protected/private in C++11
    // protected:
    //     mutex ct_mutex;
    //     std::set< container< T >* > containers;
    // }
    
    // CONTAINER<> /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    template< typename T > void clear( container< T >& c )                      // Pre-C++11 workaround
    {
        c.clear();
    }
    
    template< typename T > container< T >::container( T* p )
    {
        parent = p;
        if( parent == NULL )
            throw exception( "container<>::container(): Pointer NULL" );
        acquired = false;
        parent -> register_container( this );
    }
    template< typename T > container< T >::~container()
    {
        if( acquired )
            throw exception( "container<>::~container(): Still acquired on destruction" );
        if( parent != NULL )
            parent -> deregister_container( this );
    }
    
    template< typename T > T* container< T >::acquire()
    {
        container_mutex.lock();
        if( acquired )
            throw exception( "container<>::acquire(): Already acquired" );
        acquired = true;
        return parent;
    }
    template< typename T > void container< T >::release()
    {
        scoped_lock< mutex > slock( container_mutex );                          // This is OK as jade::mutex allows recursive locking
        if( !acquired )
            throw exception( "container<>::release(): Not acquired" );
        acquired = false;
        container_mutex.unlock();
    }
    
    template< typename T > void container< T >::clear()
    {
        scoped_lock< mutex > slock( container_mutex );
        if( acquired )
            throw exception( "container<>::~container(): Still acquired on clear" );
        parent = NULL;
    }
    
    // CONTAINER_TRACKER<> /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // template< typename T > container_tracker< T >::~container_tracker()
    // {
    //     for( std::set< container< T >* >::iterator iter = containers.begin();
    //          iter != containers.end();
    //          ++iter )
    //     {
    //         ( *iter ) -> clear();
    //     }
    // }
    
    // template< typename T > void container_tracker< T >::register_container( container< T >* c )
    // {
    //     scoped_lock< mutex > slock( ct_mutex );
    //     containers.insert( c );
    // }
    // template< typename T > void container_tracker< T >::deregister_container( container< T >* c )
    // {
    //     scoped_lock< mutex > slock( ct_mutex );
    //     if( !containers.erase( c ) )                                            // Might as well throw an exception as something's wrong in the calling code
    //         throw exception( "container_tracker<>::deregister_container(): Container not a child" );
    // }
}

/******************************************************************************//******************************************************************************/

#endif


