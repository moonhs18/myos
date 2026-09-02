.section ".text"
.global arm64_mmu_enable

// void arm64_mmu_enable(uint64_t l0_table_pa);
// x0 is the physical addr of the L0 table
arm64_mmu_enable:
    //set MAIR_EL1
    //Attr 0 (bits[7:0]) = 0x00(device-nGnRnE)
    //Attr 1 (bits[15:8]) = 0xFF(normal cachable)
    ldr x1, =0x000000000000FF00
    msr mair_el1, x1

    //set TCR_EL1
    // T0SZ = 16 (48-bit Virtual Address, Bits[5:0])
    // IRGN0 = 0b01 (Inner WB-WA, Bits[9:8])
    // ORGN0 = 0b01 (Outer WB-WA, Bits[11:10])
    // SH0 = 0b11 (Inner Shareable, Bits[13:12])
    // TG0 = 0b00 (4KB Granule, Bits[15:14])
    // EPD1 = 1 (Disable TTBR1 walks for now, Bit 23)
    // IPS = 0b010 (40-bit Physical Address space 1TB, Bits[34:32])
    ldr x2, =   ((16ULL << 0) | (1ULL << 8)  | (1ULL << 10) | (3ULL << 12) | (0ULL << 14) | (1ULL << 23) | (2ULL << 32))
    msr tcr_el1, x2

    //set L0 root table addr in TTBR0_EL1 
    msr ttbr0_el1, x0

    //invalid all tlb entries
    dsb ish
    tlbi vmalle1
    dsb ish 
    isb

    //modify SCTLR_EL1 to enable mmu, data cache, instruction cache
    mrs x3, sctlr_el1
    orr x3, x3, #(1 << 0)   //Bit 0 : MMU enable
    orr x3, x3, #(1 << 2)   //Bit 2 : Data cache enable
    orr x3, x3, #(1 << 12)  //Bit 12: Instruction cache enable
    msr sctlr_el1, x3
    isb

    ret
    


