#ifndef LOCAL_DATATYPES_H_
#define LOCAL_DATATYPES_H_

// Added by AC to store measured values
struct bldcMeasure {
	//7 Values int16_t not read(14 byte)
	float avgMotorCurrent;
	float avgInputCurrent;
	float dutyCycleNow;
	long rpm;
	float inpVoltage;
	float ampHours;
	float ampHoursCharged;
	//2 values int32_t not read (8 byte)
	long tachometer;
	long tachometerAbs;
	int errorCode;
};

#define ERRCODE_NO_ERROR	0
#define ERRCODE_NO_COMMS	1

//Define remote Package

struct remotePackage {

	int		valXJoy;
	int		valYJoy;
	boolean	valUpperButton;
	boolean	valLowerButton;
};

#endif