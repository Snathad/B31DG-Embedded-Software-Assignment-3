

//Charles Birdsall H00219071 15.4.22//
//B32DG Embedded Software Assignment3//
//This version attempts to add RTOS functionality for each task along with a queue system//

//The time library used
#include <time.h>

//Establish pins as variables
const int green_led = 21;

//Task 1 variables
//Establish variables. a,b,c & d are used to define the lengths of LED pulses, as taken from assignment1. //
const int a=0.2;
const int b=0.9;
int c=14;
const int d=2;
const int L = 0.05; //delay to keep the LED on for task 1
int T;  //The total time variable for calculating task1 pulses

//Task2 variables
const int push_switch1 = 23; //establish input for push switch
int switch_flag = LOW;// A flag used to say if the switch was pressed.

//Task 3 variables
//establish frequency input pin as a variable//
const int freq_in = 34;
int frequency = 0; //the value of the frequency in Hz
int freq_count = 0; //The number of pulses over the time period
int raw_value; //The unprocessed input value
int raw_value_old; //The previous unprocessed input value
unsigned long start_timeF; // used to determine the time at which the frequency check was started
unsigned long currentTime; //used to determine the current time

//Tasks 4 variables
const int analogue_in = 35;//Establish analogue read as a variable

//Task 5 variables
//The past four analogue values are stored in analogue_hist0-3
float analogue_hist0;
float analogue_hist1;
float analogue_hist2;
float analogue_hist3;
float analogue_average ; //The average analogue value

//Task 7 variables
const float analogue_max = 3.3; //sets a limit on the maximum analogue value for error processing
int error_code = 0; //a flag for whether the error condition is met or not

//Task 8 variables
const int red_led = 15; //establish the red LED to be used as an error output.

//Define each of the tasks for freeRTOS
void wave_out (void *pvParameters);
void switchread (void *pvParameters);
void freq_measure (void *pvParameters);
void analogue_read (void *pvParameters);
void analogue_filter (void *pvParameters);
void Task_6 (void *pvParameters);
void analogue_error (void *pvParameters);
void error_out (void *pvParameters);
void log_out (void *pvParameters);

xQueueHandle tasks5and7 = xQueueCreate (2, sizeof(int));
///////////////////////////////////////////////////////////////

void setup() {
  //Establish Pins
  pinMode(green_led, OUTPUT);//establish LED pin as an output
  pinMode(red_led, OUTPUT); //establish LED pin as an output
  pinMode(push_switch1, INPUT); //set button pin as INPUT
  pinMode (freq_in, INPUT); //set the frequency read pin as an input 
  pinMode(analogue_in, INPUT); //set the analogue read from the potentiometer as an input
  
  T=(c*a)+(b*c)+L*(1+2+3+4+5+6+7+8+9+10+11+12+13); //The delay value from assignment one for task one

  //Define the historical analogue read values to shift them
  //Please note a bitshifter and array are seen in earlier iterations
  analogue_hist0=analogRead(analogue_in);
  analogue_hist1=analogue_hist0;
  analogue_hist2=analogue_hist1;
  analogue_hist3=analogue_hist2;
  
  Serial.begin(9600); //start serial plotter

  mainTimings.attach_ms(10, ticker_count);
  float clock_val=0;

//This section creates tasks for each function with equal priorities for now
  xTaskCreate(wave_out, "wave_out", 1024, NULL, 1, NULL);
  xTaskCreate(switchread, "switchread", 1024, NULL, 1, NULL);
  xTaskCreate(freq_measure, "freq_measure", 1024, NULL, 1, NULL);
  xTaskCreate(analogue_read, "analogue_read", 1024, NULL, 1, NULL);
  xTaskCreate(analogue_filter, "analogue_filter", 1024, NULL, 1, NULL);
  xTaskCreate(task6, "task6", 1024, NULL, 1, NULL);
  xTaskCreate(analogue_error, "analogue_error", 1024, NULL, 1, NULL);
  xTaskCreate(error_out, "error_out", 1024, NULL, 1, NULL);
  xTaskCreate(log_out, "log_out", 1024, NULL, 1, NULL);
}


///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//Task1
void wave_out(void *parameter){
   //the task1 watchdog
    digitalWrite(green_led,HIGH); // turn green led on
    vTaskDelay(L / portTICK_PERIOD_MS);
    digitalWrite(green_led,LOW);
}

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//task 2
void switchread(void *parameter){
  for(;;){
    int ps1s = digitalRead(push_switch1); //set variable that is read from switch value
    if (ps1s==HIGH){
      switch_flag=HIGH;
      }
    else if (ps1s==LOW){
      switch_flag=LOW;
      }
    }
}


///////////////////////////////////////////////////////////////////////////////
//task 3
//This subfunction measures the frequency from the input
void freq_measure (void *parameter){
  for(;;){
    raw_value=digitalRead(freq_in);  //set the raw value to be equal to the digital read from the input
    raw_value_old= raw_value;  //set the old value equal to the current
    freq_count=0; //reset the frequency count to zero   
 
    start_timeF= micros(); //define the start time of the clock
    currentTime=micros(); // define current time

    //It was found using one time alone had a processing lag which had to be factored in
    while ((currentTime-start_timeF) < 40000){  //effectively delays for 0.04seconds, the optimum time for detecting frequency that keeps errors within +/- 2.5%
      raw_value=digitalRead(freq_in); //remeasure the actual input
       
      if (raw_value_old != raw_value){ //If frequency input is high and was previously low (ie flag is on) add one to the value of the frequency counter.
        freq_count++; //increase frequency
        raw_value_old=raw_value;//update the old frequency value
        }
      } 
    if (micros()>= start_timeF +40000){ //calculate the frequency by scaling 0.04s up to 1s
        frequency =freq_count*25/2;  
        //Serial.println("Frequency condition is met"); //a flag used to indicate whether this stage is operating
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//task 4
void analogue_process(void *parameter){
  for(;;){
    //Read in the analogue values and shift the previous values along one
    analogue_hist3=analogue_hist2;
    analogue_hist2=analogue_hist1;
    analogue_hist1=analogue_hist0;
    analogue_hist0=analogRead(analogue_in)*(3.3/4095); //multiplies input by scaling factor for analogue inputs
  }
}


///////////////////////////////////////////////////////////////////////////////
//task 5
void analogue_filter(void *parameter){
  for(;;){
    analogue_average= (analogue_hist0+analogue_hist1+analogue_hist2+analogue_hist3)/4; //take average of analogue values
    //optional prints to show status
    //Serial.print("filtered analog value");
    //Serial.println(analogue_average);
    //Serial.print(analogue_hist0);
  }
}


///////////////////////////////////////////////////////////////////////////////
//task 7
void analogue_error(void *parameter){
  for(;;){
    if (analogue_hist0 > (0.5*analogue_max)){ //if condition is met return positive error code
      error_code = 1;
      }
    else if (analogue_hist0 < (0.5*analogue_max)){ //if condition is not met, return negative error code
      error_code = 0;  
      }
  }
}

///////////////////////////////////////////////////////////////////////////////
//task8
void error_out(void *parameter){
  for(;;){
    if (error_code==1){ //if error code is positive, turn red LED on
      digitalWrite(red_led,HIGH);
    }
    else if (error_code!=1){ //if error code is no longer positive, turn red LED off
      digitalWrite(red_led,LOW);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//task 6
void task6 (void *parameter){//perform 1000 times
  for(;;){
    int i;
    for (i=1; i<1000; +i){
      __asm__ __volatile__ ("nop");
    }
  }
}
///////////////////////////////////////////////////////////////////////////////
//task 9
void log_out (void *parameter){
  if (switch_flag==HIGH){
    Serial.println("Switch is pressed");  //if the input is high print that the button is pressed
    Serial.print("frequency = ");
    Serial.print(frequency);
    Serial.println("Hz"); //print the processed frequency
    Serial.print("Filtered analogue input =");
    Serial.print(analogue_average);
    Serial.println("V");
    switch_flag=LOW;
}

///////////////////////////////////////////////////////////////////////////////
//The attempted queue linking tasks five and seven. this is not correctly implemented
void Sender_5and7_queue (void *argument)
{
  int i=222;
  uint32_t TickDelay = pdMS_TO_TICKS(2000);
  while (1)
  {
    if (xQueueSend(tasks5and7, &i, portMAX_DELAY) == pdPASS)
    {
      //char *str2 = " Successfully sent to queue\nLeaving SENDER_HPT Task\n\n\n";
      HAL_UART_Transmit(&huart2, (uint8_t *)str2, strlen (str2), HAL_MAX_DELAY);
    }
    vTaskDelay(TickDelay);
  }
}

///////////////////////////////////////////////////////////////////////////////
void loop() {
  
  if (tasks5and7==0){ //alternate messages for the circumstance that the queue does not fill correctly
    char *str = "Unable to create queue for tasks 5 and 7\n\n";
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen (str), HAL_MAX_DELAY);
  }
  else
  {
    char *str = "Queue successf\n\n";
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen (str), HAL_MAX_DELAY);
  }

  
  

  if (clock_val%4==0){
    analogue_process(); //call analogue processing function
    analogue_filter(); //call analogue averaging function
  }

  if (clock_val%33==0){
    analogue_error(); //call analogue error function
    error_out(); //call analogue error visual alarm function
  }

  if (clock_val%20==0){
    switchread(); //call switchread function
  }

  if(clock_val%10==0){
    task6(); //call 1000iter task six function
  }

  if(clock_val%100==0){
    freq_measure();  //call frequency measuring function
  }

  
  
  }
