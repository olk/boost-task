
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//  idea of node-base locking from 'C++ Concurrency in Action', Anthony Williams

#ifndef BOOST_FIBERS_qUEUE_H
#define BOOST_FIBERS_qUEUE_H

#include <cstddef>
#include <deque>
#include <stdexcept>
#include <utility>

#include <boost/atomic.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/config.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/move/move.hpp>
#include <boost/throw_exception.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>

#include <boost/task/detail/queue_op_status.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {
namespace detail {

template< typename T >
class queue : private noncopyable
{
public:
    typedef T   value_type;

private:
    enum state
    {
        OPEN = 0,
        CLOSED
    };

    atomic< state >             state_;
    std::deque< T >             queue_;
    mutable mutex               mtx_;
    condition_variable          not_empty_cond_;

    bool is_closed_() const
    { return CLOSED == state_; }

    void close_()
    {
        state_ = CLOSED;
        not_empty_cond_.notify_all();
    }

    bool is_empty_() const
    { return queue_.empty(); }

public:
    queue() :
        state_( OPEN),
        queue_(),
        mtx_(),
        not_empty_cond_()
    {}

    bool is_closed() const
    {
        mutex::scoped_lock lk( mtx_);
        return is_closed_();
    }

    void close()
    {
        mutex::scoped_lock lk( mtx_);
        close_();
    }

    bool is_empty() const
    {
        mutex::scoped_lock lk( mtx_);
        return is_empty_();
    }

    queue_op_status push( value_type const& va)
    {
        mutex::scoped_lock lk( mtx_);

        if ( is_closed_() ) return queue_op_status::closed;

        queue_.push_back( va);
        not_empty_cond_.notify_one();

        return queue_op_status::success;
    }

    queue_op_status pop( value_type & va)
    {
        mutex::scoped_lock lk( mtx_);

        while ( ! is_closed_() && is_empty_() ) not_empty_cond_.wait( lk);

        if ( is_closed_() && is_empty_() ) return queue_op_status::closed;
        BOOST_ASSERT( ! is_empty_() );

        try
        {
            va = boost::move( queue_.front() );
            queue_.pop_front();
            return queue_op_status::success;
        }
        catch (...)
        {
            close_();
            throw;
        }
    }

    queue_op_status try_pop( value_type & va)
    {
        mutex::scoped_lock lk( mtx_);

        if ( is_closed_() && is_empty_() ) return queue_op_status::closed;
        if ( is_empty_() ) return queue_op_status::empty;

        va = boost::move( queue_.front() );
        queue_.pop_front();
        return queue_op_status::success;
    }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_qUEUE_H
