; ------------------------------------------------------------
; World music player
; - custom IRQ driver
; - pair8 format for map music
; - map music only for now
; - stops completely before ROM keyboard paths (battle/dialogue)
; ------------------------------------------------------------

MUSIC_MAP_DATA		equ map_music
MUSIC_VECTOR_JP		equ $C3
MUSIC_IRQ_ACK_BAUD	equ $10
MUSIC_IRQ_TICK_DIV_NORMAL	equ 2
MUSIC_IRQ_TICK_STEP_NORMAL	equ 1
MUSIC_IRQ_TICK_STEP_RENDER	equ 4
MUSIC_IRQ_REST_DIVISOR	equ 960
MUSIC_BUZZER_MODE_RUN	equ F4_SETBC|F4_BRGST|F4_MD1|F4_MD0
MUSIC_BUZZER_MODE_ON	equ MUSIC_BUZZER_MODE_RUN|F4_BZON
SFX_CONFIRM_NOTE		equ 18
SFX_CONFIRM_DURATION	equ 1
SFX_CANCEL_NOTE		equ 6
SFX_CANCEL_DURATION	equ 1
SFX_HIT_NOTE			equ 20
SFX_HIT_DURATION		equ 1
SFX_HURT_NOTE			equ 8
SFX_HURT_DURATION		equ 1

music_start_map:
	ld a,(music_driver_installed)
	or a
	ret nz
	ld a,T6834_CMD_CPUKEY_OUT_OFF
	call ROM_T6834_CMD
	ld a,T6834_CMD_KEYBUF_CLEAR
	call ROM_T6834_CMD
	jr music_start_map_continue

music_start_map_late:
	ld a,(music_driver_installed)
	or a
	ret nz
music_start_map_continue:
	call music_clear_breakflag
	ld a,(MUSIC_MAP_DATA)
	cp $FF
	ret z
	di
	call music_irq_install
	call music_restart_core
	ei
	ret

music_stop:
	ld a,(music_driver_installed)
	or a
	jr z,music_stop_restore_cpu
	call irq_player_stop
	call music_irq_restore

music_stop_restore_cpu:
	call music_force_buzzer_off
	ld a,T6834_CMD_KEYBUF_CLEAR
	call ROM_T6834_CMD
	ld a,T6834_CMD_CPUKEY_OUT_ON
	call ROM_T6834_CMD
	jp music_clear_breakflag

music_update:
	ld a,(music_driver_installed)
	or a
	ret z
	ld a,(music_render_active)
	or a
	ret z
	jp music_poll_baud

music_poll_baud:
	ld a,(music_driver_installed)
	or a
	ret z
	in a,(IOP_F2_BAUD_L_FLAGS)
	and F2R_BG1
	ret z
	push af
	push bc
	push de
	push hl
	push ix
	push iy
	call music_service_bg1
	pop iy
	pop ix
	pop hl
	pop de
	pop bc
	pop af
	ret

music_clear_breakflag:
	xor a
	ld (SYS_BREAKFLAG),a
	ret

music_capture_system_state:
	ld a,(SYS_INT_SERIAL)
	ld (saved_serial_vector+0),a
	ld hl,(SYS_INT_SERIAL+1)
	ld (saved_serial_vector+1),hl
	ld a,(SYS_INTIMAG)
	ld (saved_intimag),a
	in a,(IOP_F4_MODE)
	ld (saved_f4_mode),a
	ret

music_enable_cpukey:
	ld a,T6834_CMD_KEYBUF_CLEAR
	call ROM_T6834_CMD
	ld a,T6834_CMD_CPUKEY_OUT_ON
	jp ROM_T6834_CMD

music_disable_cpukey:
	ld a,T6834_CMD_KEYBUF_CLEAR
	call ROM_T6834_CMD
	ld a,T6834_CMD_CPUKEY_OUT_OFF
	jp ROM_T6834_CMD

music_reset_runtime_state:
	ld hl,0
	ld (irq_ticks),hl
	xor a
	ld (music_note),a
	ld (music_tick_accum),a
	ld (music_note_active),a
	ld (music_render_active),a
	ret

music_restart_core:
	call irq_player_pause
	ld hl,MUSIC_MAP_DATA
	ld (music_ptr),hl
	ld (music_loop_ptr),hl
	xor a
	ld (music_note),a
	ld (music_tick_accum),a
	ld (music_note_active),a
	ld hl,0
	ld (irq_ticks),hl
	call irq_player_load_next
	ret

music_render_begin:
	ld a,(music_render_active)
	inc a
	ld (music_render_active),a
	cp 1
	ret nz
	ld a,(music_driver_installed)
	or a
	ret z
	ld a,(SYS_INTIMAG)
	and $DF
	ld (SYS_INTIMAG),a
	out (IOP_F0_INTCTRL),a
	ret

music_render_end:
	ld a,(music_render_active)
	or a
	ret z
	dec a
	ld (music_render_active),a
	ret nz
	ld a,(music_driver_installed)
	or a
	ret z
	call music_poll_baud
	ld a,(SYS_INTIMAG)
	or F0_BIE
	ld (SYS_INTIMAG),a
	out (IOP_F0_INTCTRL),a
	ret

music_force_buzzer_off:
	call music_force_buzzer_off_quick
	ld a,T6834_CMD_BUZZER_OFF
	jp ROM_T6834_CMD

music_force_buzzer_off_quick:
	call music_reset_runtime_state
	xor a
	ld (MEV_BUZZER_CNT),a
	in a,(IOP_F4_MODE)
	and %11111101
	out (IOP_F4_MODE),a
	ld a,MUSIC_IRQ_ACK_BAUD
	out (IOP_F5_INTRESET),a
	ret

music_irq_install:
	ld a,(music_driver_installed)
	or a
	ret nz
	call music_capture_system_state

	ld a,MUSIC_VECTOR_JP
	ld (SYS_INT_SERIAL),a
	ld hl,irq_serial_handler
	ld (SYS_INT_SERIAL+1),hl

	ld a,1
	ld (music_driver_installed),a
	ret

music_irq_restore:
	ld a,(music_driver_installed)
	or a
	ret z
	jp music_restore_system_state

music_restore_system_state:
	di
	ld a,(saved_intimag)
	ld (SYS_INTIMAG),a
	out (IOP_F0_INTCTRL),a
	ld a,(saved_f4_mode)
	out (IOP_F4_MODE),a

	ld a,(saved_serial_vector+0)
	ld (SYS_INT_SERIAL),a
	ld hl,(saved_serial_vector+1)
	ld (SYS_INT_SERIAL+1),hl

	xor a
	ld (music_driver_installed),a
	ei
	ret

irq_player_pause:
	ld a,(SYS_INTIMAG)
	and $DF
	ld (SYS_INTIMAG),a
	out (IOP_F0_INTCTRL),a
	ld a,(saved_f4_mode)
	out (IOP_F4_MODE),a
	ld a,MUSIC_IRQ_ACK_BAUD
	out (IOP_F5_INTRESET),a
	ret

irq_player_stop:
	call irq_player_pause
	call music_reset_runtime_state
	xor a
	ld (MEV_BUZZER_CNT),a
	ret

irq_player_load_next:
	ld hl,(music_ptr)

irq_player_read_event:
	ld a,(hl)
	cp $FF
	jr z,irq_player_loop_restart
	inc hl
	ld e,(hl)
	inc hl
	ld d,0
	ld (music_ptr),hl
	jr irq_player_store_event

irq_player_loop_restart:
	ld hl,(music_loop_ptr)
	ld (music_ptr),hl
	jr irq_player_read_event

irq_player_store_event:
	ld (music_note),a
	ex de,hl
	ld (irq_ticks),hl
	jp irq_player_program_current

irq_player_program_current:
	ld a,(music_note)
	or a
	jr z,irq_player_program_rest
	dec a
	add a,a
	ld l,a
	ld h,0
	ld de,music_pitch_table
	add hl,de
	ld e,(hl)
	inc hl
	ld d,(hl)
	ld a,1
	ld (music_note_active),a
	jr irq_player_program_divisor

irq_player_program_rest:
	ld de,MUSIC_IRQ_REST_DIVISOR
	xor a
	ld (music_note_active),a

irq_player_program_divisor:
	ld a,(SYS_INTIMAG)
	and $DF
	ld (SYS_INTIMAG),a
	out (IOP_F0_INTCTRL),a

	ld a,e
	out (IOP_F2_BAUD_L_FLAGS),a
	ld a,d
	out (IOP_F3_BAUD_H),a
	ld a,(music_note_active)
	or a
	ld a,MUSIC_BUZZER_MODE_RUN
	jr z,irq_player_program_mode
	ld a,MUSIC_BUZZER_MODE_ON

irq_player_program_mode:
	out (IOP_F4_MODE),a
	ld a,(music_render_active)
	or a
	jr nz,irq_player_program_ack
	ld a,(SYS_INTIMAG)
	or F0_BIE
	ld (SYS_INTIMAG),a
	out (IOP_F0_INTCTRL),a
irq_player_program_ack:
	ld a,MUSIC_IRQ_ACK_BAUD
	out (IOP_F5_INTRESET),a
	ret

irq_serial_handler:
	push af
	push bc
	push de
	push hl
	push ix
	push iy
	exx
	ex af,af'
	push af
	push bc
	push de
	push hl

	in a,(IOP_F2_BAUD_L_FLAGS)
	and F2R_BG1
	jr z,irq_serial_chain
	call music_service_bg1
	jr irq_serial_return

irq_serial_return:
	pop hl
	pop de
	pop bc
	pop af
	ex af,af'
	exx
	pop iy
	pop ix
	pop hl
	pop de
	pop bc
	pop af
	ei
	ret

irq_serial_chain:
	pop hl
	pop de
	pop bc
	pop af
	ex af,af'
	exx
	pop iy
	pop ix
	pop hl
	pop de
	pop bc
	pop af
	ld hl,(saved_serial_vector+1)
	jp (hl)

music_service_bg1:
	ld a,(music_tick_accum)
	inc a
	ld (music_tick_accum),a
	cp MUSIC_IRQ_TICK_DIV_NORMAL
	jr nz,music_service_bg1_ack
	xor a
	ld (music_tick_accum),a

	ld hl,(irq_ticks)
	ld b,MUSIC_IRQ_TICK_STEP_NORMAL
	ld a,(music_render_active)
	or a
	jr z,music_service_step_loop
	ld b,MUSIC_IRQ_TICK_STEP_RENDER
music_service_step_loop:
	ld a,h
	or l
	jr z,music_service_load_next_store
	dec hl
	djnz music_service_step_loop

music_service_store_ticks:
	ld (irq_ticks),hl
	ld a,h
	or l
	jr nz,music_service_bg1_ack

music_service_load_next_store:
	ld (irq_ticks),hl
	call irq_player_load_next

music_service_bg1_ack:
	ld a,MUSIC_IRQ_ACK_BAUD
	out (IOP_F5_INTRESET),a
	ret

sfx_play_confirm:
	ld a,SFX_CONFIRM_NOTE
	ld b,SFX_CONFIRM_DURATION
	jr sfx_play

sfx_play_cancel:
	ld a,SFX_CANCEL_NOTE
	ld b,SFX_CANCEL_DURATION
	jr sfx_play

sfx_play_hit:
	ld a,SFX_HIT_NOTE
	ld b,SFX_HIT_DURATION
	jr sfx_play

sfx_play_hurt:
	ld a,SFX_HURT_NOTE
	ld b,SFX_HURT_DURATION

sfx_play:
	ld c,a
	ld a,(music_driver_installed)
	or a
	jr z,sfx_play_beep_fallback
	push bc
	push de
	push hl
	call irq_player_pause
	ld a,c
	dec a
	add a,a
	ld l,a
	ld h,0
	ld de,music_pitch_table
	add hl,de
	ld e,(hl)
	inc hl
	ld d,(hl)
	ld a,e
	out (IOP_F2_BAUD_L_FLAGS),a
	ld a,d
	out (IOP_F3_BAUD_H),a
	ld a,MUSIC_BUZZER_MODE_ON
	out (IOP_F4_MODE),a
sfx_play_delay_outer:
	push bc
	ld hl,$0100
sfx_play_delay_inner:
	dec hl
	ld a,h
	or l
	jr nz,sfx_play_delay_inner
	pop bc
	djnz sfx_play_delay_outer
	call irq_player_program_current
	pop hl
	pop de
	pop bc
	ret

sfx_play_beep_fallback:
	push bc
	push de
	push hl
	ld a,T6834_CMD_BUZZER_OFF
	call ROM_T6834_CMD
	pop hl
	pop de
	pop bc
	push bc
	push de
	push hl
	ld a,c
	ld e,a
	ld d,0
	ld hl,sfx_beep_resume
	push hl
	push de
	pop iy
	ld h,b
	ld l,0
	call ROM_BEEP

sfx_beep_resume:
	pop hl
	pop de
	pop bc
	ret
