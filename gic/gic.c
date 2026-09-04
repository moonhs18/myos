#include "gic.h"


//32bits MMIO memory access helper
static inline void mmio_write32(uint64_t addr, uint32_t val){
    *(volatile uint32_t*)addr = val;
}

static inline uint32_t mmio_read32(uint64_t addr){
    return *(volatile uint32_t*)addr;
}

//8bits MMIO memory acceess helper to set priority
static inline void mmio_write8(uint64_t addr, uint8_t val){
    *(volatile uint8_t *)addr = val;
}

void gic_init(void){
    //GICD disable for setting
    mmio_write32(GICD_BASE + GICD_CTLR, 0);

    //disable all of the Interrupt
    for(int i=0;i<4;i++){
        mmio_write32(GICD_BASE + GICD_ICENABLER + (i * 4), 0xFFFFFFFF);
    }

    for(int i=0;i<4;i++){
        mmio_write32(GICD_BASE + GICD_IGROUPR + (i * 4), 0xFFFFFFFF);
    }

    //enable distributor
    mmio_write32(GICD_BASE + GICD_CTLR, 1);

    //cpu interface priority
    mmio_write32(GICC_BASE + GICC_PMR, 0xFF);

    //cpu interface enable
    mmio_write32(GICC_BASE + GICC_CTLR, 1);
}

void gic_enable_interrupt(uint32_t irq_id){
    //Set prority
    //0x80 => middle priority
    mmio_write8(GICD_BASE + GICD_IPRIORITYR + irq_id, 0x80);
    if(irq_id >=32){
        mmio_write8(GICD_BASE + GICD_ITARGETSR + irq_id, 0x01);
    }

    //IRQ enable
    uint32_t reg_idx = irq_id / 32;
    uint32_t bit_offset = irq_id % 32;
    mmio_write32(GICD_BASE + GICD_ISENABLER + (reg_idx * 4), (1U << bit_offset));
}

//If interrupt occured > IRQ Num check
uint32_t gic_acknowledge_irq(void){
    return mmio_read32(GICC_BASE + GICC_IAR) & 0x3FF;
}

//End of Interrupt
void gic_end_of_irq(uint32_t irq_id){
    mmio_write32(GICC_BASE + GICC_EOIR, irq_id);
}



