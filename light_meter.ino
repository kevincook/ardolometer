/*
    Example code for the Taos TSL230R and Arduino.
    
    C. A. Church - 11/2008
    
    This work is licensed under a Creative Commons
    Attribution-Share Alike 3.0 license.
*/
#include <LiquidCrystal.h>
#include <MsTimer2.h>
#include <TimeKeeper.h>
#include <TSL230.h>

// TSL230R Pin Definitions

#define TSL_FREQ_PIN  2  // output use digital pin2 for interrupt
#define TSL_S0        6
#define TSL_S1        5
#define TSL_S2        3
#define TSL_S3        4
#define CAMERA_PIN    13

// How often to calculate frequency
// 1000ms = 1s

#define READ_TM 1000

#define INTERVAL 5000
#define APERTURE 8
#define ISO 100

unsigned long last_freq = 0;

// How many steps per EV to deal with - 
// calculations round to the nearest step
// and adjustments happen in these steps,
// e.g.: 10 steps = .1 EV per step,
// 1 step = 1EV per step
byte ev_steps = 100;

// Maximum diff% in EV Value calculated from reading to reading
byte ev_diff_ceiling = 0;

// The previous EV calculated.  This helps to track the difference
// in EV between two readings
float prev_ev = 0.0;

// ev adjust can be negative
// 
// How many EV steps to add or subtract
// to each EV calculation - UI moves in 1/4EV
// Increments
float ev_adjust = 0.0;

float ttl_stop = 0.0;

TSL230 light_sensor(TSL_FREQ_PIN, TSL_S0, TSL_S1, TSL_S2, TSL_S3);
TimeKeeper exposure_interval(INTERVAL, "exposure");
TimeKeeper sample_interval(READ_TM, "sample" );

bool exposing = false;
int exp_ms = 0;

unsigned long pulse_count = 0;
unsigned long current_pulse_count = 0;
unsigned long current_time = millis();
unsigned long start_time = current_time;
unsigned int tm_diff = 0;

LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

void setup()
{
  lcd.begin(16,2);
  
  // attach interrupt to pin2,
  // sense output pin of TSL2304 to arduino 2
  // call handler on each rising pulse

  lcd.print("Initializing...");
  
  attachInterrupt(0, add_pulse, RISING);
    
  pinMode(CAMERA_PIN, OUTPUT);
    
  // debugging code
  Serial.begin(9600);
  lcd.clear();
  lcd.setCursor(7,0);
//  lcd.print("F");
//  lcd.print(APERTURE);
//  lcd.print(" I:");
//  lcd.print(ISO);
}

int sampleLight()
{
   // reset the ms counter
  sample_interval.reset();
  
  // get our current frequency reading
  lcd.setCursor(0,0);
  unsigned long frequency = get_tsl_freq();
  
  float uw_cm2 = 0.0;
  float lux = 0.0;
  
  if( frequency > 0 )
  {
    // calculate radiant energy  
    uw_cm2 = light_sensor.getUWattCm2(frequency);
//    lcd.print(uw_cm2);
    if( uw_cm2 < 0 )
    {
      uw_cm2 = 0;
      lux = 0;
    }
    else
    {      
      // calculate illuminance
      lux = calc_lux_gauss(uw_cm2);
    }
  }
  else
  {
    uw_cm2 = 0;
    lux = 0;
  }
//  lcd.print(lux);
  float ev = 0.0;
  if( lux <= 0 )
  {
    lux = 0;
    ev = -6.0;
  }
  else
  {
    ev = calc_ev(lux, ISO, ev_steps, ev_diff_ceiling);
  }
    
//  lcd.print(ev);
//  lcd.print(" - ");
  // calculate the exposure time
  float exp_tm = calc_exp_tm(ev, APERTURE);
//  lcd.print(exp_tm);
  Serial.print(exp_tm);
  Serial.print(" - ");
  int exp_ms = calc_exp_ms(exp_tm);
//  lcd.print(exp_ms);  
  Serial.print(" - ");
  Serial.println(exp_ms);
  return exp_ms;
}

void loop() {
  
  // check the value of the light sensor every READ_TM ms
  // calculate how much time has passed
  sample_interval.update_elapsed_time();  
  exposure_interval.update_elapsed_time();
      
  // if enough time has passed to
  // do a new reading...

  if( sample_interval.elapsed() )
  {
    exp_ms = sampleLight();
  }
  

//  if( exposing == false && exposure_interval.elapsed() )
//  {
//    capture(exp_ms);
//  }  
}

unsigned long get_tsl_freq()
{
  // we have to scale out the frequency --
  // scaling on the TSL230R requires us to multiply by a factor
  // to get actual frequency
  unsigned long freq = light_sensor.getFrequency(current_pulse_count);
  light_sensor.resetPulseCount();
    
  return(freq);
}

void add_pulse()
{
  pulse_count++;
  current_time = millis();
  if( current_time - start_time >= READ_TM)
  {
    current_pulse_count = pulse_count;
    pulse_count = 0;
    start_time = millis();
  }
  
  return;
}

float float_abs_diff(float f1, float f2)
{
  float diff = f1 > f2 ? f1 - f2 : f2 - f1;
  if ( diff <  0 )
  {
    diff *= -1;
  }
  
  return diff;
}

