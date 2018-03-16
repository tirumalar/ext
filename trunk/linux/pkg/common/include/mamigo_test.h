/*
 * mamigo_test.h
 *
 *  Created on: 19-Jul-2010
 *      Author: mamigo
 */

#ifndef MAMIGO_TEST_H_
#define MAMIGO_TEST_H_

#include <tut/tut.hpp>

#define MAMIGO_TEST_HEADER( TYPE) namespace tut \
{ \
	struct TYPE;	\
	typedef	test_group<TYPE> tg; \
	typedef tg::object testobject; \
}\
namespace {\
tut::tg test_group( #TYPE);\
}

#define MAMIGO_TEST_HEADER_MAX( TYPE,MAXTESTS) namespace tut \
{ \
	struct TYPE;	\
	typedef	test_group<TYPE,MAXTESTS> tg; \
	typedef tg::object testobject; \
}\
namespace {\
tut::tg test_group( #TYPE);\
}

#endif /* MAMIGO_TEST_H_ */
