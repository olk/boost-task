
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>
#include <string>

#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/utility.hpp>

#include <boost/task/worker.hpp>

#include <cstdio>

int fn1()
{
    return 7;
}

void test_move()
{
    boost::tasks::worker w1;
    BOOST_CHECK( ! w1);
    boost::tasks::worker w2( 0);
    BOOST_CHECK( w2);
    w1 = boost::move( w2);
    BOOST_CHECK( w1);
    BOOST_CHECK( ! w2);
}

void test_id()
{
    boost::tasks::worker w1;
    boost::tasks::worker w2( 0);
    BOOST_CHECK( ! w1);
    BOOST_CHECK( w2);

    BOOST_CHECK_EQUAL( boost::tasks::worker::id(), w1.get_id() );
    BOOST_CHECK( boost::tasks::worker::id() != w2.get_id() );

    boost::tasks::worker w3( 0);
    BOOST_CHECK( w2.get_id() != w3.get_id() );

    w1 = boost::move( w2);
    BOOST_CHECK( w1);
    BOOST_CHECK( ! w2);

    BOOST_CHECK( boost::tasks::worker::id() != w1.get_id() );
    BOOST_CHECK_EQUAL( boost::tasks::worker::id(), w2.get_id() );
}

void test_push()
{
    boost::tasks::worker w( 0);
    boost::fibers::future< int > f1 = w.push( boost::bind( fn1) );
    int result1 = f1.get();
    BOOST_CHECK_EQUAL( 7, result1);
}

boost::unit_test::test_suite * init_unit_test_suite( int, char* [])
{
    boost::unit_test::test_suite * test =
        BOOST_TEST_SUITE("Boost.Task: worker test suite");

    test->add( BOOST_TEST_CASE( & test_move) );
    test->add( BOOST_TEST_CASE( & test_id) );
    for ( int i = 0; i < 50; ++i)
    {
     test->add( BOOST_TEST_CASE( & test_push) );
    }

    return test;
}
