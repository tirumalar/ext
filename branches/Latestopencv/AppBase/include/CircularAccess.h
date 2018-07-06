/*
 * CircularAccess.h
 *
 *  Created on: 6 Jan, 2009
 *      Author: akhil
 */

#ifndef CIRCULARACCESS_H_
#define CIRCULARACCESS_H_
#include "SafeCounter.h"
#include "assert.h"


template<class C, class T>
class citerator{
private:
	C& myContainer;
	int myPos;
public:
	citerator(C& container ):myContainer(container),myPos(0){}
	void next(){
		myPos++;
		myPos=myPos%(myContainer.getSize());
	}
	T curr() const {
		return myContainer[myPos];
	}
	int getsize() const
	{
		return myContainer.getSize();
	}
};

/*
 * A generic class for accessing arrays in a circular fashion
 */
template <class T>
class CircularAccess {
protected:
	T *myContent;
	int myExtent;
	int myPos;
	SafeCounter myReadWriteCounter;
	inline CircularAccess& incr(){
		int pos=myPos+1;
		pos%=myExtent;
		myPos=pos;
		return *this;
	}
	inline CircularAccess& decr(){
		int pos=myPos-1+myExtent;
		pos%=myExtent;
		myPos=pos;
		return *this;
	}
public:
	CircularAccess():myContent(0), myExtent(0), myPos(0) {}
	//re-initializes
	inline CircularAccess& operator()(int extent){
		if(myContent){delete [] myContent;}
		myExtent=extent;
		myContent=new T[myExtent];
		myPos=0;
		myReadWriteCounter.init(myExtent);
		return *this;
	}
	virtual ~CircularAccess() {
		delete [] myContent;
	}
	inline int getSize() const { return myExtent;}
	inline T& getCurr() const{
			return myContent[myPos];
		}
	inline int curPos(){ return myPos;}
	inline CircularAccess& operator++(){return incr();}
	inline CircularAccess& operator++(int){return incr();}
	inline CircularAccess& operator--(){return decr();}
	inline CircularAccess& operator--(int){return decr();}

	//unsafe access.. can replace the content
	inline T& operator[](int i) const{
		int pos=i%myExtent;
		assert(pos>=0);
		return myContent[pos];
	}
	inline T getPrev() const{
		int pos=myPos-1+myExtent;
		pos%=myExtent;
		return myContent[pos];
	}
	inline T getPrev(int age) const{
		    age%=myExtent;
			int pos=myPos-age+myExtent;
			pos%=myExtent;
			return myContent[pos];
	}
	inline T getPrevOf(int index, int age) const{
		index%=myExtent;
		age%=myExtent;
		int pos=index-age+myExtent;
		pos%=myExtent;
		return myContent[pos];
	}
	inline T getNext() const{
			int pos=myPos+1;
			pos%=myExtent;
			return myContent[pos];
	}

	// these functions are for queue management
	void incrCounter(){
		return myReadWriteCounter.Incr();
	}
	void decrCounter(){
		return myReadWriteCounter.Decr();
	}
	bool isEmpty(){
		return myReadWriteCounter.isMin();
	}
	bool isFull(){
		return myReadWriteCounter.isMax();
	}
};

#endif /* CIRCULARACCESS_H_ */
