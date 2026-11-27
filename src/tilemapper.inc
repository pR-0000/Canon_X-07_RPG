; A = tile row within the current chunk.
tilemap_build_row_from_chunk_work:
	ld b,a
	ld a,(chunk_y)
	add a,a
	add a,a
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
	call music_render_begin
	ld b,SCREEN_HEIGHT_PIXELS
redraw_line:
	push bc
	ld a,SCREEN_HEIGHT_PIXELS
	sub b
	ld c,a
	and 7
	jr nz,redraw_send_line
	ld a,c
	rrca
	rrca
	rrca
	push bc
	call tilemap_build_row_from_chunk_work
	pop bc

redraw_send_line:
	; Rebuild both pointers from the scanline, never from the previous blit.
	ld l,c
	ld h,0
	add hl,hl
	add hl,hl
	add hl,hl
	add hl,hl
	ld a,l
	and 7*MEV_WIDTH_BYTES
	ld de,MEV_SCREEN
	add hl,de
	ex de,hl
	ld hl,row_work_buf
	ld c,a
	ld b,0
	add hl,bc
	ex de,hl
	ld bc,CHUNK_W_TILES
	call blit_buf_range_to_lcd_fast_burst
	call game_break_pending
	jr nz,redraw_chunk_quit
	call music_poll_baud
	pop bc
	djnz redraw_line
	jp music_render_end

redraw_chunk_quit:
	pop bc
	call music_render_end
	jp quit
