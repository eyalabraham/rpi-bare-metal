# Barriers the hard way

- Check CP15 Memory Model Feature Register 2 with ```MRC p15, 0, <Rd>, c0, c1, 6```
  - Look at bits [23:20] which indicates support for memory barrier operations.
  - 0x2, ARM1176JZF-S processors support: Data Synchronization Barrier, Prefetch Flush, Data Memory Barrier.
- Check CP15 Instruction Set Attributes Register 4 bit functions with ```MRC p15, 0, <Rd>, c0, c2, 4```
  - Look at bits [19:16] which indicates support for barrier instructions.
  - 0x0, None. ARM1176JZF-S processors support only the CP15 barrier operations through CP15 above.
- For ARMv7 and above, use the ```DMB``` Assembly instruction, the CP15 equivalent barrier instructions available in ARMv6 are deprecated in ARMv7.
- We are concerned about Data Synchronization Barrier (DSB) and Data Memory Barrier (DMB)
  - for DMB: ```mcr p15, 0, r0, c7, c10, #5```
  - for DSB: ```mcr p15, 0, r0, c7, c10, #4```
- DMB and DSB can be done in user mode too.

> **Data Synchronization Barrier (DSB)** operation is to ensure that all outstanding explicit memory transactions complete before any following instructions begin. This ensures that data in memory is up to date before the processor executes any more instructions.  
> This instruction completes when all explicit memory transactions occurring in program order before this instruction are completed. No instructions occurring in program order after this instruction are executed until this instruction completes. Therefore, no explicit memory transactions occurring in program order after this instruction are started until this instruction completes.  
> Use whenever a memory access needs to have completed before program execution progresses.

> **Data Memory Barrier (DMB)** operation is to ensure that all outstanding explicit memory transactions complete before any following explicit memory transactions begin. This ensures that data in memory is up to date before any memory transaction that depends on it:  
> 1. Use whenever a memory access requires ordering with regards to another memory access.  
> 2. A memory write barrier **before the first write** to a peripheral.  
> 3. A memory read barrier **after the last read** of a peripheral.  
> 4. In a ISR, issue a DMB at the start and end of the routine.

## Code

- [Resource 1](https://www.raspberrypi.org/forums/viewtopic.php?t=82286)
- [Resource 2](https://www.raspberrypi.org/forums/viewtopic.php?t=234420)

```

//memory barriers for ARMv6. These 3 are special and non-privileged.
#ifndef barrierdefs
 #define barrierdefs
 //raspi is ARMv6. It does not have ARMv7 DMB/DSB/ISB, so go through CP15.
 #define isb() __asm__ __volatile__ ("mcr p15, 0, %0, c7,  c5, 4" : : "r" (0) : "memory")
 #define dmb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory")
 //ARMv6 DSB (DataSynchronizationBarrier): also known as DWB (drain write buffer / data write barrier) on ARMv5
 #define dsb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory")
#endif
//use dmb() around the accesses. ("before write, after read")

```

## ARM FAQ: In what situations might I need to insert memory barrier instructions?

Classic ARM processors, such as the ARM7TDMI, execute instructions and complete data accesses in program order.  The latest ARM processors can optimize the order of instruction execution and data accesses.  For example, an ARM architecture v6 or v7 processor could optimize the following sequence of instructions:  

```

LDR r0, [r1]   ; Load from Normal/Cacheable memory leads to a cache miss
STR r2, [r3]   ; Store to Normal/Non-cacheable memory

```

The first load from memory misses in the cache it will cause a cache line-fill.  This typically takes many cycles to complete. Classic (cached) ARM processors, for example, the ARM926EJ-S, would wait for the load to complete before executing the store instruction.  ARM architecture v6/v7 based processors can recognize that the next instruction does not depend on the result of the load (in register r0) and can execute the store instruction before the load instruction completes.  

In some circumstances, processor optimizations such as speculative reads or out-of-order execution (as in the example above), are undesirable and can lead to unintended program behavior.  In such situations it is necessary to insert barrier instructions into code where there is a requirement for stricter, 'Classic ARM processor-like' behavior.  There are three types of barrier instructions.  For simplicity, note that the descriptions below are for a uni-processor environment:  

- A Data Synchronization Barrier (DSB) completes when all instructions before this instruction complete.
- A Data Memory Barrier (DMB) ensures that all explicit memory accesses before the DMB instruction complete before any explicit memory accesses after the DMB instruction start.
- An Instruction Synchronization Barrier (ISB) flushes the pipeline in the processor, so that all instructions following the ISB are fetched from cache or memory, after the ISB has been completed.

The CP15 equivalent barrier instructions available in ARMv6 are deprecated in ARMv7. Therefore, if possible, it is recommended that any code that uses these instructions is migrated to use the new barrier instructions described above instead.

