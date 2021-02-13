#include <Arduino.h>

const int stanokPinInput = 8; // Прописываем пин входа станка.

class Timer{ // Этот класс описывает тикающее время
  int8_t sec=0;
  int8_t min=0;
  int8_t hour=0;
  int onlySec=0; // Переменная для только секунд которые после 59 не обнуляются

  unsigned long timing=0; // Надо для отсчёта прохождения одной секунды
  bool impulsPrint=false; // Для печати после изменения времени а не постоянно

  // Отсчёт с текущего момента
  bool setTheTimeOnce = false;
  int lowerBound = 0; // С какого времени считать 
  int upperBound = 0; // По какое время считать отсчёт 5мин. В секундах.
  // Отсчёт с текущего момента
public:
  void TicTac(){ // Метод. Занимается отсчётом секунд
    if (millis() - timing > 1000){
      sec++;
      onlySec++; // 5min = 300sec
      timing = millis(); impulsPrint=true;
 	}
    if(sec>59){
      sec = 0;
      min++;
    }
    if(min>59){
      min = 0;
      hour++;
    }
    
  }
  void StartTickFiveMinFromThisMoment(){ // Отсчитать время с текущего момента
    if(setTheTimeOnce == false){
          lowerBound=onlySec;
          upperBound=lowerBound+300;
       setTheTimeOnce = true;
    }
  }
  bool TickTackFiveMin(){
    if(onlySec > upperBound){
      return true;
    }
    else{
      return false;
    }
  }
  void ReloadStartTickFiveMinFromThisMoment(){
    Serial.println("I make reload");
    setTheTimeOnce = false;
  }
  void PrintTime(){
    if(impulsPrint == true){
      	Serial.print("Timer: ");
 
    	  Serial.print(hour);Serial.print(":");
    	  Serial.print(min); Serial.print(":");
    	  Serial.print(sec); Serial.print(" ");
        Serial.print("onlySec:"); Serial.print(onlySec); Serial.print(" ");
        
        Serial.print("lowerBound:"); Serial.print(lowerBound); Serial.print(" ");
        Serial.print("upperBound:"); Serial.print(upperBound); Serial.print(" ");

        Serial.print("TickTackFiveMin():"); Serial.print(TickTackFiveMin()); Serial.print(" ");
        Serial.print("digitalRead(stanokPinInput):"); Serial.print(digitalRead(stanokPinInput)); Serial.print(" ");

        Serial.println("");
      	impulsPrint=false;
    }
  }
};
Timer timer;

class Buzzer{
private:  
  const int buzzerPin = 13;
  unsigned long timingBeepBeep=0;
  int stepPlay =0;
public:
  Buzzer(){
    pinMode(buzzerPin, OUTPUT);
  }
  void PlayMelody(){
      if ( (millis() - timingBeepBeep > 500) and stepPlay == 0){ 
          digitalWrite(buzzerPin,HIGH);   
          stepPlay =1;
          timingBeepBeep = millis(); 
      }
      if ( (millis() - timingBeepBeep > 500) and stepPlay == 1){ 
          digitalWrite(buzzerPin,LOW);   
          stepPlay =0;
          timingBeepBeep = millis(); 
      }
  }
  void StopMelody(){
    stepPlay =0;
    digitalWrite(buzzerPin,LOW);   
  }
};
Buzzer buzzer;

class Button{
private:
    const int sensorButton = 7;
    bool buttonState = false;
    bool oldButtonState = false;

 
public:

    Button(){ // Конструктор. В конструкторе мы описали пин кнопки как Вход
      pinMode(sensorButton,INPUT);
    }
    void ButtonLogic(){
        if(digitalRead(sensorButton) == true){
          buttonState = true;
        }
    }
    bool GetButtonState(){
        if(buttonState == true){
          return true;
        }
        else{
          return false;
        }
    }
    void ResetButtonState(){
      buttonState = false;
    }
    void printButton(){
      if(oldButtonState != buttonState){
        Serial.print("buttonState:");Serial.println(buttonState);
        oldButtonState = buttonState;
      }
    }

};
 Button button;



void setup()
{
  Serial.begin(9600);
  pinMode(stanokPinInput,INPUT);
}

bool canStopPlay = false;
bool katana = false; // Когда контроллер, совершая проверку каждые 5 минут, обнаружил, что станок в работе, последующая проверка должна быть с минимальной задержкой. 
void loop()
{
  timer.TicTac();
  timer.PrintTime();

  button.ButtonLogic();
  button.printButton();

// Cам наш алгоритм работы
  if(digitalRead(stanokPinInput)==HIGH and canStopPlay==false and katana == false){ //Если станок остановился
      buzzer.PlayMelody(); // Играет мелодия 
  }
  if(digitalRead(stanokPinInput)==HIGH and button.GetButtonState() ==true and katana == false){  //Ecли нажата кнопка и станок остановлен
          canStopPlay=true;
          buzzer.StopMelody(); // Мелодия перестаёт играть

          timer.ReloadStartTickFiveMinFromThisMoment(); // Запуск на проверку ещё через 5мин после текущего момента
          timer.StartTickFiveMinFromThisMoment(); // То начать отсчёт 5 мин с текущего момента
      
      button.ResetButtonState();              // Програмно отпустить кнопку
      //Начать отсчёт с этого момента 300сек
  }
  // Если прошло 5 мин с нажатия на кнопку когда станок остановлен, и станок по прежнему остановлен то запищать
  if(digitalRead(stanokPinInput)==HIGH and timer.TickTackFiveMin() == true and katana == false){
      buzzer.PlayMelody();
  }
  // Если прошло 5 мин с нажатия на кнопку когда станок остановлен, и станок по прежнему остановлен то запищать


  // Если прошло 5 мин с нажатия на кнопку когда станок остановлен, а станок ЗАПУЩЕН и с ним ничего ненадо делать
    // То проверить ещё через 5 мин
    if(timer.TickTackFiveMin() == true and digitalRead(stanokPinInput)==LOW){
      katana=true;
    }
  // Если прошло 5 мин с нажатия на кнопку когда станок остановлен, а станок ЗАПУЩЕН и с ним ничего ненадо делать
  if(katana==true){
    if(digitalRead(stanokPinInput)==HIGH){
      buzzer.StopMelody();
      Serial.println("I am mute buzzer no waiting 5 minutes");
      katana = false;
    }
  }
  // Cам наш алгоритм работы
}