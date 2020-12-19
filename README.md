Program for ATMega4809 to play Google Chrome Dino jumping game to atleast 200 points. Works by detecting light level changes with a photoresistor and then presses down the spacebar on keyboard using servo.
Initializes TCA for PWM and ADC in the beginning, then compares ADC ready result from LDR(photoresistor) to a set threshold value. If the threshold is crossed, then powers Servo using PWM to press down on spacebar.
Program then goes to IDLE until ADC result is ready interrupts IDLE mode and compares the result to threshold again.
