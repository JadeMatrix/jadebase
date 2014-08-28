#ifndef BQT_METRICS_HPP
#define BQT_METRICS_HPP

/* 
 * bqt_metrics.hpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

#include "../bqt_log.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    typedef unsigned long metric_atom;
    static metric_atom NULL_METRIC = 0x00;
    
    metric_atom getMetricAtom( std::string metric_string );
    void clearMetricAtom( metric_atom metric );
    
    void setMetricDescription( metric_atom metric, std::string desc );
    void addMetricDescription( metric_atom metric, std::string desc );
    void clearMetricDescriptions( metric_atom metric );
    
    void setMetricValue( metric_atom metric, double val );
    void incrementMetricValue( metric_atom metric, double inc );
    
    void addSubMetric( metric_atom parent, metric_atom child );
    void removeSubMetric( metric_atom parent, metric_atom child );
    void clearSubMetrics( metric_atom metric );
    
    void printMetrics( metric_atom parent = NULL_METRIC );
    
    void cleanUpMetrics();
    
    // class metric_base
    // {
    // protected:
    // public:
        
    // };
    
    // template< class C > metric : public metric_base
    // {
    // protected:
    // public:
    // };
}

/******************************************************************************//******************************************************************************/

#endif


