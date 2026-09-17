# MiniOS

ARM64 기반으로 구현한 교육 목적의 Mini Operating System 프로젝트입니다

본 문서는 MiniOS를 구현하는 과정에서 발생한 주요 결함을 문제 상황 → 디버깅 및 추적 → 근본 원인 → 해결 방법 → 고찰의 흐름으로 기록하기 위한 문서입니다.

---

# 결함 및 디버깅 기록

## 1. GCC 자동 벡터화(-O2)와 CPU FPU/SIMD 하드웨어 트랩 충돌

### 문제 상황

동적 메모리 할당자 `kmalloc` 구현 후 정상적인 RAM 주소를 할당받았음에도, 메모리에 값을 반복적으로 쓰는 순간 커널 패닉이 발생했다.

처음에는 잘못된 포인터나 메모리 정렬 문제에 의한 `Data Abort`를 의심했다.

### 디버깅 및 추적

트랩 프레임을 분석한 결과:

* 접근 주소는 정상적인 RAM 영역
* 포인터 정렬 역시 정상
* `FAR_EL1`이 `0x0`

이라는 특이점을 확인했다.

일반적인 메모리 접근 오류라면 Fault Address가 `FAR_EL1`에 기록되어야 한다. 따라서 실제 메모리 주소 문제가 아니라 CPU가 특정 명령어 실행을 Trap한 상황으로 범위를 좁혔다.

`ESR_EL1`의 Exception Class(EC)를 분석한 결과:

```text
EC = 0x07
```

이었으며, 이는 Advanced SIMD/Floating Point 접근 Trap임을 확인했다.

### 근본 원인

GCC의 `-O2` 최적화 과정에서 단순한 메모리 쓰기 루프가 ARM64 NEON/SIMD 명령어로 자동 벡터화되었다.

그러나 초기 부팅 상태에서는 CPU의 FPU/SIMD 사용이 허용되지 않아 해당 명령어가 실행될 때 예외가 발생했다.

즉,

```text
C 코드
  ↓
GCC -O2
  ↓
NEON/SIMD 명령어로 자동 벡터화
  ↓
FPU/SIMD 접근 Trap
  ↓
Kernel Exception
```

의 흐름이었다.

### 해결

EL1 초기화 과정에서 `CPACR_EL1`의 `FPEN` 비트를 `0b11`로 설정하여 FPU/SIMD 접근을 허용하고 `ISB`를 수행했다.

추가적으로 커널 빌드 옵션에:

```text
-mgeneral-regs-only
```

를 적용하여 커널 코드가 불필요한 SIMD/FPU 명령어를 생성하지 않도록 제한했다.

### 고찰

최적화는 일반적인 애플리케이션에서는 성능 향상에 도움이 되지만, OS 커널에서는 **컴파일러가 생성하는 명령어까지 하드웨어 초기화 상태와 연결되어 있다는 점**을 확인했다.

특히 인터럽트와 Context Switch를 직접 구현하는 환경에서는 SIMD 레지스터 상태까지 관리해야 하므로, 초기 커널에서는 General Purpose Register 중심으로 실행 범위를 제한하는 것이 안정적이었다.

---

## 2. 힙 관리자 베이스 주소 오염으로 인한 Data Abort

### 문제 상황

힙 초기화 직후 첫 번째 `kmalloc()`은 정상적으로 수행되는 것처럼 보였지만, 반환된 포인터를 역참조하여 데이터를 쓰는 순간 Data Abort가 발생했다.

### 디버깅 및 추적

커널 패닉 덤프에서 메모리 접근에 사용된 레지스터 값이:

```text
0x0000000000000020
```

으로 나타났다.

이 값이 힙 블록 메타데이터 구조체인 `block_header_t`의 크기와 정확히 일치한다는 점에 주목하여 `heap_init()`과 `kmalloc()`의 포인터 연산을 역추적했다.

### 근본 원인

`heap_init()`에서 힙의 시작 주소인 `heap_start`에 실제 PMM 할당 주소를 저장해야 했지만, 실수로 **힙 전체 크기를 계산한 값을 포인터 변수에 대입**했다.

그 결과 힙의 시작 주소가 잘못 설정되었고, `kmalloc()`은 잘못된 베이스 주소에 `HEADER_SIZE(0x20)`를 더하여 다음과 같은 주소를 반환하게 되었다.

```text
잘못된 heap_start
      ↓
kmalloc()
      ↓
heap_start + HEADER_SIZE
      ↓
0x20
      ↓
잘못된 주소 역참조
      ↓
Data Abort
```

### 해결

힙 시작 주소를 PMM이 반환한 실제 메모리 주소로 정확하게 초기화했다.

```c
heap_start = (block_header_t *)initial_mem;
```

또한 메모리 크기 계산을 위한 정수 연산과 실제 메모리를 가리키는 포인터 연산을 명확하게 분리했다.

### 고찰

이번 문제를 통해 **주소값과 크기값은 모두 64비트 정수로 표현될 수 있지만 의미는 완전히 다르다**는 점을 확인했다.

특히 커널 메모리 관리에서는 작은 포인터 연산 실수도 즉시 낮은 주소 영역의 Data Abort로 이어질 수 있기 때문에, 주소와 크기를 명확하게 구분하는 것이 중요하다.

---

## 3. 페이지 디스크립터 주소 마스킹 및 페이지 테이블 초기화 오류

### 문제 상황

MMU를 활성화한 후 가상 주소를 물리 주소로 변환하는 `mmu_translate()` 테스트에서 정상적인 물리 주소 대신:

```text
0x707
0x403
```

등의 비정상적인 값이 반환되었다.

### 디버깅 및 추적

반환값의 비트 패턴을 분석한 결과 `0x707`은 실제 물리 주소가 아니라 MMU Mapping Attribute에 사용되는 하위 비트와 일치했다.

따라서 L3 Page Descriptor에서 물리 주소를 추출하는 비트 마스킹 로직을 집중적으로 확인했다.

### 근본 원인

페이지 디스크립터에서 물리 주소를 추출하는 과정에서 비트 반전 연산을 잘못 사용하여 **실제 물리 주소 영역을 제거하고 속성 비트만 남기는 문제**가 있었다.

추가적으로 UART와 같은 Device Mapping에서는 주소에 Execute Never 등의 속성 비트가 섞여 들어갈 가능성도 있었다.

또 다른 문제는 새 Page Table을 PMM에서 할당한 후 메모리를 0으로 초기화하지 않았다는 것이었다.

Page Table에는 사용하지 않는 엔트리가 반드시 `0`이어야 하는데, 이전 메모리의 쓰레기 값이 남아 있을 경우 하드웨어가 이를 유효한 Descriptor로 해석할 가능성이 있었다.

### 해결

물리 주소 추출 시 ARMv8-A 주소 비트 범위에 맞춰 명확한 주소 마스크를 사용했다.

```c
0x0000FFFFFFFFF000ULL
```

또한 새로운 Page Table을 할당할 때 4KB 전체를 0으로 초기화하도록 수정했다.

```text
Page Table 할당
      ↓
4KB 전체 Zero Initialize
      ↓
필요한 Entry만 설정
```

### 고찰

페이지 테이블은 단순한 배열이 아니라 **CPU의 MMU가 직접 해석하는 하드웨어 자료구조**다.

따라서 일반적인 자료구조와 달리 초기화되지 않은 메모리나 잘못된 비트 하나도 실제 주소 변환 오류로 이어질 수 있다는 점을 확인했다.

---

## 4. 동적 재매핑 시 TLB 무효화 및 Pipeline Synchronization 누락

### 문제 상황

동일한 가상 주소를 다른 물리 페이지로 변경하는 Dynamic Remapping 테스트에서 Page Table Entry는 새로운 물리 주소로 정상적으로 변경되었지만, 실제 메모리 접근에서는 이전 물리 페이지의 데이터가 계속 읽혔다.

### 디버깅 및 추적

Page Table Entry를 직접 확인한 결과:

```text
Page Table → 새로운 Physical Address
```

로 정상적으로 변경되어 있었다.

따라서 Page Table 자체의 문제가 아니라 CPU가 **이전 주소 변환 결과를 계속 사용하고 있는 것**으로 판단했다.

### 근본 원인

ARMv8-A CPU는 성능 향상을 위해 Virtual Address → Physical Address 변환 결과를 TLB에 캐싱한다.

따라서 Page Table Entry를 변경하더라도 기존 TLB Entry가 남아 있으면 CPU는 새로운 Page Table을 다시 읽지 않고 이전 주소 변환 결과를 사용할 수 있다.

또한 ARM64의 Weakly-Ordered Memory Model에서는 메모리 접근 순서를 명확하게 보장하기 위해 적절한 Barrier가 필요하다.

### 해결

Dynamic Mapping 변경 후 다음과 같은 TLB Flush Sequence를 구현했다.

```text
Page Table 수정
      ↓
DSB ISH
      ↓
TLBI VMALLE1
      ↓
DSB ISH
      ↓
ISB
```

이를 `mmu_tlb_flush()`로 추상화하여 Mapping 변경 이후 TLB와 Pipeline이 새로운 Page Table 상태를 사용하도록 보장했다.

### 고찰

이번 문제를 통해 **메모리의 값이 변경되었다는 것과 CPU가 그 변경을 즉시 관찰한다는 것은 별개의 문제**라는 점을 확인했다.

MMU를 직접 구현할 경우 Page Table 수정뿐만 아니라 TLB와 CPU Pipeline까지 함께 고려해야 한다.

---

## 5. GICv2 Group 설정 오류로 인한 Interrupt Storm

### 문제 상황

ARM Generic Timer와 GICv2를 연결하고 CPU Interrupt를 활성화한 뒤 Timer Interrupt가 발생하기 시작했지만, Interrupt Handler에서 빠져나오지 못하고 계속해서 Interrupt Vector로 재진입하는 문제가 발생했다.

### 디버깅 및 추적

Timer 주기를 변경해도 증상이 동일했다.

또한:

```text
Interrupt 발생
    ↓
Handler 실행
    ↓
EOI
    ↓
eret
    ↓
즉시 다시 Interrupt
```

가 반복되는 것을 확인했다.

따라서 Timer 주기 문제가 아니라 **GIC가 Interrupt를 정상적으로 Deactivate하지 못하고 있는 문제**로 범위를 좁혔다.

### 근본 원인

QEMU `virt` 환경에서 사용 중인 GICv2 설정과 기존 초기화 코드의 Interrupt Group 설정이 일치하지 않았다.

기존 코드에서는:

```text
GICD_IGROUPR = 0xFFFFFFFF
```

를 사용하여 모든 Interrupt를 Group 1로 설정하고 있었다.

이로 인해 현재 실행 환경에서 기대하는 Interrupt 처리 방식과 GIC의 Group 설정이 충돌했고, EOI 이후에도 Interrupt가 계속 재발생하는 문제가 발생했다.

또한 Interrupt Handler에서 UART 출력과 같은 무거운 작업을 수행하면서 비동기 Interrupt와 메인 코드의 UART 접근이 충돌할 가능성도 존재했다.

### 해결

GIC 초기화 시 Interrupt를 Group 0으로 명시적으로 설정했다.

```text
GICD_IGROUPR = 0x00000000
```

또한 ISR에서는 불필요한 UART 출력을 제거하고 Timer Tick 증가와 같은 최소한의 작업만 수행하도록 단순화했다.

```text
Timer IRQ
   ↓
Timer Tick 증가
   ↓
Timer 재설정
   ↓
EOI
   ↓
Return
```

### 고찰

Interrupt Handler는 일반적인 함수와 달리 **언제든 비동기적으로 실행될 수 있는 특수한 실행 영역**이다.

따라서 ISR에서는 가능한 한 짧고 단순한 작업만 수행하고, 실제 처리 작업은 일반 실행 흐름이나 Scheduler에 넘기는 구조가 안정적이라는 점을 확인했다.

---

# 6. Scheduler Context Switch 시 IRQ Mask 상태 상속 문제

### 문제 상황

Preemptive Scheduler에서 Task를 여러 개 생성한 후 Timer Interrupt를 이용해 Context Switch를 수행했지만, 특정 Task가 한 번 실행된 이후 다음 Task로 정상적으로 전환되지 않는 문제가 발생했다.

### 디버깅 및 추적

Scheduler의 Task 선택 로직과 Context 구조를 확인한 결과 Task 자체의 상태와 Stack은 정상적으로 생성되고 있었다.

문제는 **새 Task가 실행되는 시점의 CPU IRQ Mask 상태**였다.

특히 Timer Interrupt Handler 내부에서 `schedule()`이 호출되고 Context Switch가 발생하면서, 새 Task가 Interrupt가 Mask된 실행 상태를 이어받을 수 있었다.

이 경우 새 Task가 실행되더라도 Timer Interrupt가 다시 발생하지 않아 Preemptive Scheduling이 멈추게 된다.

### 근본 원인

Context Switch는 General Purpose Register와 Stack Pointer를 저장/복원하지만, 새 Task의 최초 실행 시점에서 CPU의 IRQ Mask 상태를 명확하게 보장하지 않았다.

즉,

```text
Timer IRQ Handler
      ↓
schedule()
      ↓
Context Switch
      ↓
새 Task 실행
      ↓
IRQ Mask 상태 유지
      ↓
Timer IRQ 재발생 불가
      ↓
Scheduler 정지
```

와 같은 문제가 발생했다.

### 해결

새 Task가 최초 실행될 때 사용하는 `task_entry_trampoline`에서 IRQ Mask를 명시적으로 해제했다.

```asm
task_entry_trampoline:
    msr daifclr, #2
    isb
    blr x19
    bl task_exit
```

이를 통해 새로운 Task가 실행되기 시작하는 시점에 Timer Interrupt가 정상적으로 다시 활성화되도록 했다.

### 결과

수정 이후:

```text
Task B
  ↓
Task A
  ↓
Task B
  ↓
Task A
```

형태의 Preemptive Context Switch가 정상적으로 반복되는 것을 확인했다.

---

# 7. User Task 종료 후 Scheduler가 호출되지 않는 문제

### 문제 상황

EL0 User Task와 System Call 구현 이후 User Task 1이 `sys_exit()`까지 정상적으로 실행되었지만, Task가 종료된 이후 다음 User Task가 실행되지 않고 시스템이 멈추는 문제가 발생했다.

당시 실행 흐름은 다음과 같았다.

```text
User Task 1
    ↓
sys_exit()
    ↓
task_exit()
    ↓
TASK_TERMINATED
    ↓
무한 대기
```

### 디버깅 및 추적

`task_exit()`을 확인한 결과 Task의 상태를 `TASK_TERMINATED`로 변경한 후 IRQ를 Disable하고 `wfe`를 통해 무한 대기하도록 구현되어 있었다.

문제는 **Task가 종료된 이후 CPU를 다른 실행 가능한 Task에게 넘기는 코드가 없었다는 것**이었다.

Scheduler의 `schedule()`은 현재 Task 다음부터 `READY` 또는 `RUNNING` 상태의 Task를 탐색하도록 구현되어 있다.

따라서 종료 직후 Scheduler를 호출하면 현재 Task를 자연스럽게 실행 대상에서 제외하고 다음 Task를 선택할 수 있었다.

### 해결

`task_exit()`의 실행 흐름을 다음과 같이 변경했다.

```text
현재 Task
    ↓
TASK_TERMINATED
    ↓
schedule()
    ↓
다음 READY/RUNNING Task 탐색
    ↓
Context Switch
    ↓
다음 Task 실행
```

핵심은 **`schedule()`을 호출하기 전에 IRQ를 Disable하지 않는 것**이다.

종료한 Task가 Scheduler로 CPU를 넘긴 뒤에는 다시 실행되지 않아야 하므로, 기존의 IRQ Disable 후 무한 대기 구조를 제거하고 Scheduler가 다음 Task를 선택하도록 변경했다.

### 결과

수정 후 User Task의 전체 실행 흐름이 정상적으로 동작했다.

```text
User Task 1
    ↓
getpid
    ↓
write
    ↓
yield
    ↓
sleep
    ↓
wake up
    ↓
exit
    ↓
schedule()
    ↓
User Task 2
    ↓
System Calls
    ↓
exit
    ↓
모든 User Task 종료
```

최종적으로 다음 메시지가 출력되며 Phase 6 테스트가 통과했다.

```text
[TEST] [PASS] User Mode (EL0) & System Calls Verified!
```

---

# 8. Phase 6 최종 검증 결과

Phase 6에서는 단순한 EL0 진입뿐만 아니라 User Task와 Kernel의 전체 실행 경로를 검증했다.

검증한 주요 기능은 다음과 같다.

```text
Kernel EL1
   │
   │ Context Switch
   ▼
User EL0
   │
   ├── getpid()
   ├── write()
   ├── yield()
   ├── sleep()
   └── exit()
          │
          ▼
     SVC Exception
          │
          ▼
   syscall_dispatch()
          │
          ▼
      Scheduler
          │
          ├── Context Switch
          ├── Sleep / Wakeup
          └── Task Exit
```

실제 실행 결과:

```text
[User Task 1] Running in EL0.
[User Task 1] Yielding CPU.
[User Task 1] Back from yield. Sleeping 5 ticks.
[User Task 1] Wokr up. Exiting.
[TEST] [PASS] User Mode (EL0) & System Calls Verified!
```

또한 기존 Kernel Scheduler의 Task들도 정상적으로 종료되었으며 최종적으로:

```text
>> Kernel is now in idle state.
```

까지 도달했다.

따라서 현재 구현에서는 **EL0 User Mode, SVC 기반 System Call, Timer 기반 Scheduling, Yield, Sleep/Wakeup, Task Exit까지의 기본 실행 흐름이 정상적으로 동작함을 확인했다.**

---

# 9. 전체 디버깅을 통해 얻은 고찰

MiniOS를 구현하면서 발생한 문제들은 단순한 문법 오류보다는 **소프트웨어와 하드웨어의 경계에서 발생하는 문제**가 대부분이었다.

특히 다음과 같은 관계를 직접 확인할 수 있었다.

```text
Compiler
   ↕
CPU Instruction Set
   ↕
Exception / Register
   ↕
MMU / TLB
   ↕
Interrupt Controller
   ↕
Scheduler
   ↕
User / Kernel Mode
```

일반적인 애플리케이션 개발에서는 추상화되어 있던 CPU Register, Exception Level, TLB, Interrupt Controller, Context Switch 등의 동작을 직접 다루면서, 작은 설정 오류 하나가 전체 시스템 정지로 이어질 수 있다는 것을 확인했다.

이번 디버깅에서 가장 중요했던 것은 단순히 오류를 수정하는 것이 아니라,

> **증상 → CPU 상태 확인 → 관련 하드웨어 레이어로 범위 축소 → 근본 원인 확인 → 최소한의 수정 → 테스트를 통한 검증**

의 순서로 문제를 추적하는 과정이었다.

이를 통해 MiniOS는 단순히 기능을 구현하는 프로젝트를 넘어, **ARM64 하드웨어와 Operating System이 실제로 어떻게 상호작용하는지 이해하고 검증하는 프로젝트**로 발전시킬 수 있었다.
