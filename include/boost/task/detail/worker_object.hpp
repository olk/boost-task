
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_DETAIL_WORKER_OBJECT_H
#define BOOST_TASKS_DETAIL_WORKER_OBJECT_H

#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/config.hpp>
#include <boost/thread/thread.hpp>

#include <boost/task/detail/worker_base.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {
namespace detail {

template< typename Allocator >
class worker_object : public worker_base
{
public:
    typedef typename Allocator::template rebind<
        worker_object< Allocator >
    >::other                                    allocator_t;

private:
    allocator_t         alloc_;

    static void destroy_( allocator_t & alloc, worker_object * p)
    {
        alloc.destroy( p);
        alloc.deallocate( p, 1);
    }

    worker_object( worker_object const&); // = delete
    worker_object & operator=( worker_object const&); // = delete

protected:
    void deallocate_object()
    { destroy_( alloc_, this); }

public:
    worker_object( allocator_t const& alloc) BOOST_NOEXCEPT :
        alloc_( alloc)
    { thrd_ = thread( bind( & worker_base::start_worker_, this) ); }

    ~worker_object() BOOST_NOEXCEPT
    {}
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_DETAIL_WORKER_OBJECT_H
