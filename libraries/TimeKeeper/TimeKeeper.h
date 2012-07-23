/*
	TimeKeeper.h - Library for managing elapsed time
	Created by Kevin Cook, 7/20/2012
	Released into the public domain
*/

#ifndef TimeKeeper_h
#define TimeKeeper_h

class TimeKeeper
{
public:
	TimeKeeper(unsigned long interval, const char *name);
	void update_elapsed_time(bool log = false);
	unsigned long get_elapsed_time();
	void reset();
	bool elapsed();
	
private:	
	unsigned long current_time;
	unsigned long previous_time;
	unsigned long elapsed_time;
	unsigned long interval_time;
	
	const char *instance_name;
	
	static unsigned int instance_count;
};

#endif