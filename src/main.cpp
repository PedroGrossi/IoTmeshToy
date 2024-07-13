#include "painlessMesh.h"
#include "ArduinoJson.h"
#include <esp32-hal-timer.h> // timer lib

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

#define   MAX_BLIPS       4

painlessMesh  mesh;
hw_timer_t *timer=NULL;
volatile bool timerFlag=false;

// Prototypes -> functions before loop
String buildControl(void);
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback(void);
void nodeTimeAdjustedCallback(int32_t offset);
void IRAM_ATTR onTimer(void);

void setup()
{
  Serial.begin(115200);

  Serial.printf("teste\n");

  // setup mesh
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes(ERROR | STARTUP);  // set before init() so that you can see startup messages
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  //mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  Serial.printf("\nIn setup() my NodeId=%d\n", mesh.getNodeId());

  // Timer setup
  timer=timerBegin(0, 80, true); // 1us
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true); // 1s to alarm
  timerAlarmEnable(timer);
}

void loop()
{
  mesh.update();

  static uint16_t previousConnections=0;
  uint16_t numConnections=mesh.getNodeList().size();

  if(previousConnections!=numConnections)
  {
    Serial.printf("loop(): numConnections=%d\n", numConnections);

    if(numConnections==0)
    {
      Serial.printf("No conection\n");
      /* //coisas da animação dos leds
      controllers[smoothIdx].nextAnimation = searchingIdx;
      controllers[searchingIdx].nextAnimation = searchingIdx;
      controllers[searchingIdx].hue[0] = 0.0f;
      */
    }
    else
    {
      Serial.printf("\nHave conection\n");
      /* //coisas da animação dos leds
      controllers[searchingIdx].nextAnimation = smoothIdx;
      controllers[smoothIdx].nextAnimation = smoothIdx;
      */
    }

    previousConnections=numConnections;
  }
}

String buildControl(void)
{
  uint16_t blips=mesh.getNodeList().size()+1;
  Serial.printf("buildControl(): blips=%d\n", blips);

  if (blips>MAX_BLIPS)
  {
    Serial.printf(" blips out of range =%d\n", blips);
    blips=MAX_BLIPS;
  }

  // Create a json document
  StaticJsonDocument<200> jsonDocument;
  // Create a object inside the document
  JsonObject control=jsonDocument.to<JsonObject>();
  for(int i=0;i<blips;i++)
  {
    //coisas da animação dos leds
    //control[String(i)]=String(controllers[smoothIdx].hue[i]);
  }

  String ret;
  serializeJson(control, ret); // Serialize the json to string
  
  return ret;
}

// Needed for painless library
void receivedCallback(uint32_t from, String &msg)
{
  Serial.printf("newConnectionCallback()\n");

  DynamicJsonDocument jsonDocument(50);
  DeserializationError error=deserializeJson(jsonDocument, msg);

  // Verify error in deserializeJson
  if(error)
  {
    Serial.printf("deserializeJson() error: %s\n",error.c_str());
    return;
  }

  Serial.printf("control=%s\n", msg.c_str());

  for(int i=0;i<(mesh.getNodeList().size()+1);i++)
  {
    /*//coisas da animação dos leds
    float hue = jsonDocument[String(i)];
    controllers[smoothIdx].hue[i] = hue;
    */
  }
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("newConnectionCallback()\n");
  String control=buildControl();
  mesh.sendBroadcast(control);
}

void changedConnectionCallback(void)
{
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset)
{
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

// Needed to timer
void IRAM_ATTR onTimer(void)
{
  timerFlag=true;
}
