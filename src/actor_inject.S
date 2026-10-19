/* Called by patched JAL at 0x088576C8; game executes the unchanged
 * 0x088576CC lw a0,0x18(sp) in the JAL delay slot.
 * Not a normal C ABI function. DO NOT add stack frame or call C here.
 * Critical: original sw a0,0(fp) must read *prior* a0 (from 0x20(sp)),
 * not current a0 (already overwritten in the delay slot).
 * RA = 0x088576D0 on entry. The game's true RA is saved at 0x30(sp).
 */
    .set noreorder
    .set nomacro
    .text
    .align 2
    .globl ttt_actor_inject_shim
    .ent ttt_actor_inject_shim
ttt_actor_inject_shim:
    /* Guard: only actor P1's ID field, never P2-P4 or unrelated structs. */
    lui     $t0, 0x08B4
    ori     $t0, $t0, 0x8AE8
    bne     $fp, $t0, 9f
    nop
    lui     $t0, %hi(ttt_ai_enabled)
    lw      $t0, %lo(ttt_ai_enabled)($t0)
    beqz    $t0, 9f
    nop
    /* Record the last actually replaced assignment before changing ID. */
    lw      $t1, 0x20($sp)
    lui     $t0, %hi(ttt_ai_last_original_id)
    sw      $t1, %lo(ttt_ai_last_original_id)($t0)
    lui     $t0, %hi(ttt_ai_last_original_color)
    sw      $a1, %lo(ttt_ai_last_original_color)($t0)
    lui     $t0, %hi(ttt_ai_requested_id)
    lw      $t1, %lo(ttt_ai_requested_id)($t0)
    sw      $t1, 0($fp)
    lui     $t0, %hi(ttt_ai_last_requested_id)
    sw      $t1, %lo(ttt_ai_last_requested_id)($t0)
    /* Color uses a1 at PC=0x088576D0, so override it BEFORE returning. */
    lui     $t0, %hi(ttt_ai_requested_color)
    lw      $a1, %lo(ttt_ai_requested_color)($t0)
    lui     $t0, %hi(ttt_ai_last_requested_color)
    sw      $a1, %lo(ttt_ai_last_requested_color)($t0)
    lui     $t0, %hi(ttt_ai_hits)
    lw      $t1, %lo(ttt_ai_hits)($t0)
    addiu   $t1, $t1, 1
    sw      $t1, %lo(ttt_ai_hits)($t0)
    jr      $ra
    nop
9:
    /* Exact equivalent of replaced store for normal game execution. */
    lw      $t0, 0x20($sp)
    sw      $t0, 0($fp)
    jr      $ra
    nop
    .end ttt_actor_inject_shim
    .set macro
    .set reorder
