#ifndef JADEBASE_SHAREDPOINTER_HPP
#define JADEBASE_SHAREDPOINTER_HPP

/* 
 * jb_sharedpointer.hpp
 * 
 * Simple, copy-time automatic memory management wrapper for objects that might
 * interface with a garbage-collected system.  These shared_ptrs should be
 * treated as raw data, and should never be passed by pointer, only copied or
 * passed by reference.  As such only reference counts are thread-safe; behavior
 * is undefined if the same shared_ptr object should be accessed by multiple
 * threads.
 * Also, a non-local reference should never be initialized from the result of
 * operator&() or any of the offset operators.
 * 
 * Note: shared_ptrs are thread-safe for creating & copying, but do not protect
 * their contents; that is the responsibility of their wrapped object.
 * 
 */

// TODO: Replace with std::shared_ptr after v1.0 with switch to C++11

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_exception.hpp"
#include "../threading/jb_mutex.hpp"
#include "../threading/jb_scopedlock.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    template< typename T > class shared_ptr
    {
    public:
        shared_ptr( T* );                                                       // Constructor for new shared_ptr; object must be unique; shared_ptr owns ptr
        // shared_ptr( shared_ptr< T >& );                                         // Copy constructor
        shared_ptr( const shared_ptr< T >& );                                   // Const copy constructor
        ~shared_ptr();
        
        T& operator*() const;                                                   // Dereference operator
        T* operator->() const;                                                  // Arrow operator
        shared_ptr< T >& operator=( shared_ptr< T >& );                         // Assignment operator
        shared_ptr< T >& operator=( T* );                                       // Assignment operator; object must be unique; shared_ptr owns pointer
        
        // Offset operators
        T* operator+( long ) const;
        T* operator-( long ) const;
        T& operator[]( long ) const;
        
        // Boolean operators also check for object/ref_count inconsistencies
        friend bool operator==( const shared_ptr< T >&, const shared_ptr< T >& );   // TODO: Declare as inline?
        friend bool operator!=( const shared_ptr< T >&, const shared_ptr< T >& );
        friend bool operator>( const shared_ptr< T >&, const shared_ptr< T >& );
        friend bool operator<( const shared_ptr< T >&, const shared_ptr< T >& );
        friend bool operator>=( const shared_ptr< T >&, const shared_ptr< T >& );
        friend bool operator<=( const shared_ptr< T >&, const shared_ptr< T >& );
    private:
        struct sptr_contents
        {
            T* object;
            int ref_count;
            mutex contents_mutex;                                               // Mutex is only locked for changes to ref_count
        };
        sptr_contents* contents;
    };
    
    template< typename T > shared_ptr< T >::shared_ptr( T* original )
    {
        if( original == NULL )
            throw exception( "shared_ptr<T>::shared_ptr(T*): Pointer NULL" );
        
        contents = new sptr_contents;
        
        contents -> object = original;
        contents -> ref_count = 1;
    }
    template< typename T > shared_ptr< T >::shared_ptr( const shared_ptr< T >& original )
    {
        scoped_lock< mutex > slock( original.contents -> contents_mutex );
        
        contents = original.contents;
        contents -> ref_count += 1;
    }
    template< typename T > shared_ptr< T >::~shared_ptr()
    {
        contents -> contents_mutex.lock();
        
        contents -> ref_count -= 1;
        
        if( contents -> ref_count == 0 )
        {
            delete contents -> object;
            
            contents -> contents_mutex.unlock();                                // Safe to do as we have no other references
            
            delete contents;
        }
        else
            contents -> contents_mutex.unlock();
    }
    
    template< typename T > T& shared_ptr< T >::operator*() const
    {
        return *( contents -> object );
    }
    template< typename T > T* shared_ptr< T >::operator->() const
    {
        return contents -> object;
    }
    template< typename T > shared_ptr< T >& shared_ptr< T >::operator=( shared_ptr< T >& original )
    {
        // Swap in new contents
        
        sptr_contents* old_contents = contents;
        
        scoped_lock< mutex > slock( original.contents -> contents_mutex );
        
        contents = original.contents;
        
        contents -> ref_count += 1;
        
        // Conditionally delete old contents (do this second in case new & old are the same)
        
        old_contents -> contents_mutex.lock();
        
        old_contents -> ref_count -= 1;
        
        if( old_contents -> ref_count == 0 )
        {
            delete contents -> object;
            
            old_contents -> contents_mutex.unlock();                            // Safe to do as we have no other references
            
            delete old_contents;
        }
        else
            old_contents -> contents_mutex.unlock();
        
        // Return
        
        return *this;
    }
    template< typename T > shared_ptr< T >& shared_ptr< T >::operator=( T* original )
    {
        if( original == NULL )
            throw exception( "shared_ptr<T>::operator=(T*): Pointer NULL" );
        
        // Create new contents
        
        sptr_contents* old_contents = contents;
        
        contents = new sptr_contents;
        
        contents -> object = original;
        contents -> ref_count = 1;
        
        // Conditionally delete old contents (do this second in case new & old are the same)
        
        old_contents -> contents_mutex.lock();
        
        old_contents -> ref_count -= 1;
        
        if( old_contents -> ref_count == 0 )
        {
            delete contents -> object;
            
            old_contents -> contents_mutex.unlock();                            // Safe to do as we have no other references
            
            delete old_contents;
        }
        else
            old_contents -> contents_mutex.unlock();
        
        // Return
        
        return *this;
    }
    
    template< typename T > T* shared_ptr< T >::operator+( long offset ) const
    {
        return ( contents -> object ) + offset;
    }
    template< typename T > T* shared_ptr< T >::operator-( long offset ) const
    {
        return ( contents -> object ) - offset;
    }
    template< typename T > T& shared_ptr< T >::operator[]( long offset ) const
    {
        return ( contents -> object )[ offset ];
    }
    
    template< typename T > inline bool operator==( const shared_ptr< T >& a, const shared_ptr< T >& b )
    {
        scoped_lock< mutex > alock( a.contents -> contents_mutex );             // ==, >, and < reference ref_count so they need to lock
        scoped_lock< mutex > block( b.contents -> contents_mutex );
        
        if( a.contents -> object == b.contents -> object )
        {
            if( a.contents -> ref_count == b.contents -> ref_count )
                return true;
            else
                throw exception( "operator==( const shared_ptr<>&, const shared_ptr<>& ): Pointer same but count different" );
        }
        else
            return false;
    }
    template< typename T > inline bool operator!=( const shared_ptr< T >& a, const shared_ptr< T >& b )
    {
        return !( a == b );
    }
    template< typename T > inline bool operator>( const shared_ptr< T >& a, const shared_ptr< T >& b )
    {
        scoped_lock< mutex > alock( a.contents -> contents_mutex );
        scoped_lock< mutex > block( b.contents -> contents_mutex );
        
        if( a.contents -> object == b.contents -> object
            && a.contents -> ref_count != b.contents -> ref_count )
        {
            throw exception( "operator>( const shared_ptr<>&, const shared_ptr<>& ): Pointer same but count different" );
        }
        else
            return a.contents -> object > b.contents -> object;
    }
    template< typename T > inline bool operator<( const shared_ptr< T >& a, const shared_ptr< T >& b )
    {
        scoped_lock< mutex > alock( a.contents -> contents_mutex );
        scoped_lock< mutex > block( b.contents -> contents_mutex );
        
        if( a.contents -> object == b.contents -> object
            && a.contents -> ref_count != b.contents -> ref_count )
        {
            throw exception( "operator<( const shared_ptr<>&, const shared_ptr<>& ): Pointer same but count different" );
        }
        else
            return a.contents -> object < b.contents -> object;
    }
    template< typename T > inline bool operator>=( const shared_ptr< T >& a, const shared_ptr< T >& b )
    {
        return !( a < b );
    }
    template< typename T > inline bool operator<=( const shared_ptr< T >& a, const shared_ptr< T >& b )
    {
        return !( a > b );
    }
}

/******************************************************************************//******************************************************************************/

#endif


