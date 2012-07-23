#include "Arduino.h"
#include "TimeKeeper.h"

unsigned int TimeKeeper::instance_count = 0;

TimeKeeper::TimeKeeper(unsigned long interval, const char *name) :
interval_time(interval)
{
	Serial.println("Creating a TimeKeeper");
	current_time = previous_time = millis();
	elapsed_time = 0;
	instance_count++;
	instance_name = name;
}

void TimeKeeper::update_elapsed_time(bool log)
{
	previous_time = current_time;
	current_time = millis();
	
	if(log)
	{
		Serial.print(instance_name);
		Serial.print(": ");
		Serial.print(previous_time);
		Serial.print(" - ");
		Serial.print(current_time);		
	}
	
	if(current_time > previous_time)
	{
		if(log)
		{
			Serial.print("  >  ");
			Serial.print( elapsed_time );
			Serial.print( " -- ");			
		}
		elapsed_time += current_time - previous_time;
	}
	else
	{
		if(log)
		{
			Serial.print("  <=  ");
			Serial.print( elapsed_time );
			Serial.print( " -- ");			
		}
		elapsed_time += (current_time + (34359737 - previous_time));
	}
	
	if(log)
	{
		Serial.println(elapsed_time);		
	}
}

unsigned long TimeKeeper::get_elapsed_time()
{
	return elapsed_time;
}

void TimeKeeper::reset()
{
	elapsed_time = 0;
}

bool TimeKeeper::elapsed()
{
	return elapsed_time > interval_time;
}