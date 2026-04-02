; ------------------------------------------------------------
; rng8
; Out: A = new seed (pseudo-random)
; Uses: rng_seed
; ------------------------------------------------------------

rng8:
	ld a,(rng_seed)
	or a
	jr nz,rng8_step
	ld a,r
rng8_step:
	rra			; bit0 -> carry
	jr nc,no_xor
	xor $B8

no_xor:
	ld (rng_seed),a
	ret

; ------------------------------------------------------------
; UI helpers
; ------------------------------------------------------------

ui_show_modal_message:
	call ROM_LCD_CLS
	ld d,2
	ld e,1
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR
	ld a,T6834_CMD_KEYBUF_CLEAR
	call ROM_T6834_CMD
	jp ui_wait_confirm_with_music

ui_show_console_message:
	call ROM_LCD_CLS

ui_show_console_message_char_loop:
	ld a,(hl)
	inc hl
	or a
	jr z,ui_show_console_message_done
	cp 13
	jr z,ui_show_console_message_newline
	call ROM_LCD_PUTC
	jr ui_show_console_message_char_loop

ui_show_console_message_newline:
	ld a,13
	call ROM_LCD_PUTC
	ld a,10
	call ROM_LCD_PUTC
	jr ui_show_console_message_char_loop

ui_show_console_message_done:
	ld a,T6834_CMD_KEYBUF_CLEAR
	call ROM_T6834_CMD
	jp ui_wait_confirm_with_music

world_set_home_position:
	ld a,HOME_CHUNK_X
	ld (chunk_x),a
	ld a,HOME_CHUNK_Y
	ld (chunk_y),a
	ld a,HOME_PLAYER_X
	ld (player_x),a
	ld a,HOME_PLAYER_Y
	ld (player_y),a
	ret

game_has_save:
	ld a,(save_magic)
	cp SAVE_MAGIC_VALUE
	ret

game_autosave:
	ld a,SAVE_MAGIC_VALUE
	ld (save_magic),a
	ld a,(chunk_x)
	ld (save_chunk_x),a
	ld a,(chunk_y)
	ld (save_chunk_y),a
	ld a,(player_x)
	ld (save_player_x),a
	ld a,(player_y)
	ld (save_player_y),a
	ld a,(player_level)
	ld (save_player_level),a
	ld a,(player_hp_cur)
	ld (save_player_hp_cur),a
	ld a,(player_atk_bonus)
	ld (save_player_atk_bonus),a
	ld a,(player_def_bonus)
	ld (save_player_def_bonus),a
	ld a,(inventory_potions)
	ld (save_inventory_potions),a
	ld a,(inventory_smokes)
	ld (save_inventory_smokes),a
	ld a,(event_flags)
	ld (save_event_flags),a
	ld hl,(player_exp)
	ld (save_player_exp),hl
	ld hl,(player_gold)
	ld (save_player_gold),hl
	ret

game_load_save:
	ld a,(save_chunk_x)
	ld (chunk_x),a
	ld a,(save_chunk_y)
	ld (chunk_y),a
	ld a,(save_player_x)
	ld (player_x),a
	ld a,(save_player_y)
	ld (player_y),a
	ld a,(save_player_level)
	ld (player_level),a
	ld a,(save_player_atk_bonus)
	ld (player_atk_bonus),a
	ld a,(save_player_def_bonus)
	ld (player_def_bonus),a
	ld a,(save_inventory_potions)
	ld (inventory_potions),a
	ld a,(save_inventory_smokes)
	ld (inventory_smokes),a
	ld a,(save_event_flags)
	ld (event_flags),a
	ld hl,(save_player_exp)
	ld (player_exp),hl
	ld hl,(save_player_gold)
	ld (player_gold),hl
	call progress_sync_player_stats_from_level
	ld a,(save_player_hp_cur)
	ld (player_hp_cur),a
	xor a
	ld (battle_event_flag_mask),a
	ld (battle_outcome),a
	ld (battle_enemy_tile),a
	ld (last_player_damage),a
	ld (last_enemy_damage),a
	ld (level_up_flag),a
	ld (last_player_bonus),a
	ld (encounter_cooldown),a
	ld (post_move_pending),a
	ld (redraw_active),a
	ld (redraw_row),a
	ld (redraw_subrow),a
	ld hl,0
	ld (battle_enemy_name_ptr),hl
	ret

game_begin_from_title:
	call game_has_save
	jr nz,game_begin_new
	jp game_load_save

game_begin_new:
	call init_player_state
	call world_set_home_position
	call game_autosave
	IF DEBUG_SKIP_INTRO
	jp game_autosave
	ENDIF
	call ui_show_intro_text
	call music_clear_breakflag
	jp game_autosave

title_show:
	call title_draw_screen
	IF DEBUG_SKIP_STARTUP_MUSIC
	; keep title silent in debug builds
	ELSE
	call music_start_map
	ENDIF
	call menu_set_cancel_none
	ld hl,menu_title
	call menu_vertical
	push af
	call music_clear_breakflag
	pop af
	ret

title_draw_screen:
	call ROM_LCD_CLS
	ld hl,msg_title_screen
	ld d,3
	ld e,1
	ld (SYS_CURSOR_XY),de
	jp ROM_MSG_DISPLAY_NOSTR

title_menu_continue:
	xor a
	ret

title_menu_quit:
	ld a,1
	ret

ui_show_intro_text:
	call music_enable_cpukey
	call ROM_LCD_CLS
	ld hl,INTRO_TEXT_PTR

ui_show_intro_text_char_loop:
	ld a,(hl)
	inc hl
	or a
	jr z,ui_show_intro_text_done
	cp 13
	jr z,ui_show_intro_text_newline
	push hl
	call ROM_LCD_PUTC
	call ui_intro_char_delay
	pop hl
	jr ui_show_intro_text_char_loop

ui_show_intro_text_newline:
	push hl
	ld a,13
	call ROM_LCD_PUTC
	ld a,10
	call ROM_LCD_PUTC
	call ui_intro_char_delay
	pop hl
	jr ui_show_intro_text_char_loop

ui_show_intro_text_done:
	call ui_intro_finish_input
	call music_disable_cpukey
	jp ui_wait_confirm_with_music

ui_intro_char_delay:
	push bc
	push de
	push hl
	ld b,2
ui_intro_char_delay_outer:
	call music_update
	ld de,$0500
ui_intro_char_delay_inner:
	dec de
	ld a,d
	or e
	jr nz,ui_intro_char_delay_inner
	djnz ui_intro_char_delay_outer
	pop hl
	pop de
	pop bc
	ret

ui_intro_finish_input:
	ld a,T6834_CMD_KEYBUF_CLEAR
	jp ROM_T6834_CMD

game_break_pending:
	ld a,(SYS_BREAKFLAG)
	and $81
	ret

test_s1_keys:
	ld a,T6834_CMD_TEST_KEY
	ld hl,param_check_S1_keys
	ld bc,$0201
	ld de,SYS_T6834_REPLYBUF
	call ROM_T6834_CMD_PARAM
	ld a,(SYS_T6834_REPLYBUF)
	ret

test_fcnt_keys:
	ld a,T6834_CMD_TEST_KEY
	ld hl,param_check_Fcnt_keys
	ld bc,$0201
	ld de,SYS_T6834_REPLYBUF
	call ROM_T6834_CMD_PARAM
	ld a,(SYS_T6834_REPLYBUF)
	ret

test_confirm_key:
	call test_fcnt_keys
	bit 5,a
	ret

ui_test_confirm_keys:
	call test_s1_keys
	bit 1,a
	ret nz
	jp test_confirm_key

ui_wait_confirm_release:
	call music_update
	call ui_test_confirm_keys
	jr nz,ui_wait_confirm_release
	ret

ui_wait_confirm_with_music:
	call ui_wait_confirm_release
ui_wait_confirm_with_music_loop:
	call music_update
	call ui_test_confirm_keys
	jr z,ui_wait_confirm_with_music_loop
	call ui_wait_confirm_release
	ld a,T6834_CMD_KEYBUF_CLEAR
	jp ROM_T6834_CMD

ui_wait_f6_release:
	call music_update
	call test_confirm_key
	jr nz,ui_wait_f6_release
	ret

ui_wait_f6_with_music:
	call ui_wait_f6_release
ui_wait_f6_with_music_loop:
	call music_update
	call test_confirm_key
	jr z,ui_wait_f6_with_music_loop
	jp ui_wait_f6_release

game_over_restart:
	call music_clear_breakflag
	call music_disable_cpukey
	call ROM_LCD_CLS
	ld hl,msg_game_over
	ld d,6
	ld e,2
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR
	call ui_wait_f6_with_music
	call progress_heal_player_full
	call world_set_home_position
	call music_clear_breakflag
	call game_autosave
	call world_redraw_map_with_player
	jp main

game_break_poll_pending:
	call ROM_BREAKFLAG_QUERY
	ld a,(SYS_BREAKFLAG)
	and $81
	ret

world_redraw_map_with_player:
	call redraw_chunk
	jp draw_player

battle_return_to_map:
	call music_clear_breakflag
	call music_disable_cpukey
	call game_autosave
	jp world_redraw_map_with_player

village_show_no_gold:
	ld hl,msg_no_gold
	call ui_show_modal_message
	jp village_menu_enter

x_ram_write_fast_nodi:
	ld e,T6834_CMD_RAM_WRITE
	call send_t6834_e_fast
	ret nz
	ld e,d
	call send_t6834_e_fast
	ret nz
	ld e,l
	call send_t6834_e_fast
	ret nz
	ld e,h
	call send_t6834_e_fast
	ret nz
	ld a,(SYS_INTIMAG)
	out (IOP_F0_INTCTRL),a
	xor a
	ret

x_ram_write_fast_atomic:
	di
	call x_ram_write_fast_nodi
	ei
	ret

send_t6834_e_fast:
	ld a,(SYS_BREAKFLAG)
	and $81
	ret nz
send_t6834_e_fast_wait:
	in a,(IOP_F2_BAUD_L_FLAGS)
	and %00000010
	jr nz,send_t6834_e_fast_ready
	push bc
	push de
	push hl
	ei
	call game_break_pending
	di
	pop hl
	pop de
	pop bc
	ret nz
	jr send_t6834_e_fast_wait
send_t6834_e_fast_ready:
	ld a,(SYS_INTIMAG)
	or $80
	out (IOP_F0_INTCTRL),a
	ld a,e
	out (IOP_F1_XBTR_XBRR),a
	ld a,$02
	out (IOP_F5_INTRESET),a
	xor a
	ret
