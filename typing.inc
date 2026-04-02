; ------------------------------------------------------------
; Typing challenge
; ------------------------------------------------------------

typing_challenge:
	call music_enable_cpukey
	call typing_select_word
	call typing_flush_keys
	xor a
	ld (typing_index),a
	ld (typing_last_key),a
	ld (typing_result),a
	ld a,TYPING_TICK_RELOAD
	ld (typing_tick_div),a
	call battle_clear_bottom_rows
	call typing_draw_word
	call typing_draw_cursor

typing_challenge_loop:
	call music_update
	ld a,(typing_tick_div)
	dec a
	ld (typing_tick_div),a
	jr nz,typing_challenge_poll_key
	ld a,TYPING_TICK_RELOAD
	ld (typing_tick_div),a
	ld a,(typing_timeout)
	or a
	jr z,typing_challenge_timeout
	dec a
	ld (typing_timeout),a

typing_challenge_poll_key:
	call ROM_KBD_INKEY
	jr z,typing_challenge_no_key
	call typing_normalize_key
	or a
	jr nz,typing_challenge_have_key
	xor a
	ld (typing_last_key),a
	jr typing_challenge_loop
typing_challenge_have_key:
	ld b,a
	ld a,(typing_last_key)
	cp b
	jr z,typing_challenge_loop
	ld a,b
	ld (typing_last_key),a
	call typing_process_key
	or a
	jr z,typing_challenge_loop
	jr typing_challenge_done

typing_challenge_no_key:
	xor a
	ld (typing_last_key),a
	jr typing_challenge_loop

typing_challenge_timeout:
	jr typing_challenge_timeout_miss

typing_challenge_timeout_miss:
	xor a
	ld (typing_result),a
	jr typing_challenge_done

typing_challenge_done:
	jp music_disable_cpukey

typing_process_key:
	ld a,(typing_index)
	ld e,a
	ld d,0
	ld hl,(typing_word_ptr)
	add hl,de
	ld a,(hl)
	cp b
	jr z,typing_process_key_correct

	ld a,(typing_index)
	inc a
	ld b,a
	ld a,(typing_early_limit)
	cp b
	jr nc,typing_process_key_miss
	jr typing_process_key_miss

typing_process_key_miss:
	call sfx_play_cancel
	xor a
	ld (typing_result),a
	ld a,1
	ret

typing_process_key_correct:
	call sfx_play_confirm
	ld a,(typing_index)
	inc a
	ld (typing_index),a
	ld b,a
	ld a,(typing_word_len)
	cp b
	jr z,typing_process_key_done
	call typing_draw_cursor
	xor a
	ret

typing_process_key_done:
	ld a,(typing_timeout)
	ld b,a
	ld a,(typing_bonus_limit)
	cp b
	jr z,typing_process_key_amp
	jr c,typing_process_key_amp
	ld a,TYPING_RESULT_NORMAL
	ld (typing_result),a
	ld a,1
	ret

typing_process_key_amp:
	ld a,TYPING_RESULT_AMP
	ld (typing_result),a
	ld a,1
	ret

typing_select_word:
	ld a,(player_level)
	cp 6
	jr nc,typing_select_group_1
	xor a
	jr typing_select_group_ready
typing_select_group_1:
	ld a,1
	ld b,a
	ld a,(player_level)
	cp 12
	ld a,b
	jr c,typing_select_group_ready
	ld a,2
	ld b,a
	ld a,(player_level)
	cp 24
	ld a,b
	jr c,typing_select_group_ready
	ld a,3
typing_select_group_ready:
	add a,a
	ld e,a
	ld d,0
	ld hl,typing_group_ptrs
	add hl,de
	ld e,(hl)
	inc hl
	ld d,(hl)
	ex de,hl
	call rng8
	ld b,a
	ld a,r
	xor b
	and $03
	add a,a
	ld e,a
	ld d,0
	add hl,de
	ld e,(hl)
	inc hl
	ld d,(hl)
	ex de,hl
	ld (typing_word_ptr),hl
	push hl
	ld b,0

typing_select_word_len_loop:
	ld a,(hl)
	or a
	jr z,typing_select_word_len_done
	inc b
	inc hl
	jr typing_select_word_len_loop

typing_select_word_len_done:
	ld a,b
	ld (typing_word_len),a
	pop hl

	ld a,(player_level)
	srl a
	srl a
	ld b,a
	ld a,72
	sub b
	jr nc,typing_timeout_nonneg
	xor a
typing_timeout_nonneg:
	cp 34
	jr nc,typing_timeout_store
	ld a,34
typing_timeout_store:
	ld (typing_timeout),a
	sub 22
	jr nc,typing_bonus_nonneg
	xor a
typing_bonus_nonneg:
	cp 16
	jr nc,typing_bonus_store
	ld a,16
typing_bonus_store:
	ld (typing_bonus_limit),a

	ld a,(typing_word_len)
	dec a
	ld e,a
	ld d,0
	ld hl,typing_early_limit_table
	add hl,de
	ld a,(hl)
	ld (typing_early_limit),a
	ret

typing_flush_keys:
	call ROM_KBD_INKEY
	ret z
	jr typing_flush_keys

typing_draw_word:
	ld a,(typing_word_len)
	srl a
	ld b,a
	ld a,11
	sub b
	ld (typing_word_x),a
	ld d,a
	ld e,3
	ld hl,(typing_word_ptr)
	ld (SYS_CURSOR_XY),de
	jp ROM_MSG_DISPLAY_NOSTR

typing_draw_cursor:
	ld a,T6834_CMD_LINE_CLEAR
	ld hl,param_line_clear_4
	ld bc,$0100
	ld de,SYS_T6834_REPLYBUF
	call ROM_T6834_CMD_PARAM
	ld a,(typing_word_x)
	ld b,a
	ld a,(typing_index)
	add a,b
	ld d,a
	ld e,4
	ld a,"^"
	ld (SYS_CURSOR_XY),de
	jp ROM_LCD_PUTC

typing_normalize_key:
	cp "a"
	jr c,typing_normalize_key_uppercase_check
	cp "z"+1
	jr nc,typing_normalize_key_uppercase_check
	and %11011111

typing_normalize_key_uppercase_check:
	cp "A"
	jr c,typing_normalize_key_invalid
	cp "Z"+1
	ret c

typing_normalize_key_invalid:
	xor a
	ret

typing_early_limit_table:
	.db 1,1,1,2,2,2,3,3,3,4
