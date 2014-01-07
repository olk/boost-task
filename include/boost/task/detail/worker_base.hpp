
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
#include <boost/fiber/asio/spawn.hpp>
#include <boost/fiber/future.hpp>
#include <boost/fiber/operations.hpp>
#include <boost/fiber/round_robin.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/move/move.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility/result_of.hpp>

#include <boost/task/detail/queue.hpp>
#include <boost/task/detail/round_robin.hpp>

#include <cstdio>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {
namespace detail {

template< typename PT >
void run_as_fiber( PT pt)
{ fibers::fiber( move( * pt) ).detach(); }

class worker_base
{
private:
    template< typename Fn >
    class callable
    {
    private:
        typedef typename result_of< Fn() >::type    result_t;

        class impl
        {
        private:
            std::size_t                 use_count_;

        public:
            asio::io_service        &   io_svc;
            Fn                          fn;
            fibers::promise< result_t > pr;

            impl( asio::io_service & io_svc_, Fn fn_,
                  fibers::promise< result_t > & pr_) :
                use_count_( 0),
                io_svc( io_svc_),
                fn( fn_),
                pr( move( pr_) )
            {}

            friend inline void intrusive_ptr_add_ref( impl * p) BOOST_NOEXCEPT
            { ++p->use_count_; }

            friend inline void intrusive_ptr_release( impl * p)
            { if ( 0 == --p->use_count_) delete p; }
        };

        intrusive_ptr< impl >           impl_;

        void run_( fibers::asio::yield_context yield)
        { impl_->pr.set_value( impl_->fn() ); }

    public:
        callable( asio::io_service & io_svc, Fn fn,
                  fibers::promise< result_t > & pr) :
            impl_( new impl( io_svc, fn, pr) )
        {}

        void operator()()
        {
            fibers::asio::spawn( impl_->io_svc,
                bind( & callable::run_, this, _1) );
        }
    };

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
        round_robin rr( io_svc_);
        fibers::set_scheduling_algorithm( & rr);

        while ( CLOSED != state_)
        {
            io_svc_.poll();
            while ( fibers::detail::scheduler::instance()->run() );
            if ( ! rr.waiting().empty() )
            {
                std::size_t size = rr.waiting().size();
            }
        }
#endif

        while ( CLOSED != state_)
        {
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
#if 0
        typedef fibers::promise< result_t >         promise_t;
        promise_t pr;
        future_t f( pr.get_future() );
        callable< Fn > c( io_svc_, fn, pr);
        io_svc_.post( c);
#endif
        typedef fibers::packaged_task< result_t() >   packaged_task_t;
        boost::shared_ptr< packaged_task_t > pt(
            new packaged_task_t( fn) );
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
