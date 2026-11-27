; ------------------------------------------------------------
; Menu routines
; ------------------------------------------------------------

menu:
	xor a
	ld (menu_vertical_mode),a
	jr menu_enter

menu_vertical:
	ld a,1
	ld (menu_vertical_mode),a

menu_enter:
	xor a
	ld (previous_option),a
	ld (current_option),a
	ld a,(hl)
	ld (max_cursor_options),a
	inc hl
	ld (menu_jump_ptr),hl
	ld b,0
	ld c,a
	add hl,bc
	add hl,bc	; skip jump addresses
	ld (menu_entry_ptr),hl
	ld b,a

menu_draw_options:
	ld a,(hl)
	ld d,a
	inc hl
	ld a,(hl)
	ld e,a
	inc hl
	push bc
	push hl
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR
	pop hl
menu_draw_options_skip_string:
	ld a,(hl)
	inc hl
	or a
	jr nz,menu_draw_options_skip_string
	pop bc
	djnz menu_draw_options
	call menu_draw_cursor

menu_loop:
	call music_update

	ld a,T6834_STICK_CMD
	ld bc,1
	ld de,SYS_T6834_REPLYBUF
	call ROM_T6834_CMD_PARAM
	ld a,(SYS_T6834_REPLYBUF)
	ld b,a
	ld a,(menu_vertical_mode)
	or a
	ld a,b
	jr z,menu_loop_horizontal
	cp T6834_STICK_UP
	jr z,menu_go_previous
	cp T6834_STICK_DOWN
	jr z,menu_go_next
	jr menu_check_cancel

menu_loop_horizontal:
	cp T6834_STICK_LEFT
	jr z,menu_go_previous
	cp T6834_STICK_RIGHT
	jr z,menu_go_next
	jr menu_check_cancel

menu_go_previous:
	call menu_previous_option
	call menu_wait_stick_release
	jr menu_check_cancel

menu_go_next:
	call menu_next_option
	call menu_wait_stick_release
	jr menu_check_cancel

menu_check_cancel:
	call test_s1_keys
	bit 1,a	; DEL key ([DOC] S1/K2)
	jr z,menu_check_confirm
	call menu_wait_cancel_release
	ld hl,(menu_cancel_ptr)
	ld a,h
	or l
	jr z,menu_check_confirm
	call sfx_play_cancel
	jp (hl)

menu_check_confirm:
	call test_confirm_key
	jr z,menu_loop
	ld a,(current_option)
	add a,a	; *2
	ld d,0
	ld e,a
	ld hl,(menu_jump_ptr)
	add hl,de	; HL -> handler word
	ld a,(hl)
	ld e,a
	inc hl
	ld a,(hl)
	ld d,a
	ex de,hl
	push hl
	call sfx_play_confirm
	pop hl
	jp (hl)	; execute handler

menu_previous_option:
	ld a,(current_option)
	or a
	ret z
	dec a
	ld (current_option),a
	jp menu_draw_cursor

menu_next_option:
	ld a,(current_option)
	ld b,a
max_cursor_options	equ $+1
	ld a,0
	dec a
	cp b
	ret z
	ld a,b
	inc a
	ld (current_option),a
	jp menu_draw_cursor

menu_wait_stick_release:
	call music_update
	ld a,T6834_STICK_CMD
	ld bc,1
	ld de,SYS_T6834_REPLYBUF
	call ROM_T6834_CMD_PARAM
	ld a,(SYS_T6834_REPLYBUF)
	cp T6834_STICK_NONE
	jr nz,menu_wait_stick_release
	ret

menu_wait_cancel_release:
	call music_update
	call test_s1_keys
	bit 1,a
	jr nz,menu_wait_cancel_release
	ret

menu_set_cancel_none:
	xor a
	ld (menu_cancel_ptr),a
	ld (menu_cancel_ptr+1),a
	ret

menu_set_cancel_world_menu:
	ld hl,world_menu_leave_to_map
	ld (menu_cancel_ptr),hl
	ret

menu_set_cancel_castle_menu:
	ld hl,castle_menu_enter
	ld (menu_cancel_ptr),hl
	ret

menu_set_cancel_battle_menu:
	ld hl,battle_show_menu
	ld (menu_cancel_ptr),hl
	ret

menu_draw_cursor:
	; erase old cursor at previous option if any
previous_option	equ $+1
	ld a,0
	ld c," "
	call menu_draw_cursor_at_index
	; draw cursor at current option
current_option	equ $+1
	ld a,0
	ld (previous_option),a
	ld c,">"

menu_draw_cursor_at_index:
	ld b,a
	ld hl,(menu_entry_ptr)
	or a
	jr z,menu_draw_cursor_done

menu_draw_cursor_skip_index:
	inc hl		; x
	inc hl		; y
menu_draw_cursor_skip_string:
	ld a,(hl)
	inc hl
	or a
	jr nz,menu_draw_cursor_skip_string
	djnz menu_draw_cursor_skip_index

menu_draw_cursor_done:
	ld a,(hl)	; x
	dec a		; cursor is at x-1
	ld (SYS_CURSOR_XY+1),a
	inc hl
	ld a,(hl)	; y
	ld (SYS_CURSOR_XY),a
	ld a,c
	jp ROM_LCD_PUTC
