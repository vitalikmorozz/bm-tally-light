#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <SkaarhojPgmspace.h>
#include <ATEMbase.h>
#include <ATEMstd.h>

#define RED_PIN D3
#define GREEN_PIN D2
#define BLUE_PIN D4

const int CAM_NUM = 2;

ATEMstd AtemSwitcher;
IPAddress ipInBMWiFi(192, 168, 0, 123 + CAM_NUM);
IPAddress switcherIp(192, 168, 0, 200);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

const char *ssid = "ssid";
const char *password = "pass";

int curPreviewCam = 0;
int curProgramCam = 0;
bool isConnectedToWiFI = false;
bool isConnectedToATEM = false;
int const CUR_BRIGHTNESS = 255 / 2;

void write(uint8_t pin, int val)
{
  analogWrite(pin, val);
}

void showBlack()
{
  write(BLUE_PIN, 0);
  write(GREEN_PIN, 0);
  write(RED_PIN, 0);
}

void showBlue()
{
  write(BLUE_PIN, CUR_BRIGHTNESS);
  write(GREEN_PIN, 0);
  write(RED_PIN, 0);
}

void showOrange()
{
  write(BLUE_PIN, 0);
  write(GREEN_PIN, CUR_BRIGHTNESS);
  write(RED_PIN, CUR_BRIGHTNESS);
}

void showGreen()
{
  write(BLUE_PIN, 0);
  write(GREEN_PIN, CUR_BRIGHTNESS);
  write(RED_PIN, 0);
}

void showRed()
{
  write(BLUE_PIN, 0);
  write(GREEN_PIN, 0);
  write(RED_PIN, CUR_BRIGHTNESS);
}

void connectToWiFiNetwork()
{
  if (!WiFi.config(ipInBMWiFi, gateway, subnet))
  {
    Serial.println("WiFi configuration failed!");
  }

  WiFi.begin(ssid, password);
  Serial.printf("Connecting to WiFi network %s.", ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    write(BLUE_PIN, CUR_BRIGHTNESS);
    delay(300);

    Serial.print(".");
    write(BLUE_PIN, 0);

    delay(300);
  }

  Serial.printf("Connected to WiFi network %s current IP address: %s\n", ssid, WiFi.localIP().toString().c_str());
}

boolean checkWiFiConnection()
{
  if (WiFi.status() == WL_CONNECTED && !isConnectedToWiFI)
  {
    isConnectedToWiFI = true;
    return true;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    isConnectedToWiFI = false;

    showBlack();
    delay(350);

    showBlue();
    delay(350);

    return false;
  }
  return true;
}

void connectToATEM()
{
  AtemSwitcher.begin(switcherIp);
  AtemSwitcher.connect();
  Serial.print("Connecting to ATEM switcher.");
  while (!AtemSwitcher.isConnected())
  {
    showBlack();
    delay(350);

    Serial.print(".");
    AtemSwitcher.runLoop();

    showOrange();
    delay(350);
  }

  Serial.printf("\nConnected to ATEM Switcher on IP address: %s\n", switcherIp.toString().c_str());
}

boolean checkATEMConnection()
{
  if (AtemSwitcher.isConnected() && !isConnectedToATEM)
  {
    isConnectedToATEM = true;
    return true;
  }

  if (!AtemSwitcher.isConnected())
  {
    isConnectedToATEM = false;

    showOrange();
    delay(350);

    AtemSwitcher.runLoop();

    showBlack();
    delay(350);

    return false;
  }
  return true;
}

void showCurrentStatus()
{
  if (curProgramCam == CAM_NUM)
  {
    return showRed();
  }
  if (curPreviewCam == CAM_NUM)
  {
    return showGreen();
  }
  return showBlack();
}

void setup()
{
  Serial.begin(921600);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);

  connectToWiFiNetwork();
  connectToATEM();
}

void loop()
{
  if (!checkWiFiConnection())
    return;
  if (!checkATEMConnection())
    return;

  AtemSwitcher.runLoop();
  curPreviewCam = AtemSwitcher.getPreviewInput();
  curProgramCam = AtemSwitcher.getProgramInput();
  showCurrentStatus();
}