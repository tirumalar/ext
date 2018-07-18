/*
 * ShiftAndDcOffset.h
 *
 *  Created on: Feb 18, 2013
 *      Author: mamigo
 */

#ifndef SHIFTANDDCOFFSET_H_
#define SHIFTANDDCOFFSET_H_
class GPIODriver;
class ShiftAndDcOffset {
public:
	ShiftAndDcOffset();
	virtual ~ShiftAndDcOffset();
	void SetShiftAndOffsetValue(int shift,int dcoffset);
private:
	void SetToGPIOShiftAndOffset();
	int m_shiftValue;
	int m_dcOffset;
	GPIODriver *m_gpioShift[4];
	GPIODriver *m_gpioOffset[4];
	int m_sub;
};

#endif /* SHIFTANDDCOFFSET_H_ */
