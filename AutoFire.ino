#define MY_DEBUG

#define RELAY_PIN 2
#define FIRE_SENSOR_AI A0
#define FIRE_SENSOR_DI 10

#define SPARKING_TIME_MS 2000
#define DOUBLE_CHECK_INTERVAL_MS 50
#define FIRE_SENSOR_VAL_THRESHOLD 900
#define FIRE_SENSOR_DEVIATION 25
#define MAX_RETRIES 10

enum Mode
{
  Idle = 0,
  Monitoring = 1,
  Failure = 2
};

Mode mode = Mode::Idle;

int failures_count = 0;
int fire_started_count = 0;

bool has_fire()
{
  auto const fire_sensor_analog_val = analogRead(FIRE_SENSOR_AI);
  return fire_sensor_analog_val <= (FIRE_SENSOR_VAL_THRESHOLD - FIRE_SENSOR_DEVIATION);
}

bool has_no_fire()
{
  auto const fire_sensor_analog_val = analogRead(FIRE_SENSOR_AI);
  return fire_sensor_analog_val > (FIRE_SENSOR_VAL_THRESHOLD + FIRE_SENSOR_DEVIATION);
}

bool is_fire_started()
{
  int cnt = 0;
  for(int i = 0; i < 100; ++i)
  {
    if(has_fire())
    {
      ++cnt;
    }
    delay(1);
  }
  return cnt >= 75;
}

bool is_fire_extinguished()
{
  int cnt = 0;
  for(int i = 0; i < 100; ++i)
  {
    if(has_no_fire())
    {
      ++cnt;
    }
    delay(1);
  }
  return cnt >= 75;
}

// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

#ifdef MY_DEBUG
  Serial.begin(115200);
  Serial.println("Started!");
#endif
}

// the loop function runs over and over again forever
void loop() {
#ifdef MY_DEBUG
  auto fire_sensor__analog_val = analogRead(FIRE_SENSOR_AI);
  Serial.println("-----------------------------");
  Serial.print("Analog value: ");
  Serial.println(fire_sensor__analog_val);
  Serial.print("Mode: ");
  Serial.println(mode);
  Serial.print("Failures: ");
  Serial.println(failures_count);
  Serial.print("Started: ");
  Serial.println(fire_started_count);
#endif

  switch(mode)
  {
    case Mode::Idle:
    {
      if(has_fire())
      {
        if(is_fire_started())
        {
          mode = Mode::Monitoring;
        }
      }
      break;
    }
    case Mode::Monitoring:
    {
      if(has_no_fire())
      {
        if(is_fire_extinguished())
        {
          digitalWrite(RELAY_PIN, HIGH);
          delay(SPARKING_TIME_MS);
          digitalWrite(RELAY_PIN, LOW);
          
          if(is_fire_extinguished())
          {
            ++failures_count;
            mode = failures_count >= MAX_RETRIES ? Mode::Failure : Mode::Idle;
          }
          else
          {
            ++fire_started_count;
          }
        }
      }
      break;
    }
    case Mode::Failure:
    {
      // Do nothing
      break;
    }
  }

  delay(100);
}
