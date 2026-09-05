#include "timer.h"
#include "gic.h"
#include "uart.h"

static volatile uint64_t timer_ticks = 0;
static uint64_t timer_interval = 0;

static inline uint64_t read_cntfrq(void){
    uint64_t val;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(val));
    return val;
}
static inline void write_cntpfrq(uint64_t val){
    asm volatile("msr cntfrq_el0, %0" : : "r"(val));
}

static inline void write_cntp_tval(uint64_t val){
    asm volatile("msr cntp_tval_el0, %0" : : "r"(val) : "memory");
}

static inline uint64_t read_cntp_tval(void){
    uint64_t val;
    asm volatile("mrs %0, cntp_tval_el0" : "=r"(val));
    return val;
}

static inline void write_cntp_ctl(uint64_t val){
    asm volatile("msr cntp_ctl_el0, %0" : : "r"(val) : "memory");
}

static inline uint64_t read_cntp_ctl(void){
    uint64_t val;
    asm volatile("mrs %0, cntp_ctl_el0" : "=r"(val));
    return val;
}

void timer_init(uint32_t target_hz){
    if(target_hz == 0) target_hz = 100; //default = 100Hz(10ms)
    
    //Read the CPU physical timer frequency
    uint64_t freq = read_cntfrq(); 

    if (freq == 0){
        freq = 62500000ULL;
        write_cntpfrq(freq);
    }
    //Calculate the countdown value for a single interrupt
    timer_interval = freq / target_hz;

    //In GICv2 enable Num 30 Timer Interrupt
    gic_enable_interrupt(IRQ_TIMER_PHYS_NS);

    write_cntp_tval(timer_interval);
    write_cntp_ctl(1);
}

void timer_handle_irq(void){
    //if you wanna test it, uncomment the uart code below
    //uart_puts("[TIMER IRQ] ENTER\n");
    //uart_puts("[TIMER IRQ] ticks = ");
    //uart_put_hex(timer_ticks);
    //uart_puts("\n");

    timer_ticks++;
    //Reload the countdown timer for the next period
    write_cntp_tval(timer_interval);
}

uint64_t timer_get_ticks(void){
    return timer_ticks;
}

void timer_sleep_ticks(uint64_t count){
    uint64_t start = timer_ticks;
    while(timer_ticks - start < count){
        asm volatile ("wfi"); // Wait for an interrupt 
    }
}




