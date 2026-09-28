// External LED test with serial print
const int ledPin = 5;  

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("External LED test start");
}

void loop() {
  digitalWrite(ledPin, HIGH);   // turn LED ON
  Serial.println("External LED ON");
  delay(1000);                  // keep 1s

  digitalWrite(ledPin, LOW);    // turn LED OFF
  Serial.println("External LED OFF");
  delay(1000);                  // keep 1s
}
