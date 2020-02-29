#include "emmc.h"

#include "gpio.h"
#include "uart0.h"
#include "timer.h"

static constexpr auto EMMC_CD = 47;
static constexpr auto EMMC_CLK = 48;
static constexpr auto EMMC_CMD = 49;
static constexpr auto EMMC_DATA0 = 50;
static constexpr auto EMMC_DATA1 = 51;
static constexpr auto EMMC_DATA2 = 52;
static constexpr auto EMMC_DATA3 = 53;

static constexpr auto FREQUENCY_SETUP = 400000; // 400 Khz
static constexpr auto FREQUENCY_NORMAL = 25000000; // 25 Mhz

#define EMMC_ARG2 ((volatile uint32_t*)(EMMC_BASE + 0x00))
#define EMMC_BLKSIZECNT ((volatile uint32_t*)(EMMC_BASE + 0x04))
#define EMMC_ARG1 ((volatile uint32_t*)(EMMC_BASE + 0x08))
#define EMMC_CMDTM ((volatile uint32_t*)(EMMC_BASE + 0x0C))
#define EMMC_RESP0 ((volatile uint32_t*)(EMMC_BASE + 0x10))
#define EMMC_RESP1 ((volatile uint32_t*)(EMMC_BASE + 0x14))
#define EMMC_RESP2 ((volatile uint32_t*)(EMMC_BASE + 0x18))
#define EMMC_RESP3 ((volatile uint32_t*)(EMMC_BASE + 0x1C))
#define EMMC_DATA ((volatile uint32_t*)(EMMC_BASE + 0x20))
#define EMMC_STATUS ((volatile uint32_t*)(EMMC_BASE + 0x24))
#define EMMC_CONTROL0 ((volatile uint32_t*)(EMMC_BASE + 0x28))
#define EMMC_CONTROL1 ((volatile uint32_t*)(EMMC_BASE + 0x2C))
#define EMMC_INTERRUPT ((volatile uint32_t*)(EMMC_BASE + 0x30))
#define EMMC_IRPT_MASK ((volatile uint32_t*)(EMMC_BASE + 0x34))
#define EMMC_ IRPT_EN ((volatile uint32_t*)(EMMC_BASE + 0x38))
#define EMMC_CONTROL2 ((volatile uint32_t*)(EMMC_BASE + 0x3C))
#define EMMC_FORCE_IRPT ((volatile uint32_t*)(EMMC_BASE + 0x50))
#define EMMC_BOOT_TIMEOUT ((volatile uint32_t*)(EMMC_BASE + 0x70))
#define EMMC_DBG_SEL ((volatile uint32_t*)(EMMC_BASE + 0x74))
#define EMMC_EXRDFIFO_CFG ((volatile uint32_t*)(EMMC_BASE + 0x80))
#define EMMC_EXRDFIFO_EN ((volatile uint32_t*)(EMMC_BASE + 0x84))
#define EMMC_TUNE_STEP ((volatile uint32_t*)(EMMC_BASE + 0x88))
#define EMMC_TUNE_STEPS_STD  ((volatile uint32_t*)(EMMC_BASE + 0x8C))
#define EMMC_TUNE_STEPS_DDR  ((volatile uint32_t*)(EMMC_BASE + 0x90))
#define EMMC_SPI_INT_SPT  ((volatile uint32_t*)(EMMC_BASE + 0xF0))
#define EMMC_SLOTISR_VER  ((volatile uint32_t*)(EMMC_BASE + 0xFC))

static uint32_t specificationVersion;

static void initialiseGpio() {
    setGpioPinMode(EMMC_CD, GpioMode::Input);
    setGpioPinUpDown(0, (1u << 15u), GpioPinupMode::PullUp);
    *GPHEN1 |= (1u << 15u);

    setGpioPinMode(EMMC_CLK, GpioMode::Function3);
    setGpioPinMode(EMMC_CMD, GpioMode::Function3);
    setGpioPinUpDown(0, (1u << 16u) | (1u << 17u), GpioPinupMode::PullUp);

    setGpioPinMode(EMMC_DATA0, GpioMode::Function3);
    setGpioPinMode(EMMC_DATA1, GpioMode::Function3);
    setGpioPinMode(EMMC_DATA2, GpioMode::Function3);
    setGpioPinMode(EMMC_DATA3, GpioMode::Function3);
    setGpioPinUpDown(0, (1u << 18u) | (1u << 19u) | (1u << 20u) | (1u << 21u), GpioPinupMode::PullUp);
}

//static void setClock(uint32_t frequency) {
//
//    // Wait for any pending inhibit bits
//    int count = 100000;
//    while( (*EMMC_STATUS & (SR_CMD_INHIBIT|SR_DAT_INHIBIT)) && --count )
//        waitMicro(1);
//    if( count <= 0 )
//    {
//        LOG_ERROR("EMMC: Set clock: timeout waiting for inhibit flags. Status %08x.\n",*EMMC_STATUS);
//        return SD_ERROR_CLOCK;
//    }
//
//    // Switch clock off.
//    *EMMC_CONTROL1 &= ~C1_CLK_EN;
//    waitMicro(10);
//
//    // Request the new clock setting and enable the clock
//    int cdiv = sdGetClockDivider(freq);
//    *EMMC_CONTROL1 = (*EMMC_CONTROL1 & 0xffff003f) | cdiv;
//    waitMicro(10);
//
//    // Enable the clock.
//    *EMMC_CONTROL1 |= C1_CLK_EN;
//    waitMicro(10);
//
//    // Wait for clock to be stable.
//    count = 10000;
//    while( !(*EMMC_CONTROL1 & C1_CLK_STABLE) && count-- )
//        waitMicro(10);
//    if( count <= 0 )
//    {
//        LOG_ERROR("EMMC: ERROR: failed to get stable clock.\n");
//        return SD_ERROR_CLOCK;
//    }
//
//    //  printf("EMMC: Set clock, status %08x CONTROL1: %08x\n",*EMMC_STATUS,*EMMC_CONTROL1);
//
//    return SD_OK;
//}

void Emmc::initialise() {
    initialiseGpio();

    // TODO: detect card removed/inserted
    if (isCardPresent()) {
        Uart0::write("Card present!\n");
    }

    specificationVersion = (*EMMC_SLOTISR_VER >> 16u) & 0xFFu;

    reset();
}

bool Emmc::isCardPresent() {
    return true;
    // TODO
//    return !getGpioPin(EMMC_CD);
}

void Emmc::reset() {
//    *EMMC_CONTROL0 = 0;
//    *EMMC_CONTROL1 |= (1u << 24u);
//    delay(10);
//
//    auto count = 10000;
//    do {
//        waitUsec(10);
//    } while ((*EMMC_CONTROL1 & (1u << 24u)) && count--);
//
//    if(count <= 0) {
//        Uart0::write("ERROR: failed to reset EMMC\n");
//        return;
//    }
//
//    // Enable internal clock and set data timeout.
//    *EMMC_CONTROL1 |= (1u << 0u) | (0xFu << 16u);
//    waitUsec(10);
//
//    // Set clock to setup frequency.
//    setClock(FREQUENCY_SETUP);
//
//    // Enable interrupts for command completion values.
//    //*EMMC_IRPT_EN   = INT_ALL_MASK;
//    //*EMMC_IRPT_MASK = INT_ALL_MASK;
//    *EMMC_IRPT_EN   = 0xffffffff;
//    *EMMC_IRPT_MASK = 0xffffffff;
//    //  printf("EMMC: Interrupt enable/mask registers: %08x %08x\n",*EMMC_IRPT_EN,*EMMC_IRPT_MASK);
//    //  printf("EMMC: Status: %08x, control: %08x %08x %08x\n",*EMMC_STATUS,*EMMC_CONTROL0,*EMMC_CONTROL1,*EMMC_CONTROL2);
//
//    // Reset card registers.
//    sdCard.rca = 0;
//    sdCard.ocr = 0;
//    sdCard.lastArg = 0;
//    sdCard.lastCmd = 0;
//    sdCard.status = 0;
//    sdCard.type = 0;
//    sdCard.uhsi = 0;
//
//    // Send GO_IDLE_STATE
//    //  printf("---- Send IX_GO_IDLE_STATE command\n");
//    resp = sdSendCommand(IX_GO_IDLE_STATE);
//
//    return resp;
}
