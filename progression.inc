; ------------------------------------------------------------
; Player and enemy progression
; ------------------------------------------------------------

init_player_state:
	xor a
	ld (battle_event_flag_mask),a
	ld (battle_outcome),a
	ld (battle_enemy_tile),a
	ld (last_player_damage),a
	ld (last_enemy_damage),a
	ld (level_up_flag),a
	ld (last_player_bonus),a
	ld (player_atk_bonus),a
	ld (player_def_bonus),a
	ld (inventory_smokes),a
	ld (encounter_cooldown),a
	ld a,DEBUG_INITIAL_FLAGS
	ld (event_flags),a
	ld a,r
	ld (rng_seed),a
	ld hl,0
	ld (player_exp),hl
	ld (battle_enemy_name_ptr),hl
	ld hl,DEBUG_INITIAL_GOLD
	ld (player_gold),hl
	ld a,DEBUG_INITIAL_LEVEL
	ld (player_level),a
	ld a,1
	ld (inventory_potions),a
	call progress_sync_player_stats_from_level
	jp progress_heal_player_full

progress_sync_player_stats_from_level:
	call progress_calc_player_hp_max
	ld (player_hp_max),a

	call progress_calc_player_atk
	ld b,a
	ld a,(player_atk_bonus)
	add a,b
	ld (player_atk),a

	call progress_calc_player_def
	ld b,a
	ld a,(player_def_bonus)
	add a,b
	ld (player_def),a

	call progress_calc_player_flee
	ld (player_flee),a
	ret

progress_calc_player_hp_max:
	ld a,(player_level)
	ld e,a
	ld l,a
	ld h,0
	add hl,hl
	ld a,e
	srl a
	ld e,a
	ld d,0
	add hl,de
	ld de,26
	add hl,de
	ld a,h
	or a
	jr z,progress_calc_player_hp_max_store
	ld a,$FF
	ret
progress_calc_player_hp_max_store:
	ld a,l
	ret

progress_calc_player_atk:
	ld a,(player_level)
	ld b,a
	srl a
	srl a
	ld c,a
	ld a,b
	srl a
	srl a
	srl a
	add a,c
	add a,3
	ret

progress_calc_player_def:
	ld a,(player_level)
	ld b,a
	srl a
	srl a
	srl a
	ld c,a
	ld a,b
	srl a
	srl a
	srl a
	srl a
	add a,c
	add a,3
	ret

progress_calc_player_flee:
	ld a,(player_level)
	ld b,a
	srl a
	ld c,a
	ld a,b
	srl a
	srl a
	add a,c
	add a,24
	ret

progress_heal_player_full:
	ld a,(player_hp_max)
	ld (player_hp_cur),a
	ret

progress_get_current_chunk_zone:
	call world_get_player_global_coords
	ld a,b
	cp 9
	jr c,progress_zone_top_half
	ld a,c
	cp 23
	jr nc,progress_zone_tower
	ld a,c
	cp 10
	jr nc,progress_zone_cemetery
	ld a,b
	cp 10
	jr c,progress_zone_forest
	ld a,1
	ret

progress_zone_top_half:
	ld a,c
	cp 10
	jr c,progress_zone_forest
	ld a,4
	ret

progress_zone_forest:
	ld a,3
	ret

progress_zone_cemetery:
	ld a,2
	ret

progress_zone_tower:
	ld a,5
	ret

progress_select_random_enemy:
	call progress_get_current_chunk_zone
	dec a
	jr progress_load_enemy_from_id

progress_load_enemy_from_id:
	add a,a
	add a,a
	add a,a
	ld e,a
	ld d,0
	ld hl,enemy_slime
	add hl,de
	ld a,(hl)
	ld (battle_enemy_tile),a
	inc hl
	ld a,(hl)
	ld (battle_enemy_hp_max),a
	ld (battle_enemy_hp_cur),a
	inc hl
	ld a,(hl)
	ld (battle_enemy_atk),a
	inc hl
	ld a,(hl)
	ld (battle_enemy_def),a
	inc hl
	ld a,(hl)
	ld (battle_enemy_exp),a
	inc hl
	ld a,(hl)
	ld (battle_enemy_gold),a
	inc hl
	ld e,(hl)
	inc hl
	ld d,(hl)
	ex de,hl
	ld (battle_enemy_name_ptr),hl
	ret

progress_award_battle_rewards:
	xor a
	ld (level_up_flag),a
	ld a,(battle_enemy_exp)
	ld e,a
	ld d,0
	ld hl,(player_exp)
	add hl,de
	ld (player_exp),hl

	ld a,(battle_enemy_gold)
	ld e,a
	ld d,0
	ld hl,(player_gold)
	add hl,de
	ld (player_gold),hl
	jp progress_check_level_up

progress_check_level_up:
	ld a,(player_level)
	cp MAX_LEVEL
	ret z

progress_level_up_loop:
	ld a,(player_level)
	cp MAX_LEVEL
	ret z
	call progress_get_level_threshold
	ld e,l
	ld d,h
	ld hl,(player_exp)
	or a
	sbc hl,de
	ret c
	ld a,(player_level)
	inc a
	ld (player_level),a
	ld a,1
	ld (level_up_flag),a
	call progress_sync_player_stats_from_level
	call progress_heal_player_full
	jr progress_level_up_loop

progress_get_level_threshold:
	ld a,(player_level)
	ld b,a
	add a,a
	add a,8
	ld c,a
	ld hl,0
	ld d,0
progress_get_level_threshold_loop:
	ld e,c
	add hl,de
	djnz progress_get_level_threshold_loop
	ret
