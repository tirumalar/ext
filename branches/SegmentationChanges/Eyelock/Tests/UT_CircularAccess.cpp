/*
 * UT_CircularAccess.cpp
 *
 *  Created on: 6 Jan, 2009
 *      Author: akhil
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>
#include "CircularAccess.h"


namespace tut {

struct CircularAccessTestData {
	CircularAccessTestData() {}
	~CircularAccessTestData() {}
};


typedef test_group<CircularAccessTestData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group("Circular Access TESTS");
}

namespace tut {
template<>
template<>
void testobject::test<1>() {
	set_test_name("Circular Access test 1");
	int arr[5];
	CircularAccess< int > ca;
	ca(5);
	ensure_equals("curPos 0",ca.curPos(),0);
	for(int i=0;i<6;i++) {
		ca[i]=i;
		ensure_equals("curPos",ca[i],i);

		ca++;
		ensure_equals("curPos",ca.curPos(),(i+1)%5);
	}
	ensure_equals("curPos 1 ",ca.curPos(),1);
	ensure_equals("getPrev",(ca.getPrev()),5);
	ensure_equals("getCurrent",(ca.getCurr()),1);
	ensure_equals("getNext",(ca.getNext()),2);

	ca--;
	ensure_equals("curPos 0 again",ca.curPos(),0);
	ensure_equals("getPrev after --",(ca.getPrev()),4);
	ensure_equals("getCurrent after --",(ca.getCurr()),5);
	ensure_equals("getNext after --",(ca.getNext()),1);

	--ca;
	ensure_equals("curPos 4",ca.curPos(),4);

	++ca;
	ensure_equals("curPos 0 again again",ca.curPos(),0);

	ensure_equals("getPrev(2)",ca.getPrev(2),3);
	ensure_equals("getPrev(5)",ca.getPrev(5),5);
	ensure_equals("getPrev(6)",ca.getPrev(6),4);


	//direct access

	ensure_equals("direct access test 1",ca[4],4);
	ensure_equals("direct access test 2",ca[0],5);
	ensure_equals("direct access test 3",ca[5],5);

	//relative usage prevOf
	ensure_equals("relative access test 1",ca.getPrevOf(4,0),4);
	ensure_equals("relative access test 2",ca.getPrevOf(4,1),3);
	ensure_equals("relative access test 3",ca.getPrevOf(4,7),2);
	ensure_equals("relative access test 4",ca.getPrevOf(4,5),4);

	// check pointer usage

	CircularAccess< int> *pCA = &(ca);
	ensure_equals("direct access test 1",(*pCA)[4],4);
	unsigned int umaxint = 0xffffffff;
	umaxint++;
	printf("umaxint: %#x\n",umaxint);
	ensure_equals("direct access test with negative index",ca[umaxint],5);

	int maxint = 0x7fffffff;
	maxint++;
	printf("maxint: %#x\n",maxint);
	// following will throw asetion
	//ensure_equals("direct access test with negative index",ca[maxint],0);
}

template<>
template<>
void testobject::test<2>() {
	set_test_name("Circular Access test 2");
	int arr[5];
	CircularAccess< int > ca;
	ca(5);
	ensure_equals("curPos 0",ca.curPos(),0);
	for(int i=0;i<5;i++) {
		ca[i]=i;
		ensure_equals("curPos",ca[i],i);

		ca++;
		ensure_equals("curPos",ca.curPos(),(i+1)%5);
	}
	citerator<CircularAccess< int >,int> it=ca;
	ensure_equals(it.curr(),0);

	for(int i=0;i<6;i++) {
		ensure_equals("itr.curPos",it.curr(),i%5);
		it.next();
	}
	}

}
