//***********************************//
//Modulo temporizador para pileta    //
//                                   //
//Matias Pierdominici                //
//mpierdominici@itba.edu.ar          //
//***********************************//
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


//#define DEBUGG
#define SEC_TO_MILISEC(x) ((x)*1000) 
//*********Pines***********
#define PIN_YK04_D0 D6
#define PIN_YK04_D1 D7
#define PIN_YK04_D2 D1
#define PIN_YK04_D3 D5
#define PIN_YK04_VT D2

//*********end Pines*******


class myTimer
{
  public:
  myTimer(unsigned int seconds=0);
  bool timeOver(void);
  void setNewTime(unsigned long seconds_);
  void showInfo();
  
  unsigned long seconds;
  unsigned long startTime;
  void resetTimer(void);
    
};



class waterBomb
{
  public:
  waterBomb(unsigned int pin_);
  void onn(){digitalWrite(pin,HIGH);};
  void off(){digitalWrite(pin,LOW);};
  private:
  unsigned int pin;
  
};



char * ssid ="WIFI Pier2";
char * pass ="pagle736pagle";
unsigned int mqttPort=1883;

const char MqttUser[]="rcBox";
const char MqttPassword[]="1234";
const char MqttClientID[]="rc";

IPAddress mqttServer(192,168,0,116);

WiFiClient wclient;
PubSubClient mqtt_client(wclient);





void callback(char* topic, byte* payload, unsigned int length);
void  debug_message (char * string, bool newLine)
{
#ifdef DEBUGG
  if(string !=NULL)
  {
    if (!newLine)
    {
      Serial.print(string);
    }else
    {
      Serial.println(string);
    }
  }
  #endif
}

void setUpWifi(char * ssid, char * pass)
{
  String ip;
  debug_message(" ",true);
  debug_message(" ",true);
  debug_message("Conectandose a: ",false);
  debug_message(ssid,true);

  WiFi.begin(ssid,pass);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    debug_message(".",false);
  }
  debug_message(" ",true);
  debug_message("Coneccion realizada",true);
  debug_message("La ip es: ",false);
  ip=WiFi.localIP().toString();
  debug_message((char *)ip.c_str(),true);
}

void setUpMqtt(void)
{
  mqtt_client.setServer(mqttServer,mqttPort);
  mqtt_client.setCallback(callback);
}


void callback(char* topic, byte* payload, unsigned int length)
{
  int tiempo=0;
  payload[length]='\n';
  String message((char *)payload);
  debug_message("Llego un mensage, topic:",false);
  debug_message(topic,false);
  debug_message(", payload : ",false);
  debug_message((char *)payload,true);

  if(!strcmp(topic,"cisterna/watchdog"))
  {
  
    //debug_message("LLEGO MENSAJE PARA DETENER LA BOMBA",true);
   
  }else if(!strcmp(topic,"cisterna/bomba/on")){
   // eventList.push(ENCENDER_BOMBA);
    //time2checkBomba.resetTimer();
  }
 
}

void reconnect()
{
  while(!mqtt_client.connected())
  {
    debug_message("Intentando conectar al servidor MQTT",true);
    if (mqtt_client.connect(MqttClientID,MqttUser,MqttPassword))
      {
            debug_message("conectado",true);
  
  
            // ...suscrivirse a topicos
            //mqtt_client.subscribe("cisterna/watchdog");
      }
      else
      {
        debug_message("intentando conetarse al broker",true);
        delay(3000);
      }
  }
}
bool rfOutputStatus[4];
int inputs2read[]={PIN_YK04_D0,PIN_YK04_D1,PIN_YK04_D2,PIN_YK04_D3};
void setup() {
  Serial.begin(9600);
  setUpWifi(ssid,pass);
  setUpMqtt();
 pinMode(PIN_YK04_D0,INPUT);
 pinMode(PIN_YK04_D1,INPUT);
 pinMode(PIN_YK04_D2,INPUT);
 pinMode(PIN_YK04_D3,INPUT);
 pinMode(PIN_YK04_VT,INPUT);
 for(int i=0;i<4;i++){
      rfOutputStatus[i]=false;
    }

}


void loop() {
  if (!mqtt_client.connected()) 
  {
      reconnect();  
 }
 mqtt_client.loop();
 if(digitalRead(PIN_YK04_VT)){
  for(int i=0;i<4;i++){
  if(digitalRead(inputs2read[i])){
    rfOutputStatus[i]=true;
  }
 } 
 }else{
    for(int i=0;i<4;i++){
      if(rfOutputStatus[i]){
        mqtt_client.publish("rc/buttonPress",String(i).c_str());//devuelvo el estado actual
      }
      
      rfOutputStatus[i]=false;
    }
 }
 
 

}





//***********************TIMER**********************************



myTimer::myTimer(unsigned int seconds)
{
  setNewTime(seconds);
}

//timeOver
//devuelve true si ya paso el tiempo seteado,
//caso contrario devuelve false
//
bool myTimer::timeOver(void)
{
  if((millis())>startTime)
  {
    //resetTimer();//time over reseteal el timer se lo sque
    return true;
  }
  else
  {
    return false;
  }
}

void myTimer::resetTimer(void)
{
  unsigned long temp=seconds+millis();
 
  startTime=temp;
  //Serial.print("se llamo a rest timer con: ");
  //Serial.println(startTime);
}

void  myTimer::setNewTime(unsigned long seconds_)
{
  unsigned long temp=1000*seconds_;
  //Serial.println(temp);
  seconds=temp;
 
  //Serial.print("s seteo un timer cada: ");
  //Serial.print(seconds_);
  //Serial.print(" se registro un tirmpo de: ");
  //Serial.println(seconds/1000);
  resetTimer();

}

void myTimer::showInfo()
{
  //Serial.println(startTime);
  unsigned long dif=startTime-millis();
  //Serial.print("Remaining time (seconds):");
  //Serial.println(dif/1000);
  //Serial.println(startTime);
  //Serial.println(millis());
  //Serial.println(seconds/1000);
}

//*********************BOMBA DE AGUA********************

waterBomb::waterBomb(unsigned int pin_)
{
  pin=pin_;
  pinMode(pin,OUTPUT);
  off();
}
