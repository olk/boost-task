
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_DETAIL_CONFIG_H
#define BOOST_TASKS_DETAIL_CONFIG_H

#include <boost/chrono/system_clocks.hpp>
#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>

#ifdef BOOST_TASKS_DECL
# undef BOOST_TASKS_DECL
#endif

#if (defined(BOOST_ALL_DYN_LINK) || defined(BOOST_TASKS_DYN_LINK) ) && ! defined(BOOST_TASKS_STATIC_LINK)
# if defined(BOOST_TASKS_SOURCE)
#  define BOOST_TASKS_DECL BOOST_SYMBOL_EXPORT
#  define BOOST_TASKS_BUILD_DLL
# else
#  define BOOST_TASKS_DECL BOOST_SYMBOL_IMPORT
# endif
#endif

#if ! defined(BOOST_TASKS_DECL)
# define BOOST_TASKS_DECL
#endif

#if ! defined(BOOST_TASKS_SOURCE) && ! defined(BOOST_ALL_NO_LIB) && ! defined(BOOST_TASKS_NO_LIB)
# define BOOST_LIB_NAME boost_task
# if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_TASKS_DYN_LINK)
#  define BOOST_DYN_LINK
# endif
# include <boost/config/auto_link.hpp>
#endif

}}

#endif // BOOST_TASKS_DETAIL_CONFIG_H
