"""Compile the real protection/callback sources and run ARM software tests.

Usage: python Tests/Protection/run_tests.py --cc <CubeIDE arm-none-eabi-gcc.exe>
Needs unicorn and pyelftools (can be installed into an isolated --target folder).
The HAL/register mocks do not validate real pins, ADC calibration, or Fault latency.
"""
import argparse
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile

isolated = Path(tempfile.gettempdir()) / "fsbb-protection-validation" / "python"
if isolated.exists():
    sys.path.insert(0, str(isolated))
from elftools.elf.elffile import ELFFile
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_THUMB, UC_MODE_MCLASS
from unicorn.arm_const import UC_ARM_REG_SP, UC_ARM_REG_LR

ROOT = Path(__file__).resolve().parents[2]

MOCK_HEADER = r'''
#ifndef MOCK_GLOBAL_INC_H
#define MOCK_GLOBAL_INC_H
#include <stdint.h>
#include <math.h>
#include "Global_Types.h"
#define GPIOA ((void *)1)
#define GPIOB ((void *)2)
#define GPIO_PIN_0 1U
#define GPIO_PIN_11 2048U
#define GPIO_PIN_12 4096U
#define GPIO_PIN_RESET 0U
#define GPIO_PIN_SET 1U
#define DAC_CHANNEL_2 2U
#define DAC_ALIGN_12B_R 0U
#define COMP_OUTPUT_LEVEL_HIGH 1U
#define Fpwm 200000U
#define BUCK_HOLD_DUTY 0.94f
#define ADC_VIN_ZERO_COUNTS 0.0f
#define ADC_VOUT_ZERO_COUNTS 0.0f
#define ADC_IIN_ZERO_COUNTS 0.0f
#define ADC_IOUT_ZERO_COUNTS 0.0f
#define ADC_VIN_V_PER_COUNT 0.02f
#define ADC_VOUT_V_PER_COUNT 0.02f
#define ADC_IIN_A_PER_COUNT 0.01f
#define ADC_IOUT_A_PER_COUNT 0.01f
typedef struct { uint32_t id; } HRTIM_HandleTypeDef;
typedef struct { uint32_t id; } ADC_HandleTypeDef;
typedef struct { uint32_t id; } DAC_HandleTypeDef;
typedef struct { uint32_t id; } COMP_HandleTypeDef;
typedef enum { HAL_OK = 0, HAL_ERROR = 1 } HAL_StatusTypeDef;
extern uint32_t mockPrimask;
static inline uint32_t __get_PRIMASK(void) { return mockPrimask; }
static inline void __disable_irq(void) { mockPrimask = 1U; }
static inline void __set_PRIMASK(uint32_t x) { mockPrimask = x; }
extern HRTIM_HandleTypeDef hhrtim1;
extern ADC_HandleTypeDef hadc2;
extern DAC_HandleTypeDef hdac3, hdac4;
extern COMP_HandleTypeDef hcomp4, hcomp6;
extern PIDtypedef currLoop, volLoop;
extern volatile Sys_Stateypdef stateMode;
extern volatile Power_Modetypedef powerMode, previousPowerMode;
extern volatile Reg_Modetypedef regMode;
extern Paratyedef sysPara, rawPara, adcPara;
extern float previousDutyE, previousDutyF;
extern uint8_t pwmHistoryValid;
void APP_PWMDisable(void);
void APP_DualLoopInit(PIDtypedef *, PIDtypedef *);
void APP_SoftStartReset(void);
void APP_Sample(void);
void APP_SoftStart(PIDtypedef *);
float APP_SoftStartStep(float);
void APP_PowerModeUpdate(float, float);
void APP_PWMUpdate(float);
uint32_t HAL_GPIO_ReadPin(void *, uint32_t);
uint32_t HAL_COMP_GetOutputLevel(const COMP_HandleTypeDef *);
HAL_StatusTypeDef HAL_COMP_Start(COMP_HandleTypeDef *);
HAL_StatusTypeDef HAL_DAC_Start(DAC_HandleTypeDef *, uint32_t);
HAL_StatusTypeDef HAL_DAC_SetValue(DAC_HandleTypeDef *, uint32_t, uint32_t, uint32_t);
#include "Protection_Config.h"
#include "Protection.h"
#endif
'''

FIXTURE = r'''
#include "Global_Inc.h"
uint32_t mockPrimask;
HRTIM_HandleTypeDef hhrtim1 = {1U};
ADC_HandleTypeDef hadc2 = {2U};
DAC_HandleTypeDef hdac3 = {3U}, hdac4 = {4U};
COMP_HandleTypeDef hcomp4 = {4U}, hcomp6 = {6U};
PIDtypedef currLoop, volLoop;
volatile Sys_Stateypdef stateMode;
volatile Power_Modetypedef powerMode, previousPowerMode;
volatile Reg_Modetypedef regMode;
Paratyedef sysPara, rawPara, adcPara;
float previousDutyE, previousDutyF;
uint8_t pwmHistoryValid;
uint32_t keyLevel, comp4Level, comp6Level, failHal;
uint32_t dacWrites, compStarts, pwmDisables, pwmUpdates, vinDacWritten, voutDacWritten;
volatile int test_result = -1;
volatile uint32_t test_checks = 0U;
#define CHECK(x) do { test_checks++; if (!(x)) { test_result = __LINE__; return 0; } } while (0)
void APP_PWMDisable(void) { pwmDisables++; pwmHistoryValid = 0U; }
void APP_DualLoopInit(PIDtypedef *a, PIDtypedef *b) { a->Integral = b->Integral = 0.0f; }
void APP_SoftStartReset(void) {}
void APP_Sample(void) {}
void APP_SoftStart(PIDtypedef *p) { p->Target = 1.0f; }
float APP_SoftStartStep(float x) { return x; }
void APP_PowerModeUpdate(float x, float y) { (void)x; (void)y; }
void APP_PWMUpdate(float u) { (void)u; pwmUpdates++; }
uint32_t HAL_GPIO_ReadPin(void *p, uint32_t n) { (void)p; (void)n; return keyLevel; }
uint32_t HAL_COMP_GetOutputLevel(const COMP_HandleTypeDef *h) { return h->id == 4U ? comp4Level : comp6Level; }
HAL_StatusTypeDef HAL_COMP_Start(COMP_HandleTypeDef *h) { (void)h; compStarts++; return failHal ? HAL_ERROR : HAL_OK; }
HAL_StatusTypeDef HAL_DAC_Start(DAC_HandleTypeDef *h, uint32_t ch) { (void)h; (void)ch; return failHal ? HAL_ERROR : HAL_OK; }
HAL_StatusTypeDef HAL_DAC_SetValue(DAC_HandleTypeDef *h, uint32_t ch, uint32_t align, uint32_t code) {
    (void)ch; (void)align; dacWrites++;
    if (h->id == 3U) vinDacWritten = code; else voutDacWritten = code;
    return failHal ? HAL_ERROR : HAL_OK;
}
void HAL_HRTIM_Fault1Callback(HRTIM_HandleTypeDef *);
void HAL_HRTIM_Fault2Callback(HRTIM_HandleTypeDef *);
void HAL_HRTIM_Fault3Callback(HRTIM_HandleTypeDef *);
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *);

static uint8_t initHealthy(void) {
    keyLevel = GPIO_PIN_SET; comp4Level = comp6Level = failHal = 0U;
    mockPrimask = dacWrites = compStarts = pwmDisables = pwmUpdates = 0U;
    vinDacWritten = voutDacWritten = 0U;
    sysPara.OutVol = 24.0f; sysPara.OutCurr = 2.0f;
    rawPara.InputVol = rawPara.OutVol = rawPara.InputCurr = rawPara.OutCurr = 1000.0f;
    adcPara.InputVol = 24.0f; adcPara.OutVol = 24.0f;
    adcPara.InputCurr = adcPara.OutCurr = 1.0f;
    return APP_ProtectionInit();
}

int main(void) {
    test_result = -2;
#ifdef MOCK_INVALID_CONFIG
    CHECK(!initHealthy());
    CHECK(faultFlags & FAULT_CONFIG);
    CHECK(dacWrites == 0U && compStarts == 0U);
    CHECK(!APP_ProtectionOnSample() && stateMode == SYS_FAULT);
#else
    CHECK(initHealthy());
    CHECK(stateMode == SYS_IDLE && faultFlags == FAULT_NONE);
    CHECK(dacWrites == 2U && compStarts == 2U);
    CHECK(vinDacWritten == VIN_OVP_DAC_CODE && voutDacWritten == VOUT_OVP_DAC_CODE);
    CHECK(!APP_ProtectionOnSample() && stateMode == SYS_IDLE);

    /* CC at 2 A is normal; exceeding a separate 3 A OCP threshold trips. */
    stateMode = SYS_RUN; regMode = MODE_CC; adcPara.OutCurr = 2.0f;
    CHECK(APP_ProtectionOnSample() && faultFlags == FAULT_NONE);
    adcPara.OutCurr = 3.0f;
    CHECK(APP_ProtectionOnSample());
    adcPara.OutCurr = 3.01f;
    CHECK(!APP_ProtectionOnSample() && (faultFlags & FAULT_IOUT_OCP));
    CHECK(stateMode == SYS_FAULT && pwmDisables > 0U);

    /* Removing the cause does not clear the latched type or restore control. */
    adcPara.OutCurr = 1.0f;
    CHECK(!APP_ProtectionOnSample() && stateMode == SYS_FAULT);
    CHECK(faultFlags == FAULT_IOUT_OCP);
    HAL_HRTIM_Fault1Callback(&hhrtim1);
    CHECK((faultFlags & (FAULT_IOUT_OCP | FAULT_STOP_KEY)) == (FAULT_IOUT_OCP | FAULT_STOP_KEY));

    CHECK(initHealthy()); stateMode = SYS_RUN; keyLevel = GPIO_PIN_RESET;
    CHECK(!APP_ProtectionOnSample() && (faultFlags & FAULT_STOP_KEY));
    keyLevel = GPIO_PIN_SET;
    CHECK(!APP_ProtectionOnSample() && stateMode == SYS_FAULT);
    CHECK(initHealthy()); comp4Level = 1U;
    CHECK(!APP_ProtectionOnSample() && (faultFlags & FAULT_VIN_HW_OVP));
    CHECK(initHealthy()); comp6Level = 1U;
    CHECK(!APP_ProtectionOnSample() && (faultFlags & FAULT_VOUT_HW_OVP));

    CHECK(initHealthy());
    HAL_HRTIM_Fault2Callback(&hhrtim1);
    CHECK(faultFlags & FAULT_VIN_HW_OVP);
    HAL_HRTIM_Fault3Callback(&hhrtim1);
    CHECK(faultFlags & FAULT_VOUT_HW_OVP);

    CHECK(initHealthy()); stateMode = SYS_RUN; adcPara.InputVol = 9.0f;
    CHECK(!APP_ProtectionOnSample() && (faultFlags & FAULT_VIN_UVP));
    CHECK(initHealthy()); adcPara.InputVol = 9.0f;
    CHECK(!APP_ProtectionOnSample() && faultFlags == FAULT_NONE);
    CHECK(initHealthy()); adcPara.InputVol = 30.1f;
    CHECK(!APP_ProtectionOnSample() && (faultFlags & FAULT_VIN_OVP));
    CHECK(initHealthy()); adcPara.OutVol = 26.1f;
    CHECK(!APP_ProtectionOnSample() && (faultFlags & FAULT_VOUT_OVP));
    CHECK(initHealthy()); adcPara.InputCurr = -4.1f;
    CHECK(!APP_ProtectionOnSample() && (faultFlags & FAULT_IIN_OCP));

    CHECK(initHealthy()); stateMode = SYS_RUN;
    adcPara.InputVol = 31.0f; adcPara.OutVol = 27.0f;
    adcPara.InputCurr = 5.0f; adcPara.OutCurr = 4.0f;
    CHECK(!APP_ProtectionOnSample());
    CHECK((faultFlags & (FAULT_VIN_OVP | FAULT_VOUT_OVP | FAULT_IIN_OCP | FAULT_IOUT_OCP)) ==
          (FAULT_VIN_OVP | FAULT_VOUT_OVP | FAULT_IIN_OCP | FAULT_IOUT_OCP));

    CHECK(initHealthy()); stateMode = SYS_RUN; adcPara.OutVol = NAN;
    HAL_ADCEx_InjectedConvCpltCallback(&hadc2);
    CHECK((faultFlags & FAULT_ADC_DATA) && pwmUpdates == 0U);
    CHECK(initHealthy()); rawPara.InputCurr = 4095.0f;
    CHECK(!APP_ProtectionOnSample() && (faultFlags & FAULT_ADC_DATA));

    CHECK(initHealthy()); failHal = 1U;
    CHECK(!APP_ProtectionInit() && (faultFlags & FAULT_HW_INIT));
    mockPrimask = 1U; APP_FaultTrip(FAULT_CONTROL); CHECK(mockPrimask == 1U);
#endif
    test_result = 0;
    return 0;
}
'''

VALID = {'PROTECTION_CONFIG_READY': '1U', 'VOUT_SETTING_MIN_V': '3.0f', 'VOUT_SETTING_MAX_V': '24.0f', 'IOUT_SETTING_MAX_A': '2.0f', 'VIN_UVP_TRIP_V': '10.0f', 'VIN_OVP_TRIP_V': '30.0f', 'VOUT_OVP_TRIP_V': '26.0f', 'IIN_OCP_TRIP_A': '4.0f', 'IOUT_OCP_TRIP_A': '3.0f', 'VIN_OVP_DAC_CODE': '2048U', 'VOUT_OVP_DAC_CODE': '3072U'}

LINKER = '''ENTRY(main)
MEMORY { FLASH (rx) : ORIGIN = 0x10000, LENGTH = 512K
         RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K }
SECTIONS {
 .text : { *(.text*) *(.rodata*) *(.ARM.extab*) } > FLASH
 .ARM.exidx : { *(.ARM.exidx*) } > FLASH
 .data : { *(.data*) } > RAM AT> FLASH
 .bss (NOLOAD) : { *(.bss*) *(COMMON) } > RAM
}
'''

def run_case(cc, directory, name, replacements, invalid=False):
    directory.mkdir()
    config = (ROOT / "Utils/Inc/Protection_Config.h").read_text(encoding="utf-8")
    for key, value in replacements.items():
        config, count = re.subn(r"(?m)^(#define\s+" + re.escape(key) + r"\s+)[^\r\n]+",
                                lambda m: m[1] + value, config)
        assert count == 1, key
    (directory / "Protection_Config.h").write_text(config, encoding="utf-8")
    (directory / "Global_Inc.h").write_text(MOCK_HEADER, encoding="utf-8")
    (directory / "fixture.c").write_text(FIXTURE, encoding="utf-8")
    (directory / "memory.ld").write_text(LINKER, encoding="utf-8")
    elf = directory / "test.elf"
    command = [cc, "-mcpu=cortex-m4", "-mthumb", "-mfloat-abi=soft", "-std=gnu11", "-O1",
               "-g", "-Wall", "-Wextra", "-Werror", "-ffunction-sections", "-fdata-sections",
               "-I" + str(directory), "-I" + str(ROOT / "Utils/Inc"),
               str(ROOT / "Utils/Src/Protection.c"), str(ROOT / "Utils/Src/Callback.c"),
               str(directory / "fixture.c"), "-nostartfiles", "--specs=nosys.specs",
               "-Wl,--gc-sections", "-T" + str(directory / "memory.ld"), "-lm", "-o", str(elf)]
    if invalid:
        command.insert(1, "-DMOCK_INVALID_CONFIG")
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode:
        raise RuntimeError(result.stdout + result.stderr)
    with elf.open("rb") as stream:
        parsed = ELFFile(stream)
        symbols = {s.name: s["st_value"] for s in parsed.get_section_by_name(".symtab").iter_symbols()}
        emulator = Uc(UC_ARCH_ARM, UC_MODE_THUMB | UC_MODE_MCLASS)
        emulator.mem_map(0x10000, 0x100000)
        emulator.mem_map(0x20000000, 0x20000)
        for segment in parsed.iter_segments():
            if segment["p_type"] == "PT_LOAD" and segment["p_filesz"]:
                emulator.mem_write(segment["p_vaddr"], segment.data())
        stop = 0x100000
        emulator.reg_write(UC_ARM_REG_SP, 0x2001fff0)
        emulator.reg_write(UC_ARM_REG_LR, stop | 1)
        emulator.emu_start(symbols["main"] | 1, stop, count=100000000)
        outcome = struct.unpack("<i", emulator.mem_read(symbols["test_result"], 4))[0]
        checks = struct.unpack("<I", emulator.mem_read(symbols["test_checks"], 4))[0]
        if outcome:
            raise AssertionError(f"{name}: fixture failure at line {outcome}, after {checks} assertions")
    print(f"PASS {name}: {checks} assertions")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cc", default=os.environ.get("ARM_GCC", "arm-none-eabi-gcc"))
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="fsbb-protection-tests-") as temp:
        folder = Path(temp)
        run_case(args.cc, folder / "runtime", "runtime_fault_latching", VALID)
        run_case(args.cc, folder / "unconfigured", "unconfigured_blocks_output", {}, True)
        run_case(args.cc, folder / "bad-dac", "out_of_range_dac_code_is_rejected",
                 dict(VALID, VIN_OVP_DAC_CODE="4096U"), True)
