; ------------------------------------------------------------
; GFX routines
; ------------------------------------------------------------

blit_buf_range_to_lcd_fast_burst:
	call music_render_begin
	push hl
	push de
	push bc
blit_loop_fast_burst:
	call game_break_pending
	jr nz,blit_fast_burst_abort_forward
	ld a,(de)
	inc de
	push de
	ld d,a
	call x_ram_write_fast_atomic
	jr nz,blit_fast_burst_abort_write
	pop de
	call music_poll_baud

	inc hl
	dec bc
	ld a,b
	or c
	jr nz,blit_loop_fast_burst

	pop bc
	pop de
	pop hl
	jr blit_fast_burst_done
blit_fast_burst_abort_forward:
	pop bc
	pop de
	pop hl
	jr blit_fast_burst_done
blit_fast_burst_abort_write:
	pop de
	jr blit_fast_burst_abort_forward
blit_fast_burst_done:
	jp music_render_end

blit_buf_range_to_lcd_diff_once:
	call music_render_begin
	push hl
	push de
	push bc
	push ix
blit_loop_diff_once:
	call game_break_pending
	jr nz,blit_diff_abort_forward
	ld a,(de)
	cp (ix+0)
	jr z,blit_loop_diff_skip
	push de
	ld d,a
	call x_ram_write_fast_atomic
	jr nz,blit_diff_abort_write
	pop de
	call music_poll_baud
blit_loop_diff_skip:
	inc de
	inc ix
	inc hl
	dec bc
	ld a,b
	or c
	jr nz,blit_loop_diff_once
	pop ix
	pop bc
	pop de
	pop hl
	jr blit_diff_done
blit_diff_abort_forward:
	pop ix
	pop bc
	pop de
	pop hl
	jr blit_diff_done
blit_diff_abort_write:
	pop de
	jr blit_diff_abort_forward
blit_diff_done:
	jp music_render_end

; draw current player sprite
draw_player:
player_x	equ $+1
	ld a,0
	ld (spr_x),a
player_y	equ $+1
	ld a,0
	ld (spr_y),a
	ld ix,player_sprite

draw_sprite8_at_player_ix:
	ld a,(player_x)
	ld (spr_x),a
	ld a,(player_y)
	ld (spr_y),a

; IN: IX = sprite ptr (8 bytes), spr_x multiple of 8, spr_y 0..24
draw_sprite8_aligned_ix:
	call music_render_begin
	; HL = MEV_SCREEN + spr_y*16 + (spr_x>>3)
spr_y	equ $+1
	ld a,0
	ld h,0
	ld l,a
	add hl,hl
	add hl,hl
	add hl,hl
	add hl,hl		; *16
	ld bc,MEV_SCREEN
	add hl,bc

spr_x	equ $+1
	ld a,0
	srl a
	srl a
	srl a
	ld c,a
	ld b,0
	add hl,bc
	push hl
	push ix
	ld b,8

row_forward:
	call game_break_pending
	jr nz,draw_sprite_abort_forward
	ld a,(ix)
	inc ix
	ld d,a
	call x_ram_write_fast_atomic
	jr nz,draw_sprite_abort_write
	call music_poll_baud
	ld de,16
	add hl,de
	djnz row_forward

	pop ix
	pop hl
	jr draw_sprite_done
draw_sprite_abort_forward:
	pop ix
	pop hl
	jr draw_sprite_done
draw_sprite_abort_write:
	pop ix
	pop hl
draw_sprite_done:
	jp music_render_end
