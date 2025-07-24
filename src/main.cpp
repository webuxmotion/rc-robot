#include <lilka.h>

void setup() {
  lilka::begin();
  Serial.begin(9600);
}

void loop() {
  lilka::State state = lilka::controller.getState();

  if (state.up.justPressed)     Serial.println("UP pressed");
  if (state.up.justReleased)    Serial.println("UP released");

  if (state.down.justPressed)   Serial.println("DOWN pressed");
  if (state.down.justReleased)  Serial.println("DOWN released");

  if (state.left.justPressed)   Serial.println("LEFT pressed");
  if (state.left.justReleased)  Serial.println("LEFT released");

  if (state.right.justPressed)  Serial.println("RIGHT pressed");
  if (state.right.justReleased) Serial.println("RIGHT released");

  if (state.a.justPressed)      Serial.println("A pressed");
  if (state.a.justReleased)     Serial.println("A released");

  if (state.b.justPressed)      Serial.println("B pressed");
  if (state.b.justReleased)     Serial.println("B released");

  if (state.c.justPressed)      Serial.println("C pressed");
  if (state.c.justReleased)     Serial.println("C released");

  if (state.d.justPressed)      Serial.println("D pressed");
  if (state.d.justReleased)     Serial.println("D released");

  if (state.select.justPressed) Serial.println("SELECT pressed");
  if (state.select.justReleased)Serial.println("SELECT released");

  if (state.start.justPressed)  Serial.println("START pressed");
  if (state.start.justReleased) Serial.println("START released");

  delay(10);
}
