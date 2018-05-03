#include <ESP8266WiFi.h>
#include <SPI.h>
#include "si4021.h"

/***** Fonctions pour calculer les bits de configuration *****/

// réglage fréquence 
uint16_t freqset(float freq) {
  int c1;
  int c2;
  uint16_t ret;
  if(freq < 440) {
    c1 = FS_BAND433_C1;
    c2 = FS_BAND433_C2;
  } else if (freq < 880) {
    c1 = FS_BAND868_C1;
    c2 = FS_BAND868_C2;
  } else if (freq < 930) {
    c1 = FS_BAND915_C1;
    c2 = FS_BAND915_C2;
  } else {
    return(0);
  }
  ret = (((freq/10/c1)-c2)*4000)+0.5; // rounding
  if(ret > 3903 || ret < 96)
    return(0);
  return(ret | FREQSET);
}

// réglage configuration, capa et bande
uint16_t confset(float freq) {
  if(freq < 440) {
    return(CONFSET | CS_CLK10000 | CS_BAND433 | CS_CAPA160);
  } else if (freq < 880) {
    return(CONFSET | CS_CLK10000 | CS_BAND868 | CS_CAPA160);
  } else if (freq < 930) {
    return(CONFSET | CS_CLK10000 | CS_BAND915 | CS_CAPA160);
  } else {
    return(0);
  }
}

// réglage modulation et puissance
uint8_t powerset(unsigned int attenuation) {
  if(attenuation < 8)
  return(POWERSET | PS_OOK | attenuation);
  return(0);
}

// réglage ampli
uint16_t powerman() {
  return(POWERMAN | PM_OSC | PM_SYNT | PM_AMP);
}

// Envoi de valeur au si4021 en SPI
// (ce n'est pas une erreur, en C++ ceci est la surchare de fonction)
void sendToSi4021(uint8_t pincs, uint8_t val) {
  digitalWrite(pincs, LOW);
  SPI.transfer(val);
  digitalWrite(pincs, HIGH);
}
void sendToSi4021(uint8_t pincs, uint16_t val) {
  digitalWrite(pincs, LOW);
  SPI.transfer(val >> 8);
  SPI.transfer(val & 255);
  digitalWrite(pincs, HIGH);
}
