#define BLYNK_TEMPLATE_ID "TMPL3suOSEyY_"
#define BLYNK_TEMPLATE_NAME "Smart Timer Plug"
#define BLYNK_AUTH_TOKEN "LQgI7luO0UZHQUox-9zIPjgxrt6hDjdL"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <time.h>

char ssid[] = "wifi009";
char pass[] = "12345678";

#define RELAY_PIN 23

BlynkTimer timer;

int countdownSeconds = 5;
int scheduleHour = 0;
int scheduleMinute = 0;
int timeUnit = 0;

bool scheduleExecuted = false;

// Relay OFF
void turnOffRelay()
{
  digitalWrite(RELAY_PIN, HIGH);
  Blynk.virtualWrite(V0, 0);

  Serial.println("Relay OFF after countdown");
}

// Countdown (V1)
BLYNK_WRITE(V1)
{
  countdownSeconds = param.asInt();

  Serial.print("Countdown: ");
  Serial.print(countdownSeconds);
  Serial.println(" sec");
}

// Hour (V2)
BLYNK_WRITE(V2)
{
  scheduleHour = param.asInt();

  Serial.print("Hour Set: ");
  Serial.println(scheduleHour);
}

// Minute (V3)
BLYNK_WRITE(V3)
{
  scheduleMinute = param.asInt();

  Serial.print("Minute Set: ");
  Serial.println(scheduleMinute);
}

BLYNK_WRITE(V4)
{
  timeUnit = param.asInt();

  if(timeUnit == 0)
    Serial.println("Unit: Seconds");

  else if(timeUnit == 1)
    Serial.println("Unit: Minutes");

  else if(timeUnit == 2)
    Serial.println("Unit: Hours");
}

// Relay Switch (V0)
BLYNK_WRITE(V0)
{
  int state = param.asInt();

  if (state == 1)
  {
    digitalWrite(RELAY_PIN, LOW);

    Serial.print("Relay ON for ");
    Serial.print(countdownSeconds);
    Serial.println(" seconds");

    timer.setTimeout(getDelayTime(), turnOffRelay);
  }
  else
  {
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("Relay OFF");
  }
}

void checkSchedule()
{
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo))
  {
    return;
  }

  int currentHour = timeinfo.tm_hour;
  int currentMinute = timeinfo.tm_min;

  static int lastMinute = -1;

  if (currentMinute != lastMinute)
  {
    Serial.printf("Current Time: %02d:%02d\n",
                  currentHour,
                  currentMinute);

    lastMinute = currentMinute;
  }

  if (currentHour == scheduleHour &&
      currentMinute == scheduleMinute &&
      !scheduleExecuted)
  {
    Serial.println("================================");
    Serial.println("SCHEDULED TIME REACHED!");
    Serial.println("Relay Turned ON Automatically");
    Serial.println("================================");

    digitalWrite(RELAY_PIN, LOW);

    timer.setTimeout(getDelayTime(), turnOffRelay);

    scheduleExecuted = true;
  }

  if (currentMinute != scheduleMinute)
  {
    scheduleExecuted = false;
  }
}
unsigned long getDelayTime()
{
  if(timeUnit == 0)
    return countdownSeconds * 1000UL;

  else if(timeUnit == 1)
    return countdownSeconds * 60000UL;

  else
    return countdownSeconds * 3600000UL;
}
void setup()
{
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // India Time (IST)
  configTime(19800, 0, "pool.ntp.org", "time.nist.gov");

  delay(2000);

  // Restore values from Blynk
  Blynk.syncVirtual(V1);
Blynk.syncVirtual(V2);
Blynk.syncVirtual(V3);
Blynk.syncVirtual(V4);

  Serial.println("Blynk Connected");
}

void loop()
{
  Blynk.run();
  timer.run();

  checkSchedule();

  delay(100);
}