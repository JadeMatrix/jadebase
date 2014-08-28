/* 
 * bqt_metrics.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_metrics.hpp"

#include <vector>
#include <map>
#include <queue>
#include <set>

#include "../threading/bqt_mutex.hpp"
#include "../bqt_exception.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    bqt::mutex metrics_mutex;
    
    struct metric_data
    {
        const std::string* name;
        std::vector< std::string > descriptions;
        double value;
        
        std::set< bqt::metric_atom > children;
    };
    
    std::map<   std::string,      bqt::metric_atom > string_metric_map;
    std::map<   bqt::metric_atom, metric_data      > metric_data_map;
    std::queue< bqt::metric_atom                   > reusable_atoms;
    bqt::metric_atom                                 last_new_atom = bqt::NULL_METRIC;
    
    std::string tab = "    ";
    void printMetrics_recursive( const bqt::metric_atom& metric,
                                 int indent,
                                 std::set< bqt::metric_atom >& printed )
    {
        metric_data& data = metric_data_map[ metric ];
        
        {
            std::string msg;
            
            std::string padding;
            for( int i = 0; i < indent; ++i )
                ff::write( padding, tab );
            
            if( printed.count( metric ) )
            {
                ff::write( msg,
                           padding,
                           *data.name,
                           " (",
                           ff::to_x( metric ),
                           ") ...\n" );
                return;
            }
            else
            {
                ff::write( msg,
                           padding,
                           *data.name,
                           " (",
                           ff::to_x( metric ),
                           "): ",
                           data.value,
                           "\n" );
                
                int indent_width = ( indent + 1 ) * 4;
                for( int i = 0; i < data.descriptions.size(); ++i )
                {
                    for( int j = 0; j < data.descriptions[ i ].size(); /* empty */ )
                    {
                        int desc_width = indent_width > 40 ? 40 : 80 - indent_width;
                        
                        ff::write( msg,
                                   padding,
                                   j == 0 ? "  - " : "    ",
                                   data.descriptions[ i ].substr( j, desc_width ),
                                   "\n" );
                        
                        j += desc_width;
                    }
                }
                
                printed.insert( metric );
            }
            
            ff::write( bqt_out, msg );
        }
        
        for( std::set< bqt::metric_atom >::iterator iter = data.children.begin();
             iter != data.children.end();
             ++iter )
        {
            printMetrics_recursive( *iter, indent + 1, printed );
        }
    }
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    metric_atom getMetricAtom( std::string metric_string )
    {
        scoped_lock< mutex > slock( metrics_mutex );
        
        if( !string_metric_map.count( metric_string ) )
        {
            if( reusable_atoms.empty() )
                string_metric_map[ metric_string ] = ++last_new_atom;
            else
            {
                string_metric_map[ metric_string ] = reusable_atoms.front();
                
                reusable_atoms.pop();
            }
            
            metric_data_map[ reusable_atoms.front() ].name = &( string_metric_map.find( metric_string ) -> first );
        }
        
        return string_metric_map[ metric_string ];
    }
    void clearMetricAtom( metric_atom metric )
    {
        scoped_lock< mutex > slock( metrics_mutex );
        
        if( !metric_data_map.count( metric ) )
            throw exception( "clearMetricAtom(): No such metric" );
        
        string_metric_map.erase( *metric_data_map[ metric ].name );
        metric_data_map.erase( metric );
        reusable_atoms.push( metric );
    }
    
    void setMetricDescription( metric_atom metric, std::string desc )
    {
        scoped_lock< mutex > slock( metrics_mutex );
        
        if( !metric_data_map.count( metric ) )
            throw exception( "setMetricDescription(): No such metric" );
        
        metric_data& data( metric_data_map[ metric ] );
        
        data.descriptions.clear();
        data.descriptions.push_back( desc );
    }
    void addMetricDescription( metric_atom metric, std::string desc )
    {
        scoped_lock< mutex > slock( metrics_mutex );
        
        if( !metric_data_map.count( metric ) )
            throw exception( "addMetricDescription(): No such metric" );
        
        metric_data_map[ metric ].descriptions.push_back( desc );
    }
    void clearMetricDescriptions( metric_atom metric )
    {
        scoped_lock< mutex > slock( metrics_mutex );
        
        if( !metric_data_map.count( metric ) )
            throw exception( "clearMetricDescriptions(): No such metric" );
        
        metric_data_map[ metric ].descriptions.clear();
    }
    
    void setMetricValue( metric_atom metric, double val )
    {
        scoped_lock< mutex > slock( metrics_mutex );
        
        if( !metric_data_map.count( metric ) )
            throw exception( "setMetricValue(): No such metric" );
        
        metric_data_map[ metric ].value = val;
    }
    void incrementMetricValue( metric_atom metric, double inc )
    {
        scoped_lock< mutex > slock( metrics_mutex );
        
        if( !metric_data_map.count( metric ) )
            throw exception( "incrementMetricValue(): No such metric" );
        
        metric_data_map[ metric ].value += inc;
    }
    
    void addSubMetric( metric_atom parent, metric_atom child )
    {
        scoped_lock< mutex > slock( metrics_mutex );
        
        if( !metric_data_map.count( parent ) )
            throw exception( "addSubMetric(): No such metric (parent)" );
        if( !metric_data_map.count( child ) )
            throw exception( "addSubMetric(): No such metric (child)");
        
        metric_data_map[ parent ].children.insert( child );
    }
    void removeSubMetric( metric_atom parent, metric_atom child )
    {
        scoped_lock< mutex > slock( metrics_mutex );
        
        if( !metric_data_map.count( parent ) )
            throw exception( "removeSubMetric(): No such metric (parent)" );
        if( !metric_data_map.count( child ) )
            throw exception( "addSubMetric(): No such metric (child)");
        
        metric_data_map[ parent ].children.erase( child );
    }
    void clearSubMetrics( metric_atom metric )
    {
        scoped_lock< mutex > slock( metrics_mutex );
        
        if( !metric_data_map.count( metric ) )
            throw exception( "clearSubMetrics(): No such metric" );
        
        metric_data_map[ metric ].children.clear();
    }
    
    void printMetrics( metric_atom parent )
    {
        scoped_lock< mutex > slock( metrics_mutex );
        
        if( parent != NULL_METRIC && !metric_data_map.count( parent ) )
            throw exception( "printMetrics(): No such metric" );
        
        std::set< bqt::metric_atom > printed;
        
        if( parent == NULL_METRIC )
        {
            for( std::map< metric_atom, metric_data >::iterator iter = metric_data_map.begin();
                 iter != metric_data_map.end();
                 ++iter )
            {
                printMetrics_recursive( iter -> first, 0, printed );
            }
        }
        else
        {
            printMetrics_recursive( parent, 0, printed );
        }
    }
    
    void cleanUpMetrics()
    {
        scoped_lock< mutex > slock( metrics_mutex );
        
        for( std::map< metric_atom, metric_data >::iterator iter = metric_data_map.begin();
                         iter != metric_data_map.end();
                         ++iter )
        {
            clearMetricAtom( iter -> first );
        }
    }
}


