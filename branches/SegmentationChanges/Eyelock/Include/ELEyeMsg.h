/*
 * ELEyeMsg.h
 *
 *  Created on: 01-Oct-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#ifndef ELEYEMSG_H_
#define ELEYEMSG_H_

#include <HTTPPOSTMsg.h>
//I  use shallow approach: buffer is not allocated by me
class ELEyeMsg: public HTTPPOSTMsg {
public:
	ELEyeMsg();
	virtual ~ELEyeMsg();
	virtual void CopyFrom(const Copyable& _other);
};

#endif /* ELEYEMSG_H_ */
