/*
 * File:   main.c
 * Project: DinoJumper
 * 
 * Program for ATMega4809 to play Google Chrome Dino jumping game to 
 * atleast 200 points. Works by detecting light level changes with
 * a photoresistor and then presses down the spacebar on keyboard 
 * using servo.
 * 
 * Required materials: 
 * 1x ATMega4809
 * 1x Breadboad
 * 1x 4.8V Servo
 * 1x Photoresistor
 * 1x 10k resistor'
 * Connectors
 * 
 * Author: Topias Yrjölä, Matti Loimaranta, Arttu Huttunen
 * Emails: tokryr@utu.fi, makalo@utu.fi, arhutt@utu.fi.
 * Created on 18 December 2020, 19:48
 * Version 2.0
 */

#define F_CPU 3333333
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#define PWM_PERIOD (0x0FFF)

// 1-255 Set this as value for when the servo is pressing spacebar down
#define SERVO_DOWN (0x00FF)
// 1-255 Set this as value for when the servo is not pressing
#define SERVO_UP (0x00DC)
// LDR gives adc value of 0-1023
// Threshold that once the value crosses the dino jumps
#define JUMPTHRESHOLD (700)


volatile uint16_t adc_val = 0;

void ADC0_init(void);
void TCA_init(void);

void ADC0_init(void)
{
    /* Disable digital input buffer */
    PORTE.PIN0CTRL &= ~PORT_ISC_gm;
    PORTE.PIN0CTRL |= PORT_ISC_INPUT_DISABLE_gc;

    /* Disable pull-up resistor */
    PORTE.PIN0CTRL &= ~PORT_PULLUPEN_bm;

    ADC0.CTRLC = ADC_PRESC_DIV4_gc      /* CLK_PER divided by 4 */
               | ADC_REFSEL_INTREF_gc;  /* Internal reference */

    ADC0.CTRLA = ADC_ENABLE_bm          /* ADC Enable: enabled */
               | ADC_RESSEL_10BIT_gc;   /* 10-bit mode */
    
    /* Select ADC channel */
    ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc;
    
    /* Enable FreeRun mode */
    ADC0.CTRLA |= ADC_FREERUN_bm;
    
    /* Set VREF to 2,5V */
    VREF.CTRLA |= VREF_ADC0REFSEL_2V5_gc; 

    /* Enable interrupts */
    ADC0.INTCTRL |= ADC_RESRDY_bm;
    
     /* Enable event triggered conversion */
    ADC0.EVCTRL |= ADC_STARTEI_bm;

}

void TCA_init()
{
    /* set waveform output on PORT A */
    PORTMUX.TCAROUTEA |= PORTMUX_TCA0_PORTB_gc;

    // Set PB2 output, servo signal is connected there
    PORTB.DIRSET = PIN2_bm;

   
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV8_gc // Set prescaler to 8
                        | TCA_SINGLE_ENABLE_bm; // Enable bit

    // Set single-slope PWM  mode
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;

    // Set PWM frequency
    TCA0.SINGLE.PERBUF = PWM_PERIOD;

    // Set servo to UP to not press spacebar
    TCA0.SINGLE.CMP2BUF = SERVO_UP;

    // Enable compare channel 2
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm;

    // Enable TCA0 peripheral
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
   
}

int main(void) 
{
    ADC0_init();
    TCA_init();
    
    // Set port for LDR
    PORTE.IN = PORTE.IN | PIN0_bm;

    // Set IDLE sleep mode
    set_sleep_mode(SLPCTRL_SMODE_IDLE_gc);

    // Enable global interrupts
    sei();
    
    ADC0.COMMAND = ADC_STCONV_bm;
    
    while (1)
    {
        // Jump if cactus
        // Currently on dark mode, (adc_val > 300 if playing on lightmode)
        if (adc_val < JUMPTHRESHOLD)
        {
            TCA0.SINGLE.CMP2BUF = SERVO_DOWN;
        }
        else
        {
            TCA0.SINGLE.CMP2BUF = SERVO_UP;
        }
        // Sleeps until ADC triggers interrupt from LDR changing light levels
        sleep_mode();
    }

}

ISR(ADC0_RESRDY_vect)
{
    /* Set LDR converted adc value to variable*/
    adc_val = ADC0.RES;
    /* Clear the interrupt flag by writing 1: */
    ADC0.INTFLAGS = ADC_RESRDY_bm;
}