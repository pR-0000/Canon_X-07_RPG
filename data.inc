; ------------------------------------------------------------
; Data
; ------------------------------------------------------------

CHAR_E_DIAERESIS	equ $99
CHAR_E_GRAVE		equ $9A
CHAR_E_CIRC		equ $9B
CHAR_E_ACUTE		equ $9C
CHAR_E_CAPITAL_ACUTE	equ $98
CHAR_A_GRAVE		equ $89
CHAR_A_CIRC		equ $8A
CHAR_C_CAPITAL_CEDILLA	equ $E4
CHAR_C_CEDILLA	equ $E5

; Runtime BSS for compact state.
; Keep the hot row buffer in low RAM and pin the small runtime block just
; below the loader placed at $1F00.
RUNTIME_BASE		equ $1E80
menu_jump_ptr		equ RUNTIME_BASE
menu_entry_ptr		equ menu_jump_ptr+2
menu_vertical_mode	equ menu_entry_ptr+2
menu_cancel_ptr		equ menu_vertical_mode+1
music_ptr		equ menu_cancel_ptr+2
music_loop_ptr		equ music_ptr+2
irq_ticks		equ music_loop_ptr+2
music_note		equ irq_ticks+2
music_driver_installed	equ music_note+1
saved_serial_vector	equ music_driver_installed+1
saved_intimag		equ saved_serial_vector+3
saved_return_sp		equ saved_intimag+1
saved_f4_mode		equ saved_return_sp+2
music_tick_accum	equ saved_f4_mode+1
music_note_active	equ music_tick_accum+1
music_render_active	equ music_note_active+1
redraw_active		equ music_render_active+1
redraw_row		equ redraw_active+1
redraw_subrow		equ redraw_row+1
post_move_pending	equ redraw_subrow+1
param_check_Fcnt_keys:	.db $00, $20
param_check_S1_keys:	.db $00, $01
param_line_clear_3:	.db 2
param_line_clear_4:	.db 3
rng_seed		equ post_move_pending+1
encounter_cooldown	equ rng_seed+1

; Offsets in the 30x16 tile map for each visible 15x4 chunk
; chunk_y: 0..3  (each chunk row = 4 tile-rows => 4*30 = 120)
; chunk_x: 0..1  (each chunk col = 15 tiles)

chunk_x		equ encounter_cooldown+1
chunk_y		equ chunk_x+1

target_chunk_x		equ chunk_y+1
target_chunk_y		equ target_chunk_x+1
target_player_x		equ target_chunk_y+1
target_player_y		equ target_player_x+1

event_flags		equ target_player_y+1
world_event_param	equ event_flags+1
world_event_mask		equ world_event_param+1
save_magic		equ world_event_mask+1
save_chunk_x		equ save_magic+1
save_chunk_y		equ save_chunk_x+1
save_player_x		equ save_chunk_y+1
save_player_y		equ save_player_x+1
save_player_level	equ save_player_y+1
save_player_hp_cur	equ save_player_level+1
save_player_atk_bonus	equ save_player_hp_cur+1
save_player_def_bonus	equ save_player_atk_bonus+1
save_inventory_potions	equ save_player_def_bonus+1
save_inventory_smokes	equ save_inventory_potions+1
save_event_flags	equ save_inventory_smokes+1
save_player_exp		equ save_event_flags+1
save_player_gold	equ save_player_exp+2

enemy_slime:
	.db 16,10,3,10,4,7
	.dw enemy_name_slime

enemy_fantome:
	.db 17,18,5,11,6,10
	.dw enemy_name_fantome

enemy_araignee:
	.db 19,22,6,12,8,13
	.dw enemy_name_araignee

enemy_bandit_ruins:
	.db 20,36,8,13,12,18
	.dw enemy_name_bandit

enemy_bandit_tower:
	.db 20,48,9,15,16,22
	.dw enemy_name_bandit

enemy_squelette:
	.db 18,28,8,12,24,36
	.dw enemy_name_squelette

enemy_bandit_boss:
	.db 20,72,12,16,40,60
	.dw enemy_name_bandit

enemy_demon_ruins:
	.db 21,96,14,18,64,84
	.dw enemy_name_demon

enemy_demon_oasis:
	.db 21,120,16,20,88,108
	.dw enemy_name_demon

enemy_sorcier:
	.db 22,160,20,22,128,160
	.dw enemy_name_sorcier

; ------------------------------------------------------------
; Music data
; pair8 format
; custom IRQ driver
; ------------------------------------------------------------

music_pitch_table:
	.dw $028E,$02DE,$0226,$01EA,$0338,$03D4,$00F5,$0113
	.dw $0123,$0147,$016F,$0185,$01B4,$0246,$030A,$0369
	.dw $0269,$019C,$0135,$01CE,$039D,$0207
map_music:
	.db $01,$FF,$01,$D7,$02,$34,$01,$3B,$00,$28,$03,$46
	.db $01,$3B,$00,$28,$02,$34,$01,$3B,$00,$28,$04,$4E
	.db $01,$B0,$05,$5D,$01,$3B,$02,$34,$00,$28,$06,$FF
	.db $06,$13,$07,$4E,$08,$46,$09,$42,$0A,$3B,$0B,$34
	.db $0C,$31,$0D,$84,$00,$3C,$04,$27,$03,$23,$0E,$21
	.db $01,$1D,$02,$1A,$0F,$19,$10,$58,$00,$A0,$03,$FF
	.db $03,$FF,$03,$31,$11,$3E,$01,$3B,$00,$28,$04,$4E
	.db $03,$46,$00,$28,$11,$3E,$01,$3B,$04,$4E,$03,$46
	.db $11,$BB,$01,$75,$02,$34,$0C,$FF,$0C,$29,$01,$3B
	.db $04,$4E,$0C,$63,$0B,$FF,$0B,$3B,$01,$3B,$04,$4E
	.db $0B,$69,$0C,$FF,$0C,$29,$01,$3B,$04,$4E,$0C,$63
	.db $0D,$58,$01,$3B,$04,$4E,$0C,$63,$01,$3B,$04,$27
	.db $11,$1F,$0B,$FF,$0B,$3B,$02,$9D,$0C,$FF,$0C,$FF
	.db $0C,$52,$12,$FF,$12,$19,$0B,$FF,$0B,$3B,$07,$FF
	.db $07,$3A,$08,$FF,$08,$18,$13,$F9,$12,$BA,$14,$A6
	.db $04,$FF,$04,$3A,$03,$8C,$11,$7C,$03,$FF,$03,$A4
	.db $04,$FF,$04,$89,$03,$46,$11,$3E,$00,$28,$02,$B7
	.db $04,$27,$0B,$69,$00,$50,$11,$3E,$00,$28,$02,$34
	.db $11,$3E,$00,$28,$02,$B7,$04,$27,$0B,$69,$00,$50
	.db $11,$3E,$00,$28,$02,$34,$01,$3B,$00,$28,$05,$A3
	.db $03,$23,$12,$5D,$00,$50,$01,$3B,$00,$28,$05,$2F
	.db $01,$3B,$00,$28,$05,$A3,$03,$23,$12,$5D,$00,$50
	.db $01,$3B,$00,$28,$05,$2F,$02,$34,$00,$28,$06,$9D
	.db $00,$28,$15,$2A,$05,$2F,$02,$34,$01,$3B,$11,$3E
	.db $03,$FF,$03,$A4,$01,$B0,$03,$D1,$04,$FF,$04,$3A
	.db $0F,$31,$02,$34,$01,$B0,$0F,$19,$02,$1A,$01,$1D
	.db $11,$1F,$03,$23,$16,$25,$04,$FF,$04,$3A,$03,$46
	.db $11,$3E,$01,$B0,$04,$EB,$06,$EB,$15,$F9,$10,$FF
	.db $10,$09,$05,$FF,$05,$19,$04,$FF,$04,$D7,$14,$FF
	.db $14,$F4,$0D,$FF,$0D,$FF,$0D,$12,$0C,$FF,$0C,$FF
	.db $0C,$52,$FF

player_sprite	equ tiles+120
ennemy_sprites	equ tiles+128

tiles: ; (tiles)
	.db %00000000
	.db %00000000
	.db %00000000
	.db %00000000
	.db %00000000
	.db %00000000
	.db %00000000
	.db %00000000
	.db %00000000
	.db %00000000
	.db %01000000
	.db %00001000
	.db %00000000
	.db %01000000
	.db %00000010
	.db %00000000
	.db %10100000
	.db %11101010
	.db %11111111
	.db %11001100
	.db %11111000
	.db %10011001
	.db %11111011
	.db %11001011
	.db %00001011
	.db %10101111
	.db %11111111
	.db %01100111
	.db %00111111
	.db %00110011
	.db %10111111
	.db %10100110
	.db %01011100
	.db %01101110
	.db %01000111
	.db %10101011
	.db %10000011
	.db %10010011
	.db %10010011
	.db %11111110
	.db %00000100
	.db %00001100
	.db %00011100
	.db %00010100
	.db %00110100
	.db %01010100
	.db %01010100
	.db %00010001
	.db %00001000
	.db %00011100
	.db %00001000
	.db %00001000
	.db %00011000
	.db %00101100
	.db %01110110
	.db %11111111
	.db %00001111
	.db %00010000
	.db %00100011
	.db %01000100
	.db %01000100
	.db %10000100
	.db %10000010
	.db %10000001
	.db %10101010
	.db %11111110
	.db %01011100
	.db %01110100
	.db %01011100
	.db %01111100
	.db %01101100
	.db %01101100
	.db %11111111
	.db %10110011
	.db %11001101
	.db %11111111
	.db %11111111
	.db %10110011
	.db %11001101
	.db %11111111
	.db %00000000
	.db %00001000
	.db %00010100
	.db %00110010
	.db %01011010
	.db %01001001
	.db %10101001
	.db %11111111
	.db %00000000
	.db %00010000
	.db %00111000
	.db %01111100
	.db %00111000
	.db %01111100
	.db %11111110
	.db %00010000
	.db %01001000
	.db %00110001
	.db %00010001
	.db %10010010
	.db %01111110
	.db %00001100
	.db %00011100
	.db %00101010
	.db %00000000
	.db %00010000
	.db %00110000
	.db %10110000
	.db %11110100
	.db %00111100
	.db %00110000
	.db %00110000
	.db %01100010
	.db %10010101
	.db %01101010
	.db %11111111
	.db %10011001
	.db %00011000
	.db %00111000
	.db %01110000
	.db %00000000
	.db %00011000
	.db %00011000
	.db %00111100
	.db %01111110
	.db %10111101
	.db %00100100
	.db %00100100
	.db %00000000
	.db %00111100
	.db %01111110
	.db %11010111
	.db %11010111
	.db %11010111
	.db %11111111
	.db %01111110
	.db %00111000
	.db %00010100
	.db %00111100
	.db %01111111
	.db %11111110
	.db %00111100
	.db %00011000
	.db %00010000
	.db %00111000
	.db %01010100
	.db %01111100
	.db %00101000
	.db %10010010
	.db %10111010
	.db %00010000
	.db %01101100
	.db %01011010
	.db %10111101
	.db %00111100
	.db %01111110
	.db %10111101
	.db %01011010
	.db %10011001
	.db %10100101
	.db %01011110
	.db %10001010
	.db %10011100
	.db %01100011
	.db %11001101
	.db %00101101
	.db %00001110
	.db %00001010
	.db %01000010
	.db %01111110
	.db %00010100
	.db %00111000
	.db %00000110
	.db %01011101
	.db %10011100
	.db %01001010
	.db %01000100
	.db %10111111
	.db %01001010
	.db %10101110
	.db %01011111
	.db %01101010
	.db %01001110
	.db %01011011

; Packed world map: 2 tile ids per byte (high nibble then low nibble)
	include "map_packed.inc"

; Runtime buffers live outside the loaded binary.
; Keep the hot row work buffer in lower RAM where the tilemapper was stable.
row_work_buf		equ $1000

msg_title_screen:
	.db "Canon X-07 QUEST",0

menu_title:
	.db 2
	.dw title_menu_continue
	.dw title_menu_quit
	.db 8,3," CONT.",0
	.db 8,4," QUIT.",0

msg_hp_clear:
	.db "   ",0

	IF BUILD_LANG_EN
menu_battle:	; number of options, jump addresses, coordinates (X, Y), zero-terminated string
	.db 3
	.dw battle_attack
	.dw battle_show_objects_menu
	.dw battle_flee
	.db 5,4,"Atk",0
	.db 10,4,"Obj",0
	.db 15,4,"Flee",0

menu_objects:
	.db 2
	.dw battle_use_potion
	.dw battle_use_smoke
	.db 4,3,"Potion("
menu_obj_potion_count:
	.db "0","0",")",0
	.db 4,4,"Smoke("
menu_obj_smoke_count:
	.db "0","0",")",0

menu_village:
	.db 3
	.dw village_talk
	.dw village_rest
	.dw village_shop_menu_enter
	.db 3,2,"Talk",0
	.db 3,3,"Inn",0
	.db 3,4,"Shop",0

menu_village_shop:
	.db 4
	.dw shop_buy_potion
	.dw shop_buy_smoke
	.dw shop_buy_blade
	.dw shop_buy_armor
	.db 2,1,"Potion 22G",0
	.db 2,2,"Smoke 32G",0
	.db 2,3,"Blade+3 144G",0
	.db 2,4,"Arm.+3 144G",0

menu_castle:
	.db 3
	.dw castle_palace
	.dw castle_inn_menu_enter
	.dw castle_shop_menu_enter
	.db 3,1,"King",0
	.db 3,2,"Inn",0
	.db 3,3,"Shop",0

menu_castle_inn:
	.db 2
	.dw castle_inn_yes
	.dw inn_reenter
	.db 3,3,"Yes",0
	.db 3,4,"No",0

menu_castle_shop:
	.db 4
	.dw shop_buy_potion
	.dw shop_buy_smoke
	.dw shop_buy_blade
	.dw shop_buy_armor
	.db 2,1,"Potion 14G",0
	.db 2,2,"Smoke 22G",0
	.db 2,3,"Blade+1 48G",0
	.db 2,4,"Arm.+1 48G",0

typing_word_ptrs:
	.dw typing_word_blow
	.dw typing_word_rush
	.dw typing_word_slam
	.dw typing_word_stab
	.dw typing_word_strike
	.dw typing_word_charge
	.dw typing_word_thrust
	.dw typing_word_clash
	.dw typing_word_assault
	.dw typing_word_advance
	.dw typing_word_rampage
	.dw typing_word_crush
	.dw typing_word_onslaught
	.dw typing_word_devastate
	.dw typing_word_overpower
	.dw typing_word_annihilate

typing_word_blow:	.db "BLOW",0
typing_word_rush:	.db "RUSH",0
typing_word_slam:	.db "SLAM",0
typing_word_stab:	.db "STAB",0
typing_word_strike:	.db "STRIKE",0
typing_word_charge:	.db "CHARGE",0
typing_word_thrust:	.db "THRUST",0
typing_word_clash:	.db "CLASH",0
typing_word_assault:	.db "ASSAULT",0
typing_word_advance:	.db "ADVANCE",0
typing_word_rampage:	.db "RAMPAGE",0
typing_word_crush:	.db "CRUSH",0
typing_word_onslaught:	.db "ONSLAUGHT",0
typing_word_devastate:	.db "DEVASTATE",0
typing_word_overpower:	.db "OVERPOWER",0
typing_word_annihilate:	.db "ANNIHILATE",0

msg_no_potion:
	.db "No potion",0
msg_no_smoke:
	.db "No smoke",0
msg_full_hp:
	.db "HP full",0
msg_battle_intro:
	.db "attacks!",0
msg_victory:
	.db "Victory!",0
msg_level_up:
	.db "LVL UP",0
msg_damage_dealt:
	.db "Hit",0
msg_damage_taken:
	.db "Took",0
msg_gold_gain:
	.db "Gold +",0
msg_game_over:
	.db "GAME OVER",0

enemy_name_slime:	.db "SLIME",0
enemy_name_fantome:	.db "GHOST",0
enemy_name_araignee:	.db "SPIDER",0
enemy_name_bandit:	.db "BANDIT",0
enemy_name_squelette:	.db "SKELETON",0
enemy_name_demon:	.db "DEMON",0
enemy_name_sorcier:	.db "SORCERER",0

msg_no_gold:
	.db "Need gold",0
msg_upgrade_owned:
	.db "Already better",0
msg_village:
	.db "Village",0
msg_castle:
	.db "Castle",0
msg_shop:
	.db "Shop",0
msg_inn:
	.db "Inn",0
msg_inn_offer:
	.db "Rest "
msg_inn_offer_price:
	.db "20"
	.db "G?",0
msg_status_title:
	.db "Lv.",0
msg_status_hp:
	.db "HP",0
msg_status_pot:
	.db "Pot.",0
msg_status_smoke:
	.db "Smk.",0
msg_status_sword:
	.db "Bld.",0
msg_status_shield:
	.db "Shld.",0
msg_gold:
	.db "G:",0
msg_potions:
	.db "Pot",0
msg_smokes:
	.db "Smk",0
msg_fin:
	.db "END",0

intro_text:
	.db "The king says:",13
	.db "- Sorcerer at",13
	.db "graveyard.",13
	.db "Go now.",0

castle_king_msg_table:
	.dw msg_king_1
	.dw msg_king_2
	.dw msg_king_3
	.dw msg_king_4
	.dw msg_king_4

msg_king_1:
	.db "Go forest.",0
msg_king_2:
	.db "Go ruins.",0
msg_king_3:
	.db "Go oasis.",0
msg_king_4:
	.db "Go black",13,"tower.",0

msg_village_0:
	.db "Ruins east.",13,"Oasis east.",0
msg_village_1:
	.db "Tower SE.",0

	ELSE
menu_battle:	; number of options, jump addresses, coordinates (X, Y), zero-terminated string
	.db 3
	.dw battle_attack
	.dw battle_show_objects_menu
	.dw battle_flee
	.db 5,4,"Atq",0
	.db 10,4,"Obj",0
	.db 15,4,"Fuir",0

menu_objects:
	.db 2
	.dw battle_use_potion
	.dw battle_use_smoke
	.db 4,3,"Potion("
menu_obj_potion_count:
	.db "0","0",")",0
	.db 4,4,"Fum",CHAR_E_ACUTE,"e("
menu_obj_smoke_count:
	.db "0","0",")",0

menu_village:
	.db 3
	.dw village_talk
	.dw village_rest
	.dw village_shop_menu_enter
	.db 3,2,"Parler",0
	.db 3,3,"Auberge",0
	.db 3,4,"Magasin",0

menu_village_shop:
	.db 4
	.dw shop_buy_potion
	.dw shop_buy_smoke
	.dw shop_buy_blade
	.dw shop_buy_armor
	.db 2,1,"Potion 22o",0
	.db 2,2,"Fum",CHAR_E_ACUTE,"e 32o",0
	.db 2,3,CHAR_E_ACUTE,"p",CHAR_E_ACUTE,"e+3 144o",0
	.db 2,4,"Arm.+3 144o",0

menu_castle:
	.db 3
	.dw castle_palace
	.dw castle_inn_menu_enter
	.dw castle_shop_menu_enter
	.db 3,1,"Roi",0
	.db 3,2,"Auberge",0
	.db 3,3,"Magasin",0

menu_castle_inn:
	.db 2
	.dw castle_inn_yes
	.dw inn_reenter
	.db 3,3,"Oui",0
	.db 3,4,"Non",0

menu_castle_shop:
	.db 4
	.dw shop_buy_potion
	.dw shop_buy_smoke
	.dw shop_buy_blade
	.dw shop_buy_armor
	.db 2,1,"Potion 14o",0
	.db 2,2,"Fum",CHAR_E_ACUTE,"e 22o",0
	.db 2,3,CHAR_E_ACUTE,"p",CHAR_E_ACUTE,"e+1 48o",0
	.db 2,4,"Arm.+1 48o",0

typing_word_ptrs:
	.dw typing_word_coup
	.dw typing_word_choc
	.dw typing_word_rage
	.dw typing_word_elan
	.dw typing_word_assaut
	.dw typing_word_charge
	.dw typing_word_frappe
	.dw typing_word_ruade
	.dw typing_word_attaque
	.dw typing_word_bagarre
	.dw typing_word_poussee
	.dw typing_word_percee
	.dw typing_word_offensive
	.dw typing_word_bousculade
	.dw typing_word_massacrer
	.dw typing_word_fulgurant

typing_word_coup:	.db "COUP",0
typing_word_choc:	.db "CHOC",0
typing_word_rage:	.db "RAGE",0
typing_word_elan:	.db "ELAN",0
typing_word_assaut:	.db "ASSAUT",0
typing_word_charge:	.db "CHARGE",0
typing_word_frappe:	.db "FRAPPE",0
typing_word_ruade:	.db "RUADE",0
typing_word_attaque:	.db "ATTAQUE",0
typing_word_bagarre:	.db "BAGARRE",0
typing_word_poussee:	.db "POUSSEE",0
typing_word_percee:	.db "PERCEE",0
typing_word_offensive:	.db "OFFENSIVE",0
typing_word_bousculade:	.db "BOUSCULADE",0
typing_word_massacrer:	.db "MASSACRER",0
typing_word_fulgurant:	.db "FULGURANT",0

msg_no_potion:
	.db "Potion vide",0
msg_no_smoke:
	.db "Fum",CHAR_E_ACUTE,"e:0",0
msg_full_hp:
	.db "PV max",0
msg_battle_intro:
	.db "attaque!",0
msg_victory:
	.db "Victoire!",0
msg_level_up:
	.db "NIVEAU+",0
msg_damage_dealt:
	.db "Infl.",0
msg_damage_taken:
	.db "Recu",0
msg_gold_gain:
	.db "Or +",0
msg_game_over:
	.db "GAME OVER",0

enemy_name_slime:	.db "SLIME",0
enemy_name_fantome:	.db "FANTOME",0
enemy_name_araignee:	.db "ARAIGNEE",0
enemy_name_bandit:	.db "BANDIT",0
enemy_name_squelette:	.db "SQUELETTE",0
enemy_name_demon:	.db "DEMON",0
enemy_name_sorcier:	.db "SORCIER",0

msg_no_gold:
	.db "Or manque",0
msg_upgrade_owned:
	.db "D",CHAR_E_ACUTE,"j",CHAR_A_GRAVE," ",CHAR_E_ACUTE,"quip",CHAR_E_ACUTE,0
msg_village:
	.db "Village",0
msg_castle:
	.db "Ch",CHAR_A_CIRC,"teau",0
msg_shop:
	.db "Magasin",0
msg_inn:
	.db "Auberge",0
msg_inn_offer:
	.db "Repos "
msg_inn_offer_price:
	.db "20"
	.db " or?",0
msg_status_title:
	.db "Niv.",0
msg_status_hp:
	.db "Pv",0
msg_status_pot:
	.db "Pot.",0
msg_status_smoke:
	.db "Fum.",0
msg_status_sword:
	.db CHAR_E_ACUTE,"p.",0
msg_status_shield:
	.db "Bouc.",0
msg_gold:
	.db "Or:",0
msg_potions:
	.db "Pot",0
msg_smokes:
	.db "Fum",0
msg_fin:
	.db "FIN",0

intro_text:
	.db "Le roi dit:",13
	.db "- Sorcier au",13
	.db "cimeti",CHAR_E_GRAVE,"re.",13
	.db "Va voir.",0

castle_king_msg_table:
	.dw msg_king_1
	.dw msg_king_2
	.dw msg_king_3
	.dw msg_king_4
	.dw msg_king_4

msg_king_1:
	.db "Va for",CHAR_E_CIRC,"t.",0
msg_king_2:
	.db "Va ruines.",0
msg_king_3:
	.db "Va oasis.",0
msg_king_4:
	.db "Va tour",13,"noire.",0

msg_village_0:
	.db "Ruines est.",13,"Oasis est.",0
msg_village_1:
	.db "Tour SE.",0

	ENDIF
