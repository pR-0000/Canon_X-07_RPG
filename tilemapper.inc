tilemap_build_row_from_chunk_work:
	ld a,(chunk_y)
	add a,a
	add a,a
	ld b,a
	ld a,(redraw_row)
	add a,b
	ld b,a

	ld a,(chunk_x)
	ld c,a
	add a,a
	add a,a
	add a,a
	add a,a
	sub c
	ld c,a

	ld hl,row_work_buf
	ld d,CHUNK_W_TILES

tilemap_build_row_loop:
	push hl
	call world_get_map_tile_bc
	pop hl
	call tilemap_write_tile_a_to_hl
	call music_poll_baud
	inc c
	dec d
	jr nz,tilemap_build_row_loop
	ret

tilemap_write_tile_a_to_hl:
	push bc
	push de
	push ix
	push hl

	ld e,a
	ld d,0
	sla e
	rl d
	sla e
	rl d
	sla e
	rl d

	ld ix,tiles
	add ix,de
	ld b,8

tilemap_write_tile_loop:
	ld d,(ix)
	inc ix
	ld (hl),d
	ld de,MEV_WIDTH_BYTES
	add hl,de
	djnz tilemap_write_tile_loop

	pop hl
	inc hl
	pop ix
	pop de
	pop bc
	ret

tilemap_get_row_offset:
	ld a,(redraw_row)
	ld l,a
	ld h,0
	add hl,hl
	add hl,hl
	add hl,hl
	add hl,hl
	add hl,hl
	add hl,hl
	add hl,hl
	ret

redraw_prepare_work_slice:
	ld a,(redraw_subrow)
	dec a
	ld l,a
	ld h,0
	add hl,hl
	add hl,hl
	add hl,hl
	add hl,hl
	push hl
	ld de,row_work_buf
	add hl,de
	ex de,hl
	call tilemap_get_row_offset
	pop bc
	add hl,bc
	ld bc,MEV_SCREEN
	add hl,bc
	ld bc,CHUNK_W_TILES
	ret

start_chunk_redraw:
	ld a,1
	ld (redraw_active),a
	call music_render_begin
	xor a
	ld (redraw_row),a
	ld (redraw_subrow),a
	ret

redraw_step:
	ld a,(redraw_subrow)
	or a
	jr nz,redraw_step_draw_row

redraw_step_build_row:
	call tilemap_build_row_from_chunk_work
	ld a,1
	ld (redraw_subrow),a

redraw_step_draw_row:
	call redraw_prepare_work_slice
	call blit_buf_range_to_lcd_fast_burst
	call music_poll_baud
	ld a,(redraw_subrow)
	inc a
	cp 9
	jr z,redraw_step_next_row
	ld (redraw_subrow),a
	ret

redraw_step_finish:
	xor a
	ld (redraw_active),a
	ld (redraw_subrow),a
	jp music_render_end

redraw_abort:
	xor a
	ld (redraw_active),a
	ld (redraw_subrow),a
	ld (post_move_pending),a
	jp music_render_end

redraw_step_next_row:
	ld a,(redraw_row)
	cp CHUNK_H_TILES-1
	jr z,redraw_step_finish
	inc a
	ld (redraw_row),a
	xor a
	ld (redraw_subrow),a
	ret

get_tile_sprite_under_player:
	call world_get_player_tile_id

	ld l,a
	ld h,0
	add hl,hl
	add hl,hl
	add hl,hl
	ld de,tiles
	add hl,de

	push hl
	pop ix
	ret

redraw_chunk:
	call start_chunk_redraw
redraw_chunk_wait:
	call game_break_pending
	jp nz,redraw_chunk_quit
	call redraw_step
	call game_break_pending
	jp nz,redraw_chunk_quit
	ld a,(redraw_active)
	or a
	call nz,redraw_step
	call game_break_pending
	jp nz,redraw_chunk_quit
	ld a,(redraw_active)
	or a
	call nz,redraw_step
	call game_break_pending
	jp nz,redraw_chunk_quit
	ld a,(redraw_active)
	or a
	call nz,redraw_step
	call music_update
	call game_break_pending
	jp nz,redraw_chunk_quit
	ld a,(redraw_active)
	or a
	jr nz,redraw_chunk_wait
	ret

redraw_chunk_quit:
	call redraw_abort
	jp quit
