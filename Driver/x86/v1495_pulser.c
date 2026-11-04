#include <stdint.h>
#include <stdio.h>

#include "jvme.h"
#include "v1495_pulser.h"

#define DEBUG_REG_RW 0

static volatile V1495_Pulser_regs *pPulser;

static unsigned short
ReadReg16(volatile unsigned short *addr)
{
  unsigned short result = vmeRead16(addr);

#if DEBUG_REG_RW
  printf("ReadReg16 @ %p = 0x%04hX\n", (void *)addr, result);
#endif
  return result;
}

static void
WriteReg16(volatile unsigned short *addr, unsigned short val)
{
  vmeWrite16(addr, val);
#if DEBUG_REG_RW
  printf("WriteReg16 @ %p = 0x%04hX\n", (void *)addr, val);
#endif
}

static unsigned int
ReadReg32(volatile unsigned int *addr)
{
  unsigned int result = vmeRead32(addr);

#if DEBUG_REG_RW
  printf("ReadReg32 @ %p = 0x%08X\n", (void *)addr, result);
#endif
  return result;
}

static void
WriteReg32(volatile unsigned int *addr, unsigned int val)
{
  vmeWrite32(addr, val);
#if DEBUG_REG_RW
  printf("WriteReg32 @ %p = 0x%08X\n", (void *)addr, val);
#endif
}

int
V1495PulserInit(unsigned int a24_base)
{
  char *local_addr = NULL;
  unsigned int result;
  int status;

  printf("\nV1495PulserInit() started...\n");

  status = vmeBusToLocalAdrs(0x39, (char *)(uintptr_t)a24_base,
                             &local_addr);
  if(status != 0)
  {
    printf("Error: failed to map A24 VME address 0x%08X (status=%d)\n",
           a24_base, status);
    pPulser = NULL;
    return -1;
  }

  pPulser = (volatile V1495_Pulser_regs *)local_addr;
  printf("   V1495Pulser A24 VME address 0x%08X mapped to CPU address %p\n",
         a24_base, (void *)pPulser);

  result = ReadReg32(&pPulser->PulserId);
  printf("   V1495Pulser Pulser Id register = 0x%08X\n", result);
  if(result != PULSER_ID)
  {
    printf("Error: failed to find V1495Pulser at A24 VME address 0x%08X\n",
           a24_base);
    pPulser = NULL;
    return -1;
  }

  V1495PulserStatus();
  return 0;
}

void
V1495PulserUserReload(void)
{
  WriteReg16(&pPulser->configUSER, 1);
}

void
V1495PulserStatus(void)
{
  unsigned int result;
  unsigned int value;

  printf("\nV1495PulserStatus:\n");
  printf("   V1495Pulser VME Firmware Revision = 0x%08X\n",
         ReadReg16(&pPulser->firmwareRev));
  printf("   V1495Pulser Pulser Firmware Revision = 0x%08X\n",
         ReadReg32(&pPulser->FirmwareRev));

  result = ReadReg32(&pPulser->DaughterBoardId);
  printf("   V1495Pulser Pulser Daughtercard Id: D=%d, E=%d, F=%d\n",
         (result >> 0) & 0x7, (result >> 4) & 0x7,
         (result >> 8) & 0x7);

  printf("   Pulser Status:          0x%08X_%08X\n",
         ReadReg32(&pPulser->PulserStatusH),
         ReadReg32(&pPulser->PulserStatusL));
  printf("   Pulser Start Mask:      0x%08X_%08X\n",
         ReadReg32(&pPulser->PulserStartMaskH),
         ReadReg32(&pPulser->PulserStartMaskL));
  printf("   Pulser Stop Mask:       0x%08X_%08X\n",
         ReadReg32(&pPulser->PulserStopMaskH),
         ReadReg32(&pPulser->PulserStopMaskL));
  printf("   Pulser GIN OR Mask:     0x%08X_%08X\n",
         ReadReg32(&pPulser->PulserGinMaskH),
         ReadReg32(&pPulser->PulserGinMaskL));

  value = ReadReg32(&pPulser->MasterOrDelay);
  printf("   Pulser Master OR Delay: 0x%08X (%u ns)\n", value,
         value * PULSER_TICK_PERIOD_NS);
  value = ReadReg32(&pPulser->MasterOrWidth);
  printf("   Pulser Master OR Width: 0x%08X (%u ns)\n", value,
         value * PULSER_TICK_PERIOD_NS);
  value = ReadReg32(&pPulser->RandPulserFreq);
  printf("   Pulser Random Freq:     0x%08X (%u Hz)%s\n", value, value,
         value ? "" : " [Disabled]");
  value = ReadReg32(&pPulser->RandPulserWidth);
  printf("   Pulser Random Width:    0x%08X (%u cycles, %u ns)\n",
         value, value, (value + 1) * 5);
  printf("   Pulser Jumpers:         0x%08X\n",
         ReadReg32(&pPulser->Jumpers));
}

void
V1495PulserGetCfg(int pulser)
{
  unsigned int result;

  printf("\nV1495PulserGetCfg(%d)\n", pulser);
  if((pulser < 0) || (pulser >= PULSER_NUM))
  {
    printf("Error: invalid pulser id %d\n", pulser);
    return;
  }

  result = ReadReg32(&pPulser->Pulsers[pulser].Period);
  printf("   Period     = 0x%08X (%u ns)\n", result,
         (result - 1) * PULSER_TICK_PERIOD_NS);
  result = ReadReg32(&pPulser->Pulsers[pulser].Width);
  printf("   HighCycles = 0x%08X (%u ns)\n", result,
         result * PULSER_TICK_PERIOD_NS);
  result = ReadReg32(&pPulser->Pulsers[pulser].NPulses);
  printf("   NPulses    = 0x%08X (%u)\n", result, result);
}

void
V1495PulserSetMasterOrCfg(unsigned int width, unsigned int delay)
{
  WriteReg32(&pPulser->MasterOrWidth, width);
  WriteReg32(&pPulser->MasterOrDelay, delay);
}

void
V1495PulserSetGinOrMask(unsigned int maskL, unsigned int maskH)
{
  WriteReg32(&pPulser->PulserGinMaskH, maskH);
  WriteReg32(&pPulser->PulserGinMaskL, maskL);
}

void
V1495PulserSetCfg(int pulser, unsigned int period_ticks,
                  unsigned int width, unsigned int npulses)
{
  if((pulser < 0) || (pulser >= PULSER_NUM))
  {
    printf("Error: invalid pulser id %d\n", pulser);
    return;
  }

  WriteReg32(&pPulser->Pulsers[pulser].NPulses, npulses);
  WriteReg32(&pPulser->Pulsers[pulser].Width, width);
  WriteReg32(&pPulser->Pulsers[pulser].Period, period_ticks);
}

void
V1495PulserSetCfgAll(unsigned int period_ticks, unsigned int width,
                     unsigned int npulses)
{
  int pulser;

  for(pulser = 0; pulser < PULSER_NUM; pulser++)
    V1495PulserSetCfg(pulser, period_ticks, width, npulses);
}

void
V1495PulserStart(unsigned int maskL, unsigned int maskH)
{
  WriteReg32(&pPulser->PulserStartMaskH, maskH);
  WriteReg32(&pPulser->PulserStartMaskL, maskL);
  WriteReg32(&pPulser->PulserStartStop, 0x1);
}

void
V1495PulserStop(unsigned int maskL, unsigned int maskH)
{
  WriteReg32(&pPulser->PulserStopMaskH, maskH);
  WriteReg32(&pPulser->PulserStopMaskL, maskL);
  WriteReg32(&pPulser->PulserStartStop, 0x0);
}

void
V1495PulserSetRandCfg(unsigned int freq_hz, unsigned int width_cycles)
{
  WriteReg32(&pPulser->RandPulserFreq, freq_hz & 0x00FFFFFF);
  WriteReg32(&pPulser->RandPulserWidth, width_cycles & 0xFF);
}

void
V1495PulserGetRandCfg(unsigned int *freq_hz, unsigned int *width_cycles)
{
  unsigned int freq = ReadReg32(&pPulser->RandPulserFreq);
  unsigned int width = ReadReg32(&pPulser->RandPulserWidth);

  if(freq_hz)
    *freq_hz = freq;
  if(width_cycles)
    *width_cycles = width;

  printf("\nV1495PulserGetRandCfg:\n");
  printf("   Random Frequency = %u Hz (0x%06X)%s\n", freq, freq,
         freq ? "" : " [Disabled]");
  printf("   Random Width     = %u cycles (%u ns)\n", width,
         (width + 1) * 5);
}

void
V1495PulserRandEnable(unsigned int enable, unsigned int freq_hz,
                      unsigned int width_cycles)
{
  if(!enable)
  {
    WriteReg32(&pPulser->RandPulserFreq, 0);
  }
  else
  {
    WriteReg32(&pPulser->RandPulserWidth, width_cycles & 0xFF);
    WriteReg32(&pPulser->RandPulserFreq, freq_hz & 0x00FFFFFF);
  }
}
