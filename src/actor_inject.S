/* TTT V0.9.8: 1P/2P/3P/4P slot-specific pre-load actor override.
 * Only called from patched JAL at 0x088576C8. Not standard C ABI.
 * Original delay slot (0x088576CC: lw a0,0x18(sp)) stays in place.
 * We preserve t0/t1/t2 on a 16-byte temporary stack frame, restore
 * SP before returning to original instruction at 0x088576D0.
 * Original ID is in 0x20(game-sp) = 0x30(temporary-sp).
 * Color in a1 is changed only when that slot is explicitly enabled.
 */
    .set noreorder
    .set nomacro
    .text
    .align 2
    .globl ttt_actor_inject_shim
    .ent ttt_actor_inject_shim
ttt_actor_inject_shim:
    addiu   $sp, $sp, -16
    sw      $t0, 0($sp)
    sw      $t1, 4($sp)
    sw      $t2, 8($sp)
    /* Match EXACT 1P..4P actor ID destinations, not adjacent structs. */
    lui     $t0, 0x08B4
    ori     $t0, $t0, 0x8AE8
    beq     $fp, $t0, 1f
    addiu   $t1, $zero, 0
    lui     $t0, 0x08B4
    ori     $t0, $t0, 0x8BC0
    beq     $fp, $t0, 1f
    addiu   $t1, $zero, 4
    lui     $t0, 0x08B4
    ori     $t0, $t0, 0x8CB8
    beq     $fp, $t0, 1f
    addiu   $t1, $zero, 8
    lui     $t0, 0x08B4
    ori     $t0, $t0, 0x8D90
    beq     $fp, $t0, 1f
    addiu   $t1, $zero, 12
    beq     $zero, $zero, 9f
    nop
1:
    /* t1 is the 0/4/8/12 byte offset of the matched slot. */
    lui     $t0, %hi(ttt_ai_enabled_mask)
    lw      $t0, %lo(ttt_ai_enabled_mask)($t0)
    srl     $t2, $t1, 2
    srlv    $t0, $t0, $t2
    andi    $t0, $t0, 1
    beq     $t0, $zero, 9f
    nop
    /* A blank slot (0) or invalid ID (>0xA0) is not a participant.
     * Do not accidentally spawn a character in a vacant team slot. */
    lw      $t2, 0x30($sp)
    beq     $t2, $zero, 9f
    nop
    sltiu   $t0, $t2, 0xA1
    beq     $t0, $zero, 9f
    nop
    /* Save original ID before replacing it. */
    lui     $t0, %hi(ttt_ai_old_ids)
    addiu   $t0, $t0, %lo(ttt_ai_old_ids)
    addu    $t0, $t0, $t1
    sw      $t2, 0($t0)
    lui     $t0, %hi(ttt_ai_last_original_id)
    sw      $t2, %lo(ttt_ai_last_original_id)($t0)
    lui     $t0, %hi(ttt_ai_old_colors)
    addiu   $t0, $t0, %lo(ttt_ai_old_colors)
    addu    $t0, $t0, $t1
    sw      $a1, 0($t0)
    lui     $t0, %hi(ttt_ai_last_original_color)
    sw      $a1, %lo(ttt_ai_last_original_color)($t0)
    /* Place requested ID, preserving subsequent game code. */
    lui     $t0, %hi(ttt_ai_requested_ids)
    addiu   $t0, $t0, %lo(ttt_ai_requested_ids)
    addu    $t0, $t0, $t1
    lw      $t2, 0($t0)
    sw      $t2, 0($fp)
    lui     $t0, %hi(ttt_ai_last_requested_id)
    sw      $t2, %lo(ttt_ai_last_requested_id)($t0)
    /* PC 0x088576D0 will store a1 as Color: override it now. */
    lui     $t0, %hi(ttt_ai_requested_colors)
    addiu   $t0, $t0, %lo(ttt_ai_requested_colors)
    addu    $t0, $t0, $t1
    lw      $a1, 0($t0)
    lui     $t0, %hi(ttt_ai_last_requested_color)
    sw      $a1, %lo(ttt_ai_last_requested_color)($t0)
    /* Per-slot and global hit counts; slot 1-based in diagnostics. */
    lui     $t0, %hi(ttt_ai_hits_by_slot)
    addiu   $t0, $t0, %lo(ttt_ai_hits_by_slot)
    addu    $t0, $t0, $t1
    lw      $t2, 0($t0)
    addiu   $t2, $t2, 1
    sw      $t2, 0($t0)
    lui     $t0, %hi(ttt_ai_hits)
    lw      $t2, %lo(ttt_ai_hits)($t0)
    addiu   $t2, $t2, 1
    sw      $t2, %lo(ttt_ai_hits)($t0)
    srl     $t2, $t1, 2
    addiu   $t2, $t2, 1
    lui     $t0, %hi(ttt_ai_last_player)
    sw      $t2, %lo(ttt_ai_last_player)($t0)
    beq     $zero, $zero, 8f
    nop
9:
    /* Exact normal game ID assignment when no override for this slot. */
    lw      $t2, 0x30($sp)
    sw      $t2, 0($fp)
8:
    lw      $t0, 0($sp)
    lw      $t1, 4($sp)
    lw      $t2, 8($sp)
    addiu   $sp, $sp, 16
    jr      $ra
    nop
    .end ttt_actor_inject_shim
    .set macro
    .set reorder
