#include <msp430.h> 
#include <stdint.h>
#include "driverlib.h"
#include "./radio/radio.h"

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    // Initialise Clock
    //Set DCO frequency to max DCO setting
    CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_3);
    //Select DCO as the clock source for SMCLK with no frequency divider
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    // Initialise GPIO
    // RX Switch
    GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN5);

    // TX Switch
    GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN6);

    // Radio NSS
    GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN3);
    GPIO_setOutputHighOnPin(GPIO_PORT_P7, GPIO_PIN3);

    // RF_RST
    GPIO_setAsOutputPin(GPIO_PORT_P8,GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN0);

    GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P7,
            GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2,
            GPIO_PRIMARY_MODULE_FUNCTION
    );

    PM5CTL0 &= ~LOCKLPM5;

    // Initialise SPI
    EUSCI_A_SPI_initMasterParam param = {0};
    param.selectClockSource = EUSCI_A_SPI_CLOCKSOURCE_SMCLK;
    param.clockSourceFrequency = CS_getSMCLK();
    param.desiredSpiClock = 500000;
    param.clockPhase = EUSCI_A_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT;
    param.clockPolarity = EUSCI_A_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
    param.msbFirst = EUSCI_A_SPI_MSB_FIRST;
    param.spiMode = EUSCI_A_SPI_3PIN;
    EUSCI_A_SPI_initMaster(EUSCI_A2_BASE, &param);

    // Enable SPI
    EUSCI_A_SPI_enable(EUSCI_A2_BASE);

    // Initialise Radio
    init_radio();

    // Prepare LoRaWAN PHY Payload
    // Prepare MAC Payload
    const uint8_t dummy_payload_len = 10;
    uint8_t dummy_payload[dummy_payload_len] = {1,2,3,4,5,6,7,8,9,10};

    // Configure Radio and Send
    config_lora();
    send_lora(dummy_payload, dummy_payload_len);


    while(1);
}
