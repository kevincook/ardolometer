// our wavelengths (nm) we're willing to calculate illuminance for (lambda)
int wavelengths[18] = { 380, 400, 420, 440, 460, 480, 500, 520, 540, 560, 580, 600, 620, 640, 660, 680, 700, 720 };

// the CIE V(l) for photopic vision - CIE Vm(l) 1978 - mapping to the same (l) above
float v_lambda[18]  = { 0.0002, 0.0028, 0.0175, 0.0379, 0.06, 0.13902, 0.323, 0.71, 0.954, 0.995, 0.87, 0.631, 0.381, 0.175, 0.061, 0.017, 0.004102, 0.001047 };

float spd_graphs[2][18] = {
  { 49.975500, 82.754900, 93.431800, 104.865000, 117.812000, 115.923000, 109.354000, 104.790000, 104.405000, 100.000000, 95.788000, 90.006200, 87.698700, 83.699200, 80.214600, 78.284200, 71.609100, 61.604000 },
  { 9.795100, 14.708000, 20.995000, 28.702700, 37.812100, 48.242300, 59.861100, 72.495900, 85.947000, 100.000000, 114.436000, 129.043000, 143.618000, 157.979000, 171.963000, 185.429000, 198.261000, 210.365000 }
};

bool light_type = 1;

float calc_lux_single(float uw_cm2, float efficiency)
{
  // calculate lux (1m/m&^2), using standard forula:
  // Xv = X1*V(1) * Km
  // X1 is W/m^2 (calculate actual received uW/cm^2, extrapolate from sensor size (0.0136cm^2)
  // to whole cm size, then convert uW to W
  // V(1) = efficiency function (provided via argument)
  // Km = constant, 1m/W @ 555nm = 683 (555nm has efficiency function of nearly 1.0)
  //
  // Only a singal wavelength is calculated - you'd better make sure that your
  // source is of a single wavelength... Otherwise, you should be using
  // calc_lux_gauss() for multiple wavelengths
  
  // convert to w_m2
  float w_m2 = (uw_cm2 / (float)1000000) * (float)100;
  
  // calculate lux
  float lux = w_m2 * efficiency * (float)683;
  
  return(lux);
}

float calc_ev(float lux, int iso, byte ev_steps, byte ev_diff_ceiling)
{  
  // calculate EV using APEX method:
  // Ev = Av + Tv = Bv + Sv
  
  // We'll use the right-hand side for this operation
  
  // Bv = log2( B/NK )
  // Sv = log2( NSx )
  
  float Sv = log( (float) 0.3 * (float) iso ) / log(2);
  
  float Bv = log( lux / ( (float) 0.3 * (float) 14 ) ) / log(2);
  
  float ev = Bv + Sv;
    
  if( ev_steps )
  {
    ev = (float) int(ev);
  }
  
  bool neg_ev = false;

  if (ev < 0)
  {
    neg_ev = true;
    ev *= -1;
  }
  
  if( ev > int(ev) )
  {
    int rem = (( ev - int(ev)) * 100);
    int step = 100 / ev_steps;
    
    for(int i = ev_steps; i > 0; i--)
    {
      if( rem >= step * i )
      {
        rem = step * i;
        break;
      }
    }
    
    ev = (float) int(ev) + ((float) rem / 100);
  }
  
  if(neg_ev)
  {
    ev *= -1.0;
  }
  
  if(ev_diff_ceiling > 0)
  {
    float diff = float_abs_diff(ev, prev_ev);
    float ceiling = (float)ev_diff_ceiling / (float)ev_steps;
    if( diff > ceiling )
    {
      if( ev < prev_ev )
      {
        ev = prev_ev - ceiling;
      }
      else
      {
        ev = prev_ev + ceiling;
      }
    }
  }
  
  prev_ev = ev;
  
  ev += ev_adjust;
  
  if( ttl_stop > 0 )
  {
    ev += log( ttl_stop ) / log(sqrt(2));
  }
    
  return ev;
}

float calc_lux_gauss( float uw_cm2 )
{
  // # of wavelengths mapped to V(1) values - better have enough V(1) values!
  
  int nm_cnt = sizeof(wavelengths) / sizeof(int);
  
  // W/m2 from uW/cm2
  
  float w_m2 =  ( uw_cm2 / (float) 1000000 ) * (float) 100;

  
  float result = 0;
  
  // integrate X1V(1) d1
  // X1 = uW-m2-nm calculation weighted by the CIE lookup for the given light temp
  // V(1) = standard luminous efficiency function
  
  for( int i = 0; i < nm_cnt; i++)
  {
    if( i > 0 )
    {
      result +=  ( spd_graphs[light_type][i] / (float) 1000000)  * (wavelengths[i] - wavelengths[i - 1]) * w_m2  * v_lambda[i];
    }
    else
    {
      result += ( spd_graphs[light_type][i] / (float) 1000000) * wavelengths[i] * w_m2 * v_lambda[i];
    }
  }
  
  float lux_gauss = result * (float) 683;
   
  return (lux_gauss);
}

float calc_exp_tm( float ev, float aperture )
{
  // Ev = Av + Tv = Bv + Sv
  // need to determine Tv value, so Ev - Av = Tv
  // Av = log2(Aperture^2)
  // Tv = log2(1/T) = log2(T) = 2^(Ev- Av)
  
//  float exp_tm = ev - ( log( pow(aperture, 2) ) / log(2) );
  lcd.print(ev);
  lcd.print(" - ");
  float av = log( pow(aperture, 2) ) / log(2);
  lcd.print(av);
  float exp_tm = ev - av;
  lcd.setCursor(0,1);
  lcd.print(exp_tm);
  lcd.print(" - ");
  float exp_log = pow(2, exp_tm);
  lcd.print(exp_log);
  
  return( exp_log);
}

unsigned int calc_exp_ms( float exp_tm )
{
  Serial.print(exp_tm);
  return ((unsigned long) (1000 / exp_tm));
//  unsigned int cur_exp_tm = 0;
//  
//  // calculate mS of exposure, given a divisor exposure time
//  
//  if( exp_tm >= 2 )
//  {
//    // deal with times less than or equal to half a second
//    
//    if( exp_tm >= (float) int(exp_tm) + (float) 0.5 )
//    {
//      // round up
//      exp_tm = int(exp_tm) + 1;
//    }
//    else
//    {
//      // round down
//      exp_tm = int(exp_tm);
//    }
//    
//    cur_exp_tm = 1000 / exp_tm;
//  }
//  else if ( exp_tm >=1 )
//  {
//    // deal with times larger than 1/2 second
//    
//    float disp_v = 1 / exp_tm;
//    
//    // get first significant digit
//    
//    disp_v = int(disp_v * 10);
//    cur_exp_tm = (1000 * disp_v) / 10;
//  }
//  else
//  {
//    // times larger than 1 second
//    int disp_v = int( (float) 1 / exp_tm );
//    cur_exp_tm = 1000 * disp_v;
//  }
//  
//  return(cur_exp_tm);
}
