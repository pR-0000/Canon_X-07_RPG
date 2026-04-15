; ------------------------------------------------------------
; Battle routines
; ------------------------------------------------------------

check_random_battle:
	IF DEBUG_SKIP_RANDOM_BATTLES
	ret
	ENDIF
	call progress_get_current_chunk_zone
	or a
	ret z
	ld a,(encounter_cooldown)
	or a
	jr z,battle_can_roll
	dec a
	ld (encounter_cooldown),a
	ret

battle_can_roll:
	call rng8
	and $07	; probability = 1/8
	ret nz
	ld a,4	; avoid battles in next 4 tiles
	ld (encounter_cooldown),a
	call progress_select_random_enemy
	call music_clear_breakflag
	ld a,T6834_CMD_KEYBUF_CLEAR
	call ROM_T6834_CMD
	xor a
	ld (battle_event_flag_mask),a
	ld (battle_outcome),a
	call battle
	jp battle_return_to_map

battle:
	call music_clear_breakflag
	ld a,T6834_CMD_KEYBUF_CLEAR
	call ROM_T6834_CMD
	ld a,T6834_CMD_CPUKEY_OUT_OFF
	call ROM_T6834_CMD
	xor a
	ld (last_player_damage),a
	ld (last_enemy_damage),a
	ld (level_up_flag),a
	ld (last_player_bonus),a
	ld a,(battle_enemy_hp_max)
	ld (battle_enemy_hp_cur),a
	call battle_draw_scene
	call battle_show_intro
	jr battle_show_menu

battle_show_menu:
	call battle_prepare_menu_ui
	call menu_set_cancel_none
	ld hl,menu_battle
	jp menu

battle_show_objects_menu:
	call battle_prepare_menu_ui
	call battle_patch_object_menu_counts
	call menu_set_cancel_battle_menu
	ld hl,menu_objects
	jp menu_vertical

battle_draw_scene:
	call ROM_LCD_CLS
	ld hl,(battle_enemy_name_ptr)
	ld d,11
	ld e,1
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR

	ld a,32
	ld (spr_x),a
	ld a,8
	ld (spr_y),a
	ld ix,player_sprite
	call draw_sprite8_aligned_ix

	ld a,80
	ld (spr_x),a
	ld a,(battle_enemy_tile)
	ld l,a
	ld h,0
	add hl,hl
	add hl,hl
	add hl,hl
	ld de,tiles
	add hl,de
	push hl
	pop ix
	jp draw_sprite8_aligned_ix

battle_prepare_menu_ui:
	call battle_refresh_status
	jr battle_clear_bottom_rows

battle_refresh_status:
	ld hl,msg_hp_clear
	ld d,1
	ld e,2
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR
	ld hl,msg_hp_clear
	ld d,18
	ld e,2
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR

	ld d,1
	ld e,2
	ld (SYS_CURSOR_XY),de
	ld a,(player_hp_cur)
	ld l,a
	ld h,0
	call ROM_HL_TO_ASCII_PRINT

	ld d,18
	ld e,2
	ld (SYS_CURSOR_XY),de
	ld a,(battle_enemy_hp_cur)
	ld l,a
	ld h,0
	jp ROM_HL_TO_ASCII_PRINT

battle_clear_bottom_rows:
	ld a,T6834_CMD_LINE_CLEAR
	ld hl,param_line_clear_3
	ld bc,$0100
	ld de,SYS_T6834_REPLYBUF
	call ROM_T6834_CMD_PARAM
	ld a,T6834_CMD_LINE_CLEAR
	ld hl,param_line_clear_4
	ld bc,$0100
	ld de,SYS_T6834_REPLYBUF
	jp ROM_T6834_CMD_PARAM

battle_show_message:
	push hl
	call battle_clear_bottom_rows
	pop hl
	ld d,1
	ld e,3
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR
	jp ui_wait_confirm_with_music

battle_show_intro:
	call battle_clear_bottom_rows
	ld hl,(battle_enemy_name_ptr)
	ld d,1
	ld e,3
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR
	ld hl,msg_battle_intro
	ld d,1
	ld e,4
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR
	jp ui_wait_confirm_with_music

battle_patch_object_menu_counts:
	ld a,(inventory_potions)
	ld hl,menu_obj_potion_count
	call battle_store_two_digits_a
	ld a,(inventory_smokes)
	ld hl,menu_obj_smoke_count
	jr battle_store_two_digits_a

battle_store_two_digits_a:
	cp 100
	jr c,battle_store_two_digits_loop
	ld a,99
battle_store_two_digits_loop:
	ld b,'0'
battle_store_two_digits_sub10:
	cp 10
	jr c,battle_store_two_digits_done
	sub 10
	inc b
	jr battle_store_two_digits_sub10
battle_store_two_digits_done:
	ld (hl),b
	inc hl
	add a,'0'
	ld (hl),a
	ret

battle_attack:
	call typing_challenge
	call battle_clear_bottom_rows
	call battle_apply_player_attack
	ld a,(battle_enemy_hp_cur)
	push af
	call battle_refresh_status
	pop af
	or a
	jr nz,battle_attack_enemy_turn
	call battle_show_player_damage_message
	jp battle_victory

battle_attack_enemy_turn:
	call battle_enemy_turn_refresh
	call battle_show_round_messages
	ld a,(player_hp_cur)
	or a
	jp z,battle_defeat
	jp battle_show_menu

battle_flee:
	call rng8
	and $3F
	ld b,a
	ld a,(player_flee)
	srl a
	srl a
	add a,4
	cp b
	jr c,battle_flee_failed
	ld a,BATTLE_OUTCOME_FLEE
	ld (battle_outcome),a
	ret

battle_flee_failed:
	call battle_enemy_turn_refresh
	ld a,(player_hp_cur)
	or a
	jp z,battle_defeat
	jp battle_show_menu

battle_use_potion:
	ld a,(inventory_potions)
	or a
	jr nz,battle_use_potion_have_stock
	ld hl,msg_no_potion
	jp battle_show_message_then_objects_menu

battle_use_potion_have_stock:
	ld a,(player_hp_cur)
	ld b,a
	ld a,(player_hp_max)
	cp b
	jr nz,battle_use_potion_apply
	ld hl,msg_full_hp
	jp battle_show_message_then_objects_menu

battle_use_potion_apply:
	ld a,(inventory_potions)
	dec a
	ld (inventory_potions),a
	ld a,(player_hp_max)
	ld c,0
battle_use_potion_div3_loop:
	cp 3
	jr c,battle_use_potion_div3_done
	sub 3
	inc c
	jr battle_use_potion_div3_loop
battle_use_potion_div3_done:
	or a
	jr z,battle_use_potion_div3_exact
	inc c
battle_use_potion_div3_exact:
	ld a,c
	or a
	jr nz,battle_use_potion_amount_ready
	inc a
battle_use_potion_amount_ready:
	ld b,a
	ld a,(player_hp_cur)
	add a,b
	ld b,a
	ld a,(player_hp_max)
	cp b
	jr c,battle_use_potion_cap
	ld a,b
	jr battle_use_potion_store

battle_use_potion_cap:
	ld a,(player_hp_max)

battle_use_potion_store:
	ld (player_hp_cur),a
	call battle_enemy_turn_refresh
	ld a,(player_hp_cur)
	or a
	jp z,battle_defeat
	jp battle_show_menu

battle_use_smoke:
	ld a,(inventory_smokes)
	or a
	jr nz,battle_use_smoke_apply
	ld hl,msg_no_smoke
	jp battle_show_message_then_objects_menu

battle_use_smoke_apply:
	ld a,(inventory_smokes)
	dec a
	ld (inventory_smokes),a
	ld a,BATTLE_OUTCOME_FLEE
	ld (battle_outcome),a
	ret

battle_show_message_then_objects_menu:
	call battle_show_message
	jp battle_show_objects_menu

battle_enemy_turn_refresh:
	call battle_enemy_turn
	jp battle_refresh_status

battle_apply_player_attack:
	xor a
	ld e,a
	ld (last_player_bonus),a
	ld a,(typing_result)
	cp TYPING_RESULT_MISS
	jr z,battle_apply_player_attack_miss
	call battle_calc_player_damage
	ld b,a
	inc b
	inc e
	ld a,(typing_result)
	cp TYPING_RESULT_AMP
	jr nz,battle_apply_player_attack_store
	inc b
	inc e

battle_apply_player_attack_store:
	ld a,(battle_event_flag_mask)
	or a
	jr nz,battle_apply_player_attack_cap_boss
	ld a,(battle_enemy_hp_max)
	ld c,a
	srl a
	bit 0,c
	jr z,battle_apply_player_attack_cap_ready
	inc a
	jr battle_apply_player_attack_cap_ready

battle_apply_player_attack_cap_boss:
	ld a,(battle_enemy_hp_max)
	ld c,0
battle_apply_player_attack_cap_div4:
	cp 4
	jr c,battle_apply_player_attack_cap_boss_ready
	sub 4
	inc c
	jr battle_apply_player_attack_cap_div4

battle_apply_player_attack_cap_boss_ready:
	ld a,c

battle_apply_player_attack_cap_ready:
	or a
	jr nz,battle_apply_player_attack_cap_nonzero
	inc a
battle_apply_player_attack_cap_nonzero:
	cp b
	jr nc,battle_apply_player_attack_store_capped
	ld b,a
battle_apply_player_attack_store_capped:
	ld a,e
	cp b
	jr c,battle_apply_player_attack_store_split
	ld a,b
	ld (last_player_bonus),a
	xor a
	ld (last_player_damage),a
	ld a,b
	jr battle_apply_player_attack_subtract

battle_apply_player_attack_store_split:
	ld a,e
	ld (last_player_bonus),a
	ld a,b
	sub e
	ld (last_player_damage),a
	ld a,b
	jr battle_apply_player_attack_subtract

battle_apply_player_attack_miss:
	xor a
	ld (last_player_damage),a
	ld (last_player_bonus),a

battle_apply_player_attack_subtract:
	ld c,a
	ld a,(battle_enemy_hp_cur)
	ld b,a
	or a
	jr z,battle_apply_player_attack_zero
battle_apply_player_attack_damage_ready:
	ld a,c
	or a
	jr z,battle_apply_player_attack_store_hp
	push bc
	call sfx_play_hit
	pop bc
battle_apply_player_attack_store_hp:
	ld a,b
	cp c
	jr c,battle_apply_player_attack_zero
	jr z,battle_apply_player_attack_zero
	sub c
	ld (battle_enemy_hp_cur),a
	ret

battle_apply_player_attack_zero:
	xor a
	ld (battle_enemy_hp_cur),a
	ret

battle_calc_player_damage:
	ld a,(player_atk)
	ld b,a
	ld a,(player_level)
	srl a
	srl a
	add a,b
	ld b,a
	ld a,(battle_enemy_def)
	srl a
	srl a
	ld c,a
	ld a,b
	sub c
	jr nc,battle_calc_player_damage_min
	xor a
battle_calc_player_damage_min:
	inc a
battle_calc_player_damage_done:
	ret

battle_enemy_turn:
	ld a,(battle_enemy_atk)
	srl a
	ld b,a
	ld a,(player_def)
	ld c,a
	srl c
	srl c
	ld a,b
	sub c
	jr nc,battle_enemy_turn_min
	xor a

battle_enemy_turn_min:
	inc a
	ld b,a
	call rng8
	and $01
	add a,b
	ld c,a
	ld a,c
	ld (last_enemy_damage),a
	or a
	jr z,battle_enemy_turn_store_hp
	push bc
	call sfx_play_hurt
	pop bc
battle_enemy_turn_store_hp:
	ld a,(player_hp_cur)
	ld b,a
	cp c
	jr c,battle_enemy_turn_zero
	jr z,battle_enemy_turn_zero
	ld a,b
	sub c
	IF DEBUG_IMMORTAL
	or a
	jr nz,battle_enemy_turn_store_hp_debug_ok
	ld a,1
battle_enemy_turn_store_hp_debug_ok:
	ENDIF
	ld (player_hp_cur),a
	ret

battle_enemy_turn_zero:
	IF DEBUG_IMMORTAL
	ld a,1
	ld (player_hp_cur),a
	ret
	ENDIF
	xor a
	ld (player_hp_cur),a
	ret

battle_victory:
	ld a,BATTLE_OUTCOME_WIN
	ld (battle_outcome),a
	call progress_award_battle_rewards
	call battle_show_rewards
	ld a,(battle_event_flag_mask)
	or a
	ret z
	ld b,a
	ld a,(event_flags)
	or b
	ld (event_flags),a
	ld a,b
	cp EVENT_FLAG_BOSS_TOWER
	ret nz
	call game_autosave
	call ROM_LCD_CLS
	ld hl,msg_fin
	ld d,10
	ld e,2
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR
	jp quit

battle_defeat:
	ld a,BATTLE_OUTCOME_LOSE
	ld (battle_outcome),a
	jp game_over_restart

battle_show_rewards:
	call battle_clear_bottom_rows
	ld hl,msg_victory
	ld d,1
	ld e,3
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR
	ld a,(level_up_flag)
	or a
	jr z,battle_show_rewards_gold
	ld hl,msg_level_up
	ld d,13
	ld e,3
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR
battle_show_rewards_gold:
	ld hl,msg_gold_gain
	ld d,1
	ld e,4
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR
	ld a,(battle_enemy_gold)
	ld l,a
	ld h,0
	ld d,7
	ld e,4
	ld (SYS_CURSOR_XY),de
	call ROM_HL_TO_ASCII_PRINT
	jp ui_wait_confirm_with_music

battle_show_round_messages:
	call battle_clear_bottom_rows
	ld hl,msg_damage_dealt
	ld d,1
	ld e,3
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR
	call battle_print_player_damage_with_bonus
	ld hl,msg_damage_taken
	ld d,1
	ld e,4
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR
	ld a,(last_enemy_damage)
	ld l,a
	ld h,0
	ld d,8
	ld e,4
	ld (SYS_CURSOR_XY),de
	call ROM_HL_TO_ASCII_PRINT
	jp ui_wait_f6_with_music

battle_show_player_damage_message:
	call battle_clear_bottom_rows
	ld hl,msg_damage_dealt
	ld d,1
	ld e,3
	ld (SYS_CURSOR_XY),de
	call ROM_MSG_DISPLAY_NOSTR
	call battle_print_player_damage_with_bonus
	jp ui_wait_f6_with_music

battle_print_player_damage_with_bonus:
	ld a,(last_player_damage)
	ld l,a
	ld h,0
	ld d,8
	ld e,3
	ld (SYS_CURSOR_XY),de
	call ROM_HL_TO_ASCII_PRINT
	ld d,11
	ld e,3
	ld a,'+'
	ld (SYS_CURSOR_XY),de
	call ROM_LCD_PUTC
	ld a,(last_player_bonus)
	ld l,a
	ld h,0
	ld d,12
	ld e,3
	ld (SYS_CURSOR_XY),de
	jp ROM_HL_TO_ASCII_PRINT
