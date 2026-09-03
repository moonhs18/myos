#ifndef GIC_H
#define GIC_H

#include <stdint.h>

#define GICD_BASE 0x08000000ULL //Distributor base addr
#define GICC_BASE 0x08010000ULL //CPU interface base addr

// GICD(Distributor) register offset
#define GICD_CTLR       0x000   //Distributor Control
#define GICD_TYPER      0x004   //Controller Type
#define GICD_ISENABLER  0x100   //Interrupt Set-enable
#define GICD_ICENABLER  0x180   //Interrupt Clear-enable
#define GICD_IPRIORITYR 0x400   //Interrupt Priority
#define GICD_ITARGETSR  0x800   //Interrupt Targets

// GICC(CPU Interface) register offset
#define GICC_CTLR       0x0000  //CPU Interface Control
#define GICC_PMR        0x0004  //Interrupt Priority Mask
#define GICC_IAR        0x000C  //Interrupt Acknowledge
#define GICC_EOIR       0x0010  //End of Interrupt

//SYS Interrupt Num
#define IRQ_TIMER_PHYS_NS   30  //ARM Generic Timer Non-Secure Physical IRQ(PPI 14)

void gic_init(void);
void gic_enable_interrupt(uint32_t irq_id);
uint32_t gic_acknowledge_irq(void);
void gic_end_if_irq(uint32_t irq_id);


#endif