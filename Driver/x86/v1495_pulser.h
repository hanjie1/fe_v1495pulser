#ifndef V1495_PULSER_H
#define V1495_PULSER_H

#define PULSER_NUM_MAX          256
#define PULSER_NUM              56
#define PULSER_TICK_PERIOD_NS   10
#define PULSER_ID               0x50554C53

#define DAUGHTERBOARD_ID_A395A  0x0
#define DAUGHTERBOARD_ID_A395B  0x1
#define DAUGHTERBOARD_ID_A395C  0x2
#define DAUGHTERBOARD_ID_A395D  0x3

#define PULSER_IDX_C(n)         (0 + (n))
#define PULSER_IDX_D(n)         (31 + (n))
#define PULSER_IDX_E(n)         (39 + (n))
#define PULSER_IDX_F(n)         (47 + (n))

typedef struct
{
  volatile unsigned int Period;
  volatile unsigned int Width;
  volatile unsigned int NPulses;
  volatile unsigned int Reserved0;
} PulserPeriph;

typedef struct
{
  volatile unsigned int Reserved0[(0x1000-0x0000)/4];
  volatile unsigned int PulserId;
  volatile unsigned int FirmwareRev;
  volatile unsigned int DaughterBoardId;
  volatile unsigned int Jumpers;
  volatile unsigned int PulserStatusH;
  volatile unsigned int PulserStatusL;
  volatile unsigned int PulserStartMaskH;
  volatile unsigned int PulserStartMaskL;
  volatile unsigned int PulserStopMaskH;
  volatile unsigned int PulserStopMaskL;
  volatile unsigned int PulserGinMaskH;
  volatile unsigned int PulserGinMaskL;
  volatile unsigned int PulserStartStop;
  volatile unsigned int NimTtl;
  volatile unsigned int RandPulserFreq;
  volatile unsigned int RandPulserWidth;
  volatile unsigned int Reserved1[(0x1100-0x1040)/4];
  volatile unsigned int MasterOrDelay;
  volatile unsigned int MasterOrWidth;
  volatile unsigned int Reserved2[(0x2000-0x1108)/4];
  PulserPeriph Pulsers[PULSER_NUM_MAX];
  volatile unsigned int Reserved3[(0x8000-0x3000)/4];
  volatile unsigned short control;
  volatile unsigned short status;
  volatile unsigned short intLevel;
  volatile unsigned short intVector;
  volatile unsigned short geoAddr;
  volatile unsigned short moduleReset;
  volatile unsigned short firmwareRev;
  volatile unsigned short selflashVME;
  volatile unsigned short flashVME;
  volatile unsigned short selflashUSER;
  volatile unsigned short flashUSER;
  volatile unsigned short configUSER;
  volatile unsigned short scratch16;
  volatile unsigned short res1[3];
  volatile unsigned int scratch32;
  volatile unsigned short res2[110];
  volatile unsigned short configROM[127];
} V1495_Pulser_regs;

int V1495PulserInit(unsigned int a24_base);
void V1495PulserUserReload(void);
void V1495PulserStatus(void);
void V1495PulserGetCfg(int pulser);
void V1495PulserSetMasterOrCfg(unsigned int width, unsigned int delay);
void V1495PulserSetGinOrMask(unsigned int maskL, unsigned int maskH);
void V1495PulserSetCfg(int pulser, unsigned int period_ticks,
                       unsigned int width, unsigned int npulses);
void V1495PulserSetCfgAll(unsigned int period_ticks, unsigned int width,
                          unsigned int npulses);
void V1495PulserStart(unsigned int maskL, unsigned int maskH);
void V1495PulserStop(unsigned int maskL, unsigned int maskH);
void V1495PulserSetRandCfg(unsigned int freq_hz,
                           unsigned int width_cycles);
void V1495PulserGetRandCfg(unsigned int *freq_hz,
                           unsigned int *width_cycles);
void V1495PulserRandEnable(unsigned int enable, unsigned int freq_hz,
                           unsigned int width_cycles);

#endif
