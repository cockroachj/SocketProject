#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <EasyDDNS.h>
#include <HTTPClient.h>
Servo myServo;       //伺服馬達
int servoAngle = 0;  //伺服馬達起始角度
int lcdColumns = 16; //LCD行數
int lcdRows = 2;     //LCD列數
int count = 0;       //動作次數
int socketCount = 0; //SOCKET連線次數
LiquidCrystal_I2C lcd(0X3F, lcdColumns, lcdRows);
void getMac()
{ //顯示ESP32 MAC
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC : ");
    Serial.print(mac[0], HEX);
    Serial.print(":");
    Serial.print(mac[1], HEX);
    Serial.print(":");
    Serial.print(mac[2], HEX);
    Serial.print(":");
    Serial.print(mac[3], HEX);
    Serial.print(":");
    Serial.print(mac[4], HEX);
    Serial.print(":");
    Serial.println(mac[5], HEX);
}
void Text(String message1, int row)
{
    lcd.clear();
    lcd.setCursor(0, row);
    for (int i = 0; i < message1.length(); i++)
    {
        lcd.print(message1.charAt(i));
        delay(100);
    }
}
char ssid[] = "netis_2.4G";
char password[] = "19871019";
WiFiServer wifiServer(14431);

WiFiUDP udp;
NTPClient timeClient(udp);
String formattedDate;
String dayStamp;
String timeStamp;
char command;
String readString, new_ip;

char *string2char(String command)
{
    if (command.length() != 0)
    {
        char *p = const_cast<char *>(command.c_str());
        return p;
    }
}

void LocalIP()
{
    HTTPClient http;
    http.begin("http://ipv4bot.whatismyipaddress.com/");
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            new_ip = http.getString();
            Serial.println(new_ip);
        }
    }
    else
    {
        http.end();
        return;
    }
    http.end();
}

void LcdTime()
{
    while (!timeClient.update())
    {
        timeClient.forceUpdate();
    }
    formattedDate = timeClient.getFormattedDate();
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
    Serial.print("DATE:");
    Serial.println(dayStamp);
    lcd.setCursor(0, 0);
    lcd.print("DATE:");
    lcd.print(dayStamp);

    timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
    Serial.print("Time: ");
    Serial.println(timeStamp);
    lcd.setCursor(0, 1);
    lcd.print("TIME: ");
    lcd.print(timeStamp + count);
}

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void setup()
{
    Serial.begin(115200);
    lcd.init();
    lcd.backlight();
    myServo.attach(18);
    myServo.write(0);

    getMac();

    Serial.print("Connecting to ");
    Text("Connecting to ", 0);
    Serial.println(ssid);
    Text(ssid, 0);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        Text(".", 1);
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    Text("WiFi connected.", 0);
    Serial.println("IP address: ");
    Text("IP address: ", 1);
    IPAddress ip = WiFi.localIP();
    Serial.println(ip);
    Text(String(ip[0]) + "." + ip[1] + "." + ip[2] + "." + ip[3], 1);
    LocalIP();
    timeClient.begin();
    timeClient.setTimeOffset(28800);
    wifiServer.begin();
    EasyDDNS.service("noip");
    EasyDDNS.client("cackroachj.ddns.net", "cackroachj", "19871019");
    EasyDDNS.onUpdate([&](const char *oldIP, const char *newIP) {
        Serial.print("EasyDDNS - IP Change Detected : ");
        Serial.println(newIP);
    });
    LcdTime();
}
void loop()
{
    WiFiClient client = wifiServer.available();
    client.setTimeout(3);
    String bufString;
    EasyDDNS.update(10000);
    if (client)
    {
        socketCount++;
        while (client.connected())
        {
            Serial.println("client.connected()");
            while (client.available() > 0)
            {
                char input = client.read();
                bufString += input;
            }

            Serial.println(bufString);
            // if (bufString.equals("Open"))
            // {
            //     myServo.write(90);
            //     Serial.println("in O");
            //     Serial.println(bufString);
            //     LcdTime();
            //     Text(bufString, 0);
            // }
            // else if (bufString.equals("Close"))
            // {
            //     myServo.write(0);
            //     Serial.println("in C");
            //     Serial.println(bufString);
            //     LcdTime();
            //     Text(bufString, 0);
            //     // delay(3000);
            // }
            // else
            // {
            //     Serial.println("in default");
            //     Serial.println(bufString);
            //     LcdTime();
            // }
            switch (bufString.charAt(0))
            {
            case 'O':
                Serial.println("in O");
                myServo.write(90);
                LcdTime();
                count++;
                client.write("Open");
                client.flush();
                // delay(3000);
                break;
            case 'C':
                Serial.println("in C");
                myServo.write(0);
                LcdTime();
                count++;
                client.write("Close");
                client.flush();
                // delay(3000);
                break;
            default:
                Serial.println("in default");
                client.write("Socket Error...");
                client.flush();
                LcdTime();
                break;
            }
            break;
        }
        client.stop();
        Serial.println("Client disconnected ! ");
        Serial.println(socketCount);
        Serial.println(count);
    }
    else
    {
        delay(1);
    }
}