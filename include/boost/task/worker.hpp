
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_WORKER_H
#define BOOST_TASKS_WORKER_H

#include <memory>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/fiber/future.hpp>
#include <boost/move/move.hpp>
#include <boost/thread/detail/memory.hpp> // boost::allocator_arg_t
#include <boost/utility/result_of.hpp>

#include <boost/task/detail/worker_base.hpp>
#include <boost/task/detail/worker_object.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {

class worker
{
private:
    struct dummy
    { void nonnull() {} };

    typedef void ( dummy::*safe_bool)();

    detail::worker_base::ptr_t  impl_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( worker);

public:
    typedef detail::worker_base::id     id;

    worker() BOOST_NOEXCEPT :
        impl_()
    {}

    worker( int/*pin_to_core*/) :
        impl_()
    {
        std::allocator< worker > alloc;
        typedef detail::worker_object<
            std::allocator< worker >
        >                                   object_t;
        object_t::allocator_t a( alloc);
        // placement new
        impl_ = detail::worker_base::ptr_t(
                ::new( a.allocate( 1) ) object_t( a/*, pin_to_core*/) );
    }

    template< typename Allocator >
    worker( boost::allocator_arg_t, Allocator const& alloc/*, pin_to_core*/) :
        impl_()
    {
        typedef detail::worker_object<
            Allocator
        >                                   object_t;
        typename object_t::allocator_t a( alloc);
        // placement new
        impl_ = detail::worker_base::ptr_t(
                ::new( a.allocate( 1) ) object_t( a/*, pin_to_core*/) );
    }

    ~worker() BOOST_NOEXCEPT
    {}

    worker( BOOST_RV_REF( worker) other) BOOST_NOEXCEPT :
        impl_()
    { swap( other); }

    worker & operator=( BOOST_RV_REF( worker) other) BOOST_NOEXCEPT
    {
        worker tmp( move( other) );
        swap( tmp);
        return * this;
    }

    operator safe_bool() const BOOST_NOEXCEPT
    { return impl_ && ! impl_->is_closed() ? & dummy::nonnull : 0; }

    bool operator!() const BOOST_NOEXCEPT
    { return ! impl_ || impl_->is_closed(); }

    void swap( worker & other) BOOST_NOEXCEPT
    { impl_.swap( other.impl_); }

    bool is_closed() const BOOST_NOEXCEPT
    {
        BOOST_ASSERT( impl_);

        return impl_->is_closed();
    }

    void close() BOOST_NOEXCEPT
    {
        BOOST_ASSERT( impl_);

        impl_->close();
    }

    id get_id() const BOOST_NOEXCEPT
    { return impl_ ? impl_->get_id() : id(); }

    template< typename Fn >
    fibers::future< typename result_of< Fn() >::type >
    push( Fn fn)
    {
        BOOST_ASSERT( impl_);

        return impl_->push( fn);
    }
#if 0
    template< typename Fn >
    fibers::future< typename result_of< Fn() >::type >
    push( BOOST_RV_REF( Fn) fn)
    {
        BOOST_ASSERT( impl_);

        return impl_->push( move( fn) );
    }
#endif
};

inline
void swap( worker & l, worker & r)
{ return l.swap( r); }

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_WORKER_H
