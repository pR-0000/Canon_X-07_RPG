; ------------------------------------------------------------
; Movement and collision routines
; ------------------------------------------------------------

move_left_chunk:
	ld a,(player_x)
	or a
	jr z,move_left_cross_chunk
	ld a,(chunk_x)
	ld (target_chunk_x),a
	ld a,(chunk_y)
	ld (target_chunk_y),a
	ld a,(player_x)
	sub STEP_X
	ld (target_player_x),a
	ld a,(player_y)
	ld (target_player_y),a
	jp world_try_target_move

move_left_cross_chunk:
	ld a,(chunk_x)
	or a
	ret z	; already at world left edge
	ld a,(chunk_x)
	dec a
	ld (target_chunk_x),a
	ld a,(chunk_y)
	ld (target_chunk_y),a
	ld a,SCREEN_WIDTH_PIXELS-8	; 112
	ld (target_player_x),a
	ld a,(player_y)
	ld (target_player_y),a
	jp world_try_target_move

move_right_chunk:
	ld a,(player_x)
	cp SCREEN_WIDTH_PIXELS-8
	jr z,move_right_cross_chunk
	ld a,(chunk_x)
	ld (target_chunk_x),a
	ld a,(chunk_y)
	ld (target_chunk_y),a
	ld a,(player_x)
	add a,STEP_X
	ld (target_player_x),a
	ld a,(player_y)
	ld (target_player_y),a
	jp world_try_target_move

move_right_cross_chunk:
	ld a,(chunk_x)
	cp CHUNKS_X-1	; 2
	ret z
	ld a,(chunk_x)
	inc a
	ld (target_chunk_x),a
	ld a,(chunk_y)
	ld (target_chunk_y),a
	xor a
	ld (target_player_x),a	; 0
	ld a,(player_y)
	ld (target_player_y),a
	jp world_try_target_move

move_up_chunk:
	ld a,(player_y)
	or a
	jr z,move_up_cross_chunk
	ld a,(chunk_x)
	ld (target_chunk_x),a
	ld a,(chunk_y)
	ld (target_chunk_y),a
	ld a,(player_y)
	sub STEP_Y
	ld (target_player_y),a
	ld a,(player_x)
	ld (target_player_x),a
	jp world_try_target_move

move_up_cross_chunk:
	ld a,(chunk_y)
	or a
	ret z
	ld a,(chunk_y)
	dec a
	ld (target_chunk_y),a
	ld a,(chunk_x)
	ld (target_chunk_x),a
	ld a,SCREEN_HEIGHT_PIXELS-8	; 24
	ld (target_player_y),a
	ld a,(player_x)
	ld (target_player_x),a
	jp world_try_target_move

move_down_chunk:
	ld a,(player_y)
	cp SCREEN_HEIGHT_PIXELS-8
	jr z,move_down_cross_chunk
	ld a,(chunk_x)
	ld (target_chunk_x),a
	ld a,(chunk_y)
	ld (target_chunk_y),a
	ld a,(player_y)
	add a,STEP_Y
	ld (target_player_y),a
	ld a,(player_x)
	ld (target_player_x),a
	jp world_try_target_move

move_down_cross_chunk:
	ld a,(chunk_y)
	cp CHUNKS_Y-1	; 3
	ret z
	ld a,(chunk_y)
	inc a
	ld (target_chunk_y),a
	ld a,(chunk_x)
	ld (target_chunk_x),a
	xor a
	ld (target_player_y),a	; 0
	ld a,(player_x)
	ld (target_player_x),a
	jp world_try_target_move

world_try_target_move:
	call world_can_enter_target_tile
	jr c,world_try_target_move_apply
	jp check_target_tile_event

world_try_target_move_apply:
	ld a,1
	ld (moved),a
	ld a,(chunk_x)
	ld b,a
	ld a,(target_chunk_x)
	cp b
	jr nz,world_try_target_move_redraw
	ld a,(chunk_y)
	ld b,a
	ld a,(target_chunk_y)
	cp b
	jr nz,world_try_target_move_redraw
	call get_tile_sprite_under_player
	call draw_sprite8_at_player_ix
	call world_commit_target_position
	call draw_player
	jp music_update

world_try_target_move_redraw:
	call get_tile_sprite_under_player
	call draw_sprite8_at_player_ix
	call world_commit_target_position
	call start_chunk_redraw
	ld a,1
	ld (post_move_pending),a
	jp music_update

world_commit_target_position:
	ld a,(target_chunk_x)
	ld (chunk_x),a
	ld a,(target_chunk_y)
	ld (chunk_y),a
	ld a,(target_player_x)
	ld (player_x),a
	ld a,(target_player_y)
	ld (player_y),a
	jp game_autosave

world_can_enter_target_tile:
	call world_get_target_tile_id
	cp 9
	ret

world_get_target_tile_id:
	ld a,(target_player_x)
	srl a
	srl a
	srl a
	ld c,a

	ld a,(target_player_y)
	srl a
	srl a
	srl a
	ld b,a

	ld a,(target_chunk_y)
	add a,a
	add a,a
	add a,b
	ld b,a

	ld a,(target_chunk_x)
	ld e,a
	add a,a
	add a,a
	add a,a
	add a,a
	sub e
	add a,c
	ld c,a
	jp world_get_map_tile_bc
