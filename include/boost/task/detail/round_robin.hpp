
//   Copyright Christopher M. Kohlhoff, Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_DETAIL_ROUND_ROBIN_HPP
#define BOOST_TASKS_DETAIL_ROUND_ROBIN_HPP

#include <deque>
#include <utility>

#include <boost/asio/io_service.hpp>
#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/fiber/algorithm.hpp>
#include <boost/fiber/detail/config.hpp>
#include <boost/fiber/detail/fiber_base.hpp>
#include <boost/fiber/detail/main_notifier.hpp>
#include <boost/fiber/detail/notify.hpp>
#include <boost/fiber/detail/spinlock.hpp>
#include <boost/thread/lock_types.hpp> 

#include <boost/task/detail/config.hpp> 

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

# if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable:4251 4275)
# endif

namespace boost {
namespace tasks {
namespace detail {

class BOOST_TASKS_DECL round_robin : public fibers::algorithm
{
private:
    struct schedulable
    {
        fibers::detail::fiber_base::ptr_t       f;
        fibers::clock_type::time_point          tp;
        boost::asio::io_service::work           w;

        schedulable( fibers::detail::fiber_base::ptr_t const& f_,
                     fibers::clock_type::time_point const& tp_,
                     boost::asio::io_service::work const& w_) :
            f( f_), tp( tp_), w( w_)
        { BOOST_ASSERT( f); }

        schedulable( fibers::detail::fiber_base::ptr_t const& f_,
                     boost::asio::io_service::work const& w_) :
            f( f_), tp( (fibers::clock_type::time_point::max)() ), w( w_)
        { BOOST_ASSERT( f); }
    };

    typedef std::deque< schedulable >                   wqueue_t;

    boost::asio::io_service         &   io_svc_;
    fibers::detail::fiber_base::ptr_t   active_fiber_;
    wqueue_t                            wqueue_;
    fibers::detail::main_notifier       mn_;

    void evaluate_( fibers::detail::fiber_base::ptr_t const&);

public:
    round_robin( boost::asio::io_service & svc) BOOST_NOEXCEPT;

    ~round_robin() BOOST_NOEXCEPT;

    void spawn( fibers::detail::fiber_base::ptr_t const&);

    void priority( fibers::detail::fiber_base::ptr_t const&, int) BOOST_NOEXCEPT;

    void join( fibers::detail::fiber_base::ptr_t const&);

    fibers::detail::fiber_base::ptr_t active() BOOST_NOEXCEPT
    { return active_fiber_; }

    bool run();

    void wait( unique_lock< fibers::detail::spinlock > &);
    bool wait_until( fibers::clock_type::time_point const&,
                     unique_lock< fibers::detail::spinlock > &);

    void yield();

    fibers::detail::fiber_base::id get_main_id()
    { return fibers::detail::fiber_base::id( fibers::detail::main_notifier::make_pointer( mn_) ); }

    fibers::detail::notify::ptr_t get_main_notifier()
    { return fibers::detail::notify::ptr_t( new fibers::detail::main_notifier() ); }

    wqueue_t waiting()
    { return wqueue_; }
};

}}}

# if defined(BOOST_MSVC)
# pragma warning(pop)
# endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_DETAIL_ROUND_ROBIN_HPP
