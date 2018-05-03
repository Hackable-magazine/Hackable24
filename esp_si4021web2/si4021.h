#ifndef SI4021_H
#define SI4021_H

/*
   Freq set :
   Frequency Setting Command
   Fo = 10 Mhz * C1 * (C2 + F/4000)
   F = f11..f0 -> 96:3903

   Conf set:
   Configuration Setting Command
   15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    1  0  0 b1 b0 d2 d1 d0 x3 x2 x1 x0 ms m2 m1 m0
               |        |           |  |        |
               |        |           |  |        M \__ freq deviation for FSK
               |        |           |  sign       /
               |        |           xtal capa : CS_CAPA85..CS_CAPA160
               |        clock output : CS_CLK1000..CS_CLK10000
               band freq : CS_BAND315..CS_BAND915

   Fout = Fo - (-1)^sign * (M+1) * 30khz
   M = m2..m0
   sign = ms XOR FSKinput

   Power man:
   Power Management Command
   15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    1  1  0  0  0  0  0  0 a1 a0 ex es ea eb et dc
                           |  |  |  |  |  |  |  |
                           |  |  |  |  |  |  |  clock output buffer : PM_CBUF
                           |  |  |  |  |  |  wake-up timer : PM_WAKUP
                           |  |  |  |  |  low battery detector : PM_LOWBAT
                           |  |  |  |  power amplifier enabled : PM_AMP
                           |  |  |  synthesizer enabled : PM_SYNT
                           |  |  crystal oscillator enabled : PM_OSC
                           |  power amplifier on : PM_AMPON
                           crystal oscillator and synthesizer auto-control : PM_AUTOCTL

   Power Setting Command
   7  6  5  4  3  2  1  0
   1  0  1  1 ook p2 p1 p0
                   (power) : PS_OOK..PS_21DB
*/

// Commands
#define CONFSET     (4 << 13)
#define FREQSET     (10 << 12)
#define POWERMAN    (192 << 8)
#define POWERSET    (11 << 4)

// Power Management bits
#define PM_CBUF     1
#define PM_WAKUP    2
#define PM_LOWBAT   4
#define PM_AMP      8
#define PM_SYNT     16
#define PM_OSC      32
#define PM_AMPON    64
#define PM_AUTOCTL  128

// Power Setting bits
#define PS_OOK      (1 << 3)
#define PS_0DB      0
#define PS_3DB      1
#define PS_6DB      2
#define PS_9DB      3
#define PS_12DB     4
#define PS_15DB     5
#define PS_18DB     6
#define PS_21DB     7

// Configuration Setting bits
#define CS_BAND315      (0 << 11)
#define CS_BAND433      (1 << 11)
#define CS_BAND868      (2 << 11)
#define CS_BAND915      (3 << 11)

#define CS_CLK1000      (0 << 8)
#define CS_CLK1250      (1 << 8)
#define CS_CLK1660      (2 << 8)
#define CS_CLK2000      (3 << 8)
#define CS_CLK2500      (4 << 8)
#define CS_CLK3330      (5 << 8)
#define CS_CLK5000      (6 << 8)
#define CS_CLK10000     (7 << 8)

#define CS_CAPA85       (0 << 4)  // 8.5pf
#define CS_CAPA90       (1 << 4)  // 9pf
#define CS_CAPA95       (2 << 4)  // 9.5pf
#define CS_CAPA100      (3 << 4)  // 10pf
#define CS_CAPA105      (4 << 4)  // 10.5pf
#define CS_CAPA110      (5 << 4)  // 11pf
#define CS_CAPA115      (6 << 4)  // 11.5pf
#define CS_CAPA120      (7 << 4)  // 12pf
#define CS_CAPA125      (8 << 4)  // 12.5pf
#define CS_CAPA130      (9 << 4)  // 13pf
#define CS_CAPA135      (10 << 4) // 13.5pf
#define CS_CAPA140      (11 << 4) // 14pf
#define CS_CAPA145      (12 << 4) // 14.5pf
#define CS_CAPA150      (13 << 4) // 15pf
#define CS_CAPA155      (14 << 4) // 15.5pf
#define CS_CAPA160      (15 << 4) // 16pf

// M (m1 m2 m3) & MS not used in OOK
#define CS_MS           (1 << 3)  // freq deviation sign (MS xor FSK_input)

// Frequency Setting
#define FS_BAND433_C1 1
#define FS_BAND433_C2 43
#define FS_BAND868_C1 2
#define FS_BAND868_C2 43
#define FS_BAND915_C1 3
#define FS_BAND915_C2 30

uint16_t freqset(float freq);
uint16_t confset(float freq);
uint8_t powerset(unsigned int attenuation);
uint16_t powerman();
void sendToSi4021(uint8_t pincs, uint8_t val);
void sendToSi4021(uint8_t pincs, uint16_t val);

#endif 
