/*
 * ELEyeMsg.cpp
 *
 *  Created on: 01-Oct-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#include "ELEyeMsg.h"

ELEyeMsg::ELEyeMsg():HTTPPOSTMsg(0) {
	Buffer=0;
	Size=0;
	Available=0;


}

ELEyeMsg::~ELEyeMsg() {
	Buffer=0;
	Size=0;
	Available=0;
}

void ELEyeMsg::CopyFrom(const Copyable& _other){
		BinMessage& other=( BinMessage&)_other;
		Timestamp=other.GetTime();
		Buffer=other.GetBuffer();
		Size=other.GetSize();
		Available=other.GetAvailable();
};

