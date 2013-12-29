
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_DETAIL_WORKER_BASE_H
#define BOOST_TASKS_DETAIL_WORKER_BASE_H

#include <cstddef>

#include <boost/assert.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/atomic.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/config.hpp>
#include <boost/fiber/asio/round_robin.hpp>
#include <boost/fiber/future.hpp>
#include <boost/fiber/operations.hpp>
#include <boost/fiber/round_robin.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/move/move.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility/result_of.hpp>

#include <boost/task/detail/queue.hpp>

#include <cstdio>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {
namespace detail {

template< typename PT >
void run_as_fiber( PT pt)
{
    fibers::fiber( move( * pt) ).join();
}

class worker_base
{
private:
    enum state_t
    {
        OPEN = 0,
        CLOSED
    };

    atomic< std::size_t >   use_count_;
    atomic< state_t >       state_;
    asio::io_service        io_svc_;
    queue< function< void() > > queue_;

    worker_base( worker_base const&); // = delete
    worker_base & operator=( worker_base const&); // = delete

protected:
    thread                  thrd_;

    virtual void deallocate_object() = 0;

    void start_worker_()
    {
#if 0
        fibers::asio::round_robin rr( io_svc_);
        fibers::set_scheduling_algorithm( & rr);
#endif

        while ( CLOSED != state_)
        {
#if 0
            io_svc_.poll();
#endif
            function< void() > fn;
            if ( queue_op_status::success == queue_.try_pop( fn) )
            {
                fn();
                fn.clear();
            }
            while ( fibers::detail::scheduler::instance()->run() );
        }
    }

public:
    typedef intrusive_ptr< worker_base >    ptr_t;
    typedef thread::id                      id;

    worker_base() BOOST_NOEXCEPT :
        use_count_( 0),
        state_( OPEN),
        io_svc_(),
        queue_(),
        thrd_()
    {}

    virtual ~worker_base() BOOST_NOEXCEPT
    {
        close();
        if ( thrd_.joinable() ) thrd_.join();
    }

    bool is_closed() const BOOST_NOEXCEPT
    { return CLOSED == state_; }

    void close() BOOST_NOEXCEPT
    {
        state_.store( CLOSED);
        queue_.close();
        io_svc_.stop();
    }

    id get_id() const BOOST_NOEXCEPT
    { return thrd_.get_id(); }

    template< typename Fn >
    fibers::future< typename result_of< Fn() >::type >
    push( Fn fn)
    {
        typedef typename result_of< Fn() >::type    result_t;
        typedef fibers::future< result_t >          future_t;
        typedef fibers::packaged_task< result_t() > packaged_task_t;

        shared_ptr< packaged_task_t > pt( new packaged_task_t( fn) );
        future_t f( pt->get_future() );
        //io_svc_.post( bind( run_as_fiber< shared_ptr< packaged_task_t > >, pt) );
        queue_.push( bind( run_as_fiber< shared_ptr< packaged_task_t > >, pt) );

        return move( f);
    }
#if 0
    template< typename Fn >
    fibers::future< typename result_of< Fn() >::type >
    push( BOOST_RV_REF( Fn) fn)
    {
        typedef typename result_of< Fn() >::type    result_t;
        typedef fibers::future< result_t >          future_t;
        typedef fibers::packaged_task< result_t() > packaged_task_t;

        packaged_task_t pt( move( fn) );
        future_t f( pt.get_future() );
        io_svc_.post( bind( run_as_fiber< packaged_task_t >, move( pt) ) );

        return move( f);
    }
#endif
    friend inline void intrusive_ptr_add_ref( worker_base * p) BOOST_NOEXCEPT
    { ++p->use_count_; }

    friend inline void intrusive_ptr_release( worker_base * p)
    { if ( 0 == --p->use_count_) p->deallocate_object(); }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_DETAIL_WORKER_BASE_H
