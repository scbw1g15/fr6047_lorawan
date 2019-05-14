/*
 * radio.c
 *
 *  Created on: 10 May 2019
 *      Author: Samuel
 *      Target: SX1272 LoRa Module
 */

#include "radio.h"

void send_lora(uint8_t * data, uint8_t len){

    // initialize the payload size and address pointers
    write_access(LORARegFifoTxBaseAddr, 0x00);
    write_access(LORARegFifoAddrPtr, 0x00);
    write_access(LORARegPayloadLength, len);

    // write data to radio fifo
    write_buff(RegFifo, data, len);

    // turn on tx switch off rx switch
    GPIO_setOutputHighOnPin(GPIO_PORT_P7, GPIO_PIN6);
    GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN5);

    // send
    set_radio_mode(OPMODE_TX);
}

void write_buff(uint8_t address, uint8_t * data, uint8_t len){
    GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN3);
    EUSCI_A_SPI_transmitData(EUSCI_A2_BASE,address | 0b10000000);
    uint8_t i;
    for(i=0; i<len; i++) {
        EUSCI_A_SPI_transmitData(EUSCI_A2_BASE,data[i]);
    }
    GPIO_setOutputHighOnPin(GPIO_PORT_P7, GPIO_PIN3);

}

void config_lora(){
    // while in sleep mode
    set_radio_mode(OPMODE_LORA);
    set_radio_mode(OPMODE_STANDBY);

    // set the carrier frequency to 868 MHz
    write_access(RegFrfMsb, 0xD7);
    write_access(RegFrfMid, 0x00);
    write_access(RegFrfLsb, 0x00);

    // configure bandwidth (125 kHz), err coding rate (4/5), explicit header, CRC on
    // spreading factor 7
    //write_access(LORARegModemConfig1, 0b00001010);
    //write_access(LORARegModemConfig2, 0b01110000);
    // configure bandwidth (250 kHz), ecr (4/5), explicit header, crc on
    // spreading factor 12
    write_access(LORARegModemConfig1, 0b01001011);
    write_access(LORARegModemConfig2, 0b11000000);

    // configure output power
    write_access(RegPaRamp, (read_access(RegPaRamp) & 0xF0) | 0x08); // set PA ramp-up time 50 uSec
    write_access(RegPaConfig, 0x80 | 15); // max power

    // set sync word
    write_access(LORARegSyncWord, LORA_MAC_PREAMBLE);

    // set the IRQ mapping DIO0=TxDone DIO1=NOP DIO2=NOP
    write_access(RegDioMapping1, MAP_DIO0_LORA_TXDONE|MAP_DIO1_LORA_NOP|MAP_DIO2_LORA_NOP);
    // clear all radio IRQ flags
    write_access(LORARegIrqFlags, 0xFF);
    // mask all IRQs but TxDone
    write_access(LORARegIrqFlagsMask, ~IRQ_LORA_TXDONE_MASK);

}

void init_radio(){
    reset_radio();
    set_radio_mode(OPMODE_SLEEP);
}

void reset_radio(){
    // reset radio
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0);
    __delay_cycles(800); // 100 us
    GPIO_setAsInputPin(GPIO_PORT_P8, GPIO_PIN0);
    __delay_cycles(40000); // 5 ms
}

void set_radio_mode(uint8_t mode){
    write_access(RegOpMode,(read_access(RegOpMode) & ~OPMODE_MASK) | mode);
}

uint8_t read_access(uint8_t address){
    GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN3);
    EUSCI_A_SPI_transmitData(EUSCI_A2_BASE,address & 0b01111111);
    EUSCI_A_SPI_transmitData(EUSCI_A2_BASE,0x00);
    GPIO_setOutputHighOnPin(GPIO_PORT_P7, GPIO_PIN3);
    return EUSCI_A_SPI_receiveData(EUSCI_A2_BASE);
}

void write_access(uint8_t address, uint8_t data){
    GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN3);
    EUSCI_A_SPI_transmitData(EUSCI_A2_BASE,address | 0b10000000);
    EUSCI_A_SPI_transmitData(EUSCI_A2_BASE,data);
    GPIO_setOutputHighOnPin(GPIO_PORT_P7, GPIO_PIN3);
}
