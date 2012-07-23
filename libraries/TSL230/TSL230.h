#ifndef TSL230_h
#define TSL230_h

class TSL230
{
public:
	TSL230(int output, int s0, int s1, int s2, int s3);
	
	void setSens1x();
	void setSens10x();
	void setSens100x();
	
	void setScalingDiv100();
	void setScalingDiv10();
	void setScalingDiv2();
	void setScalingDiv1();
	
	int getFrequencyMultiplier();
	unsigned long getFrequency(unsigned long my_pulse_count);
	int getSensitivity();
	
	void addPulse();
	void resetPulseCount();
	int getPulseCount();
	
	float getUWattCm2(unsigned long frequency);
	
private:
	void setSens(int s0, int s1, int val);
	void setScale(int s2, int s3);
	
	int pin_output;
	int pin_s0;
	int pin_s1;
	int pin_s2;
	int pin_s3;
	
	int sensitivity;
	int scaling;
	
	int frequency_multiplier;
	volatile int pulse_count;
};

#endif