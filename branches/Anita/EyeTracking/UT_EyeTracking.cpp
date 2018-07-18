#include <tut/tut.hpp>

#include "EyeTracker.h"
#include <set>

namespace tut
{

	struct EyeTrackerData
	{
		int w, h;

		EyeTrackerData() { init(); }
		~EyeTrackerData() { term(); }

		void init()
		{
		}
		void term(){
		}

	};


	typedef	test_group<EyeTrackerData> tg;
	tg test_group1("Eye Tracking TESTS");

	typedef tg::object testobject;

	template<>
	template<>
	void testobject::test<1>()
	{
		std::map<int, EyeInfo> eyeMap;
		int i[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

		EyeTracker et;

		EyeInfo ei;
		ei.x = 10; ei.y = 10; ei.score = 10; ei.handle = i;
		eyeMap[1] = ei;
		ei.x = 100; ei.y = 10; ei.score = 10; ei.handle = i+1;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 10; ei.handle = i+2;
		eyeMap[3] = ei;
		ei.x = 100; ei.y = 50; ei.score = 10; ei.handle = i+3;
		eyeMap[4] = ei;

		std::map<std::pair<int, int>, void * > result;

		result = et.Track(1, eyeMap);

		ensure("result must be empty", result.empty());

		eyeMap.clear();

		ei.x = 120; ei.y = 10; ei.score = 12; ei.handle = i+4;
		eyeMap[1] = ei;
		ei.x = 15; ei.y = 14; ei.score = 8; ei.handle = i+5;
		eyeMap[2] = ei;
		ei.x = 5; ei.y = 60; ei.score = 4; ei.handle = i+6;
		eyeMap[3] = ei;

		result = et.Track(2, eyeMap);

		ensure("result must be empty", result.empty());
		eyeMap.clear();

		ei.x = 10; ei.y = 10; ei.score = 10; ei.handle = i+7;
		eyeMap[1] = ei;
		ei.x = 100; ei.y = 10; ei.score = 2; ei.handle = i+8;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 2; ei.handle = i+9;
		eyeMap[3] = ei;
		ei.x = 100; ei.y = 50; ei.score = 2; ei.handle = i+10;
		eyeMap[4] = ei;

		result = et.Track(3, eyeMap);

		ensure_equals("result must be empty", result.size(), 1);
		ensure("result must be (2,2)", result.begin()->first == std::make_pair(2,2));
		ensure("handle must be i+5", result.begin()->second == i+5);
	}

	template<>
	template<>
	void testobject::test<2>()
	{
		std::map<int, EyeInfo> eyeMap;
		int i[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

		EyeTracker et(12, false);

		EyeInfo ei;
		ei.x = 10; ei.y = 10; ei.score = 10; ei.handle = i;
		eyeMap[1] = ei;
		ei.x = 100; ei.y = 10; ei.score = 20; ei.handle = i+1;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 30; ei.handle = i+2;
		eyeMap[3] = ei;
		ei.x = 100; ei.y = 50; ei.score = 40; ei.handle = i+3;
		eyeMap[4] = ei;

		std::map<std::pair<int, int>, void * > result;

		result = et.Track(1, eyeMap);

		ensure_equals("result must be 1", result.size(), 1);
		ensure("result must be (1,1)", result.begin()->first == std::make_pair(1,1));
		ensure("handle must be i", result.begin()->second == i);

		eyeMap.clear();

		ei.x = 120; ei.y = 10; ei.score = 12; ei.handle = i+4;
		eyeMap[1] = ei;
		ei.x = 15; ei.y = 14; ei.score = 18; ei.handle = i+5;
		eyeMap[2] = ei;
		ei.x = 5; ei.y = 60; ei.score = 14; ei.handle = i+6;
		eyeMap[3] = ei;

		result = et.Track(2, eyeMap);

		ensure("result must be empty", result.empty());
		eyeMap.clear();

		ei.x = 10; ei.y = 10; ei.score = 10; ei.handle = i+7;
		eyeMap[1] = ei;
		ei.x = 100; ei.y = 10; ei.score = 2; ei.handle = i+8;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 22; ei.handle = i+9;
		eyeMap[3] = ei;
		ei.x = 100; ei.y = 50; ei.score = 22; ei.handle = i+10;
		eyeMap[4] = ei;

		result = et.Track(3, eyeMap);

		std::map<std::pair<int, int>, void * >::iterator rit = result.begin();

		ensure_equals("result must be empty", result.size(), 2);
		ensure("result must be (3,1)", rit->first == std::make_pair(3,1));
		ensure("handle must be i+7", rit->second == i+7);

		rit++;

		ensure("result must be (3,2)", rit->first == std::make_pair(3,2));
		ensure("handle must be i+8", rit->second == i+8);

	}

	template<>
	template<>
	void testobject::test<3>()
	{
		std::map<int, EyeInfo> eyeMap;
		int i[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

		EyeTracker et(12, true, 2);

		EyeInfo ei;
		ei.x = 10; ei.y = 10; ei.score = 10; ei.handle = i;
		eyeMap[1] = ei;
		ei.x = 100; ei.y = 10; ei.score = 20; ei.handle = i+1;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 30; ei.handle = i+2;
		eyeMap[3] = ei;
		ei.x = 100; ei.y = 50; ei.score = 40; ei.handle = i+3;
		eyeMap[4] = ei;

		std::map<std::pair<int, int>, void * > result;

		result = et.Track(1, eyeMap);

		ensure("result must be empty", result.empty());


		eyeMap.clear();

		ei.x = 120; ei.y = 10; ei.score = 12; ei.handle = i+4;
		eyeMap[1] = ei;
		ei.x = 15; ei.y = 14; ei.score = 18; ei.handle = i+5;
		eyeMap[2] = ei;
		ei.x = 5; ei.y = 60; ei.score = 14; ei.handle = i+6;
		eyeMap[3] = ei;

		result = et.Track(2, eyeMap);

		ensure_equals("result must not be empty", result.size(), 1);
		ensure("result must be (1,1)", result.begin()->first == std::make_pair(1,1));
		ensure("handle must be i", result.begin()->second == i);

		eyeMap.clear();

		ei.x = 10; ei.y = 10; ei.score = 10; ei.handle = i+7;
		eyeMap[1] = ei;
		ei.x = 100; ei.y = 10; ei.score = 2; ei.handle = i+8;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 22; ei.handle = i+9;
		eyeMap[3] = ei;
		ei.x = 100; ei.y = 50; ei.score = 22; ei.handle = i+10;
		eyeMap[4] = ei;

		result = et.Track(3, eyeMap);

		std::map<std::pair<int, int>, void * >::iterator rit = result.begin();

		ensure_equals("result must be empty", result.size(), 2);
		ensure("result must be (3,1)", rit->first == std::make_pair(3,1));
		ensure("handle must be i+7", rit->second == i+7);

		rit++;

		ensure("result must be (3,2)", rit->first == std::make_pair(3,2));
		ensure("handle must be i+8", rit->second == i+8);

	}

	template<>
	template<>
	void testobject::test<4>()
	{
		std::map<int, EyeInfo> eyeMap;
		int i[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

		EyeTracker et(12, true, 3);

		EyeInfo ei;
		ei.x = 10; ei.y = 10; ei.score = 10; ei.handle = i;
		eyeMap[1] = ei;
		ei.x = 100; ei.y = 10; ei.score = 20; ei.handle = i+1;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 30; ei.handle = i+2;
		eyeMap[3] = ei;
		ei.x = 100; ei.y = 50; ei.score = 40; ei.handle = i+3;
		eyeMap[4] = ei;

		std::map<std::pair<int, int>, void * > result;

		result = et.Track(1, eyeMap);

		ensure("result must be empty", result.empty());


		eyeMap.clear();

		ei.x = 120; ei.y = 10; ei.score = 12; ei.handle = i+4;
		eyeMap[1] = ei;
		ei.x = 15; ei.y = 14; ei.score = 18; ei.handle = i+5;
		eyeMap[2] = ei;
		ei.x = 5; ei.y = 60; ei.score = 4; ei.handle = i+6;
		eyeMap[3] = ei;
		ei.x = 110; ei.y = 60; ei.score = 60; ei.handle = i+12;
		eyeMap[4] = ei;

		result = et.Track(2, eyeMap);

		ensure("result must be empty", result.empty());

		eyeMap.clear();

		ei.x = 10; ei.y = 10; ei.score = 22; ei.handle = i+7;
		eyeMap[1] = ei;
		ei.x = 100; ei.y = 10; ei.score = 2; ei.handle = i+8;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 22; ei.handle = i+9;
		eyeMap[3] = ei;
		ei.x = 100; ei.y = 50; ei.score = 4; ei.handle = i+10;
		eyeMap[4] = ei;

		result = et.Track(3, eyeMap);

		std::map<std::pair<int, int>, void * >::iterator rit = result.begin();

		ensure_equals("result must be empty", result.size(), 4);
		ensure("result must be (1,1)", rit->first == std::make_pair(1,1));
		ensure("handle must be i", rit->second == i);

		rit++;

		ensure("result must be (2,3)", rit->first == std::make_pair(2,3));
		ensure("handle must be i+6", rit->second == i+6);

		rit++;

		ensure("result must be (3,2)", rit->first == std::make_pair(3,2));
		ensure("handle must be i+8", rit->second == i+8);

		rit++;

		ensure("result must be (3,4)", rit->first == std::make_pair(3,4));
		ensure("handle must be i+10", rit->second == i+10);
	}

	template<>
	template<>
	void testobject::test<5>()
	{
		std::map<int, EyeInfo> eyeMap;
		int i[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

		EyeTracker et(12, true, 3);	 // running block

		EyeInfo ei;
		ei.x = 10; ei.y = 10; ei.score = 10; ei.handle = i;
		eyeMap[1] = ei;
		ei.x = 100; ei.y = 10; ei.score = 20; ei.handle = i+1;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 30; ei.handle = i+2;
		eyeMap[3] = ei;
		ei.x = 100; ei.y = 50; ei.score = 40; ei.handle = i+3;
		eyeMap[4] = ei;

		std::map<std::pair<int, int>, void * > result;

		result = et.Track(1, eyeMap);

		ensure("result must be empty", result.empty());


		eyeMap.clear();

		ei.x = 120; ei.y = 10; ei.score = 12; ei.handle = i+4;
		eyeMap[1] = ei;
		ei.x = 15; ei.y = 14; ei.score = 18; ei.handle = i+5;
		eyeMap[2] = ei;
		ei.x = 5; ei.y = 60; ei.score = 4; ei.handle = i+6;
		eyeMap[3] = ei;
		ei.x = 110; ei.y = 60; ei.score = 60; ei.handle = i+12;
		eyeMap[4] = ei;

		result = et.Track(2, eyeMap);

		ensure("result must be empty", result.empty());

		eyeMap.clear();

		ei.x = 10; ei.y = 10; ei.score = 22; ei.handle = i+7;
		eyeMap[1] = ei;
		ei.x = 100; ei.y = 10; ei.score = 2; ei.handle = i+8;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 22; ei.handle = i+9;
		eyeMap[3] = ei;
		ei.x = 100; ei.y = 50; ei.score = 4; ei.handle = i+10;
		eyeMap[4] = ei;

		result = et.Track(3, eyeMap);

		std::map<std::pair<int, int>, void * >::iterator rit = result.begin();

		ensure_equals("result must be empty", result.size(), 4);
		ensure("result must be (1,1)", rit->first == std::make_pair(1,1));
		ensure("handle must be i", rit->second == i);

		rit++;

		ensure("result must be (2,3)", rit->first == std::make_pair(2,3));
		ensure("handle must be i+6", rit->second == i+6);

		rit++;

		ensure("result must be (3,2)", rit->first == std::make_pair(3,2));
		ensure("handle must be i+8", rit->second == i+8);

		rit++;

		ensure("result must be (3,4)", rit->first == std::make_pair(3,4));
		ensure("handle must be i+10", rit->second == i+10);


		ei.x = 10; ei.y = 10; ei.score = 25; ei.handle = i;
		eyeMap[1] = ei;
		ei.x = 100; ei.y = 10; ei.score = 1; ei.handle = i+1;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 25; ei.handle = i+2;
		eyeMap[3] = ei;
		ei.x = 100; ei.y = 50; ei.score = 25; ei.handle = i+3;
		eyeMap[4] = ei;

		result = et.Track(4, eyeMap);

		rit = result.begin();

		ensure_equals("result must be empty", result.size(), 1);
		ensure("result must be (4,2)", rit->first == std::make_pair(4,2));
		ensure("handle must be i+1", rit->second == i+1);
	}

	template<>
	template<>
	void testobject::test<6>()
	{
		std::map<int, EyeInfo> eyeMap;
		int i[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

		EyeTracker et(12, false, 3);	// block

		EyeInfo ei;
		ei.x = 10; ei.y = 10; ei.score = 10; ei.handle = i;
		eyeMap[1] = ei;
		ei.x = 100; ei.y = 10; ei.score = 20; ei.handle = i+1;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 30; ei.handle = i+2;
		eyeMap[3] = ei;


		std::map<std::pair<int, int>, void * > result;

		result = et.Track(1, eyeMap);

		ensure("result must be empty", result.empty());


		eyeMap.clear();

		ei.x = 120; ei.y = 10; ei.score = 10; ei.handle = i+4;
		eyeMap[1] = ei;
		ei.x = 15; ei.y = 14; ei.score = 18; ei.handle = i+5;
		eyeMap[2] = ei;
		ei.x = 5; ei.y = 60; ei.score = 4; ei.handle = i+6;
		eyeMap[3] = ei;
		ei.x = 110; ei.y = 60; ei.score = 60; ei.handle = i+12;
		eyeMap[4] = ei;

		result = et.Track(2, eyeMap);

		ensure("result must be empty", result.empty());

		eyeMap.clear();


		ei.x = 100; ei.y = 10; ei.score = 2; ei.handle = i+8;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 22; ei.handle = i+9;
		eyeMap[3] = ei;
		ei.x = 100; ei.y = 50; ei.score = 4; ei.handle = i+10;
		eyeMap[4] = ei;

		result = et.Track(3, eyeMap);

		std::map<std::pair<int, int>, void * >::iterator rit = result.begin();

		ensure_equals("result must be empty", result.size(), 4);
		ensure("result must be (1,1)", rit->first == std::make_pair(1,1));
		ensure("handle must be i", rit->second == i);

		rit++;

		ensure("result must be (2,3)", rit->first == std::make_pair(2,3));
		ensure("handle must be i+6", rit->second == i+6);

		rit++;

		ensure("result must be (3,2)", rit->first == std::make_pair(3,2));
		ensure("handle must be i+8", rit->second == i+8);

		rit++;

		ensure("result must be (3,4)", rit->first == std::make_pair(3,4));
		ensure("handle must be i+10", rit->second == i+10);


		ei.x = 10; ei.y = 10; ei.score = 25; ei.handle = i;
		eyeMap[1] = ei;
		ei.x = 100; ei.y = 10; ei.score = 1; ei.handle = i+1;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 25; ei.handle = i+2;
		eyeMap[3] = ei;
		ei.x = 100; ei.y = 50; ei.score = 25; ei.handle = i+3;
		eyeMap[4] = ei;

		result = et.Track(4, eyeMap);

		rit = result.begin();

		ensure_equals("result must be empty", result.size(), 0);
	}

	template<>
	template<>
	void testobject::test<7>()
	{
		std::map<int, EyeInfo> eyeMap;
		int i[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

		EyeTracker et;

		EyeInfo ei;
		ei.x = 10; ei.y = 10; ei.score = 10; ei.handle = i;
		eyeMap[1] = ei;
		ei.x = 100; ei.y = 10; ei.score = 10; ei.handle = i+1;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 10; ei.handle = i+2;
		eyeMap[3] = ei;
		ei.x = 100; ei.y = 50; ei.score = 10; ei.handle = i+3;
		eyeMap[4] = ei;

		std::list<std::pair<void *, void *> > result;

		result = et.generate_pairings(1, eyeMap);

		ensure("result must be empty", result.empty());

		eyeMap.clear();

		ei.x = 120; ei.y = 10; ei.score = 12; ei.handle = i+4;
		eyeMap[1] = ei;
		ei.x = 15; ei.y = 14; ei.score = 8; ei.handle = i+5;
		eyeMap[2] = ei;
		ei.x = 5; ei.y = 60; ei.score = 4; ei.handle = i+6;
		eyeMap[3] = ei;

		result = et.generate_pairings(2, eyeMap);

		ensure_equals("result size must match", result.size(), 3);
		std::set<std::pair<void *, void *> > exp_result;

		exp_result.insert(std::make_pair(i,i+5)); 
		exp_result.insert(std::make_pair(i+1,i+4));
		exp_result.insert(std::make_pair(i+2,i+6));

		for(std::list<std::pair<void *, void *> >::iterator lit=result.begin(); lit!= result.end(); lit++)
			ensure("Must exist", exp_result.find(*lit) != exp_result.end());

		eyeMap.clear();

		ei.x = 10; ei.y = 10; ei.score = 10; ei.handle = i+7;
		eyeMap[1] = ei;
		ei.x = 100; ei.y = 10; ei.score = 2; ei.handle = i+8;
		eyeMap[2] = ei;
		ei.x = 10; ei.y = 50; ei.score = 2; ei.handle = i+9;
		eyeMap[3] = ei;
		ei.x = 100; ei.y = 50; ei.score = 2; ei.handle = i+10;
		eyeMap[4] = ei;

		exp_result.clear();
		exp_result.insert(std::make_pair(i+5,i+7));
		exp_result.insert(std::make_pair(i+4,i+8));
		exp_result.insert(std::make_pair(i+6,i+9));

		result = et.generate_pairings(3, eyeMap);

		ensure_equals("result must be empty", result.size(), 3);
		for(std::list<std::pair<void *, void *> >::iterator lit=result.begin(); lit!= result.end(); lit++)
			ensure("Must exist", exp_result.find(*lit) != exp_result.end());

	}

}
