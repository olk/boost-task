
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <sstream>
#include <string>

#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/utility.hpp>

#include <boost/task/worker.hpp>

int fn1()
{
    return 7;
}

int main( int argc, char * argv[])
{
    for ( int i = 0; i < 5000; ++i)
    {
        std::cout << ".";
        boost::tasks::worker w( 0);
        boost::fibers::future< int > f1 = w.push( boost::bind( fn1) );
        int result1 = f1.get();
        BOOST_ASSERT( 7 == result1);
    }
    std::cout << std::endl;

    return 0;
}
