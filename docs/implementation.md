[Phase 1: Boot & I/O] ──► [Phase 2: Exception] ──► [Phase 3: Memory (MMU)]
(부트/스택/UART) [완료]    (벡터테이블/패닉덤프)      (PMM / kmalloc / 페이징)
                                                              │
                                                              ▼
[Phase 6: User Mode]  ◄── [Phase 5: Scheduling] ◄── [Phase 4: Interrupt]
(EL0 / SVC 시스템콜)       (PCB / 선점형 스케줄러)     (GIC / ARM Generic Timer)
        │
        ▼
[Phase 7: Storage & VFS] ──► [Phase 8: Shell & Apps]
(RamFS / Initramfs / VFS)     (ELF 로더 / Mini C Lib / 쉘)