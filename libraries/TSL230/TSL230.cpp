#include "Arduino.h"
#include "TSL230.h"

TSL230::TSL230(int output, int s0, int s1, int s2, int s3) :
pin_output(output),
pin_s0(s0),
pin_s1(s1),
pin_s2(s2),
pin_s3(s3),
pulse_count(0)
{	
	pinMode(pin_output, INPUT);
	pinMode(pin_s0, OUTPUT);
	pinMode(pin_s1, OUTPUT);
	pinMode(pin_s2, OUTPUT);
	pinMode(pin_s3, OUTPUT);
	
	setSens1x();
	setScalingDiv100();
}

void TSL230::setSens1x()
{
	setSens(HIGH, LOW, 1);
}

void TSL230::setSens10x()
{
	setSens(LOW, HIGH, 10);
}

void TSL230::setSens100x()
{
	setSens(HIGH, HIGH, 100);
}

void TSL230::setScalingDiv100()
{
	setScale(HIGH, HIGH);
	frequency_multiplier = 100;
}

void TSL230::setScalingDiv10()
{
	setScale(LOW, HIGH);
	frequency_multiplier = 10;
}

void TSL230::setScalingDiv2()
{
	setScale(HIGH, LOW);
	frequency_multiplier = 2;
}

void TSL230::setScalingDiv1()
{
	setScale(LOW, LOW);
	frequency_multiplier = 1;
}

void TSL230::setSens(int s0, int s1, int sens)
{
	digitalWrite(pin_s0, s0);
	digitalWrite(pin_s1, s1);
	sensitivity = sens;
}

void TSL230::setScale(int s2, int s3)
{
	digitalWrite(pin_s2, s2);
	digitalWrite(pin_s3, s3);
}

int TSL230::getFrequencyMultiplier()
{
	return frequency_multiplier;
}

unsigned long TSL230::getFrequency(unsigned long my_pulse_count)
{
	return my_pulse_count * frequency_multiplier;
}

void TSL230::addPulse()
{
	pulse_count++;
}

void TSL230::resetPulseCount()
{
	pulse_count = 0;
}

int TSL230::getPulseCount()
{
	return pulse_count;
}

int TSL230::getSensitivity()
{
	return sensitivity;
}

float TSL230::getUWattCm2(unsigned long frequency)
{
	float uw_cm2 = (float) frequency / (float) getSensitivity();
	uw_cm2 *= ( (float)1 / (float)0.0136 );
	return(uw_cm2);	
}