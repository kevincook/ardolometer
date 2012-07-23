void capture(int ms)
{  
  Serial.print("Exposing for ");
  Serial.print(ms);
  Serial.println(" ms");
  digitalWrite(CAMERA_PIN, HIGH);
  MsTimer2::set(ms, camera_off);
  MsTimer2::start();
  exposing = true;
}

void camera_off()
{
  Serial.println("camera_off..........");
  digitalWrite(CAMERA_PIN, LOW);
  MsTimer2::stop();
  exposing = false;
  exposure_interval.reset();
}

