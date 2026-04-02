; ------------------------------------------------------------
; Player and enemy statistics
; ------------------------------------------------------------

player_level		equ save_player_gold+2
player_exp		equ player_level+1
player_gold		equ player_exp+2

player_hp_cur		equ player_gold+2
player_hp_max		equ player_hp_cur+1
player_atk		equ player_hp_max+1
player_def		equ player_atk+1
player_flee		equ player_def+1
player_atk_bonus	equ player_flee+1
player_def_bonus	equ player_atk_bonus+1

inventory_potions	equ player_def_bonus+1
inventory_smokes	equ inventory_potions+1

battle_enemy_tile	equ inventory_smokes+1
battle_enemy_hp_cur	equ battle_enemy_tile+1
battle_enemy_hp_max	equ battle_enemy_hp_cur+1
battle_enemy_atk		equ battle_enemy_hp_max+1
battle_enemy_def		equ battle_enemy_atk+1
battle_enemy_exp		equ battle_enemy_def+1
battle_enemy_gold		equ battle_enemy_exp+1
battle_enemy_name_ptr	equ battle_enemy_gold+1
battle_event_flag_mask	equ battle_enemy_name_ptr+2
battle_outcome		equ battle_event_flag_mask+1

typing_word_ptr		equ battle_outcome+1
typing_word_len		equ typing_word_ptr+2
typing_index		equ typing_word_len+1
typing_timeout		equ typing_index+1
typing_bonus_limit	equ typing_timeout+1
typing_early_limit	equ typing_bonus_limit+1
typing_result		equ typing_early_limit+1
typing_last_key		equ typing_result+1
typing_tick_div		equ typing_last_key+1
typing_word_x		equ typing_tick_div+1
last_player_damage	equ typing_word_x+1
last_enemy_damage	equ last_player_damage+1
level_up_flag		equ last_enemy_damage+1
last_player_bonus	equ level_up_flag+1

enemy_ptr_table:
	.dw enemy_slime
	.dw enemy_fantome
	.dw enemy_araignee
	.dw enemy_bandit_ruins
	.dw enemy_bandit_tower
	.dw enemy_squelette
	.dw enemy_bandit_boss
	.dw enemy_demon_ruins
	.dw enemy_demon_oasis
	.dw enemy_sorcier

enemy_slime:
	.db 16,7,1,1,4,4
	.dw enemy_name_slime

enemy_fantome:
	.db 17,13,3,2,6,6
	.dw enemy_name_fantome

enemy_araignee:
	.db 19,18,4,2,8,8
	.dw enemy_name_araignee

enemy_bandit_ruins:
	.db 20,28,6,3,12,12
	.dw enemy_name_bandit

enemy_bandit_tower:
	.db 20,40,8,4,16,16
	.dw enemy_name_bandit

enemy_squelette:
	.db 18,20,8,3,24,24
	.dw enemy_name_squelette

enemy_bandit_boss:
	.db 20,56,11,5,40,40
	.dw enemy_name_bandit

enemy_demon_ruins:
	.db 21,80,13,6,64,60
	.dw enemy_name_demon

enemy_demon_oasis:
	.db 21,106,16,8,88,80
	.dw enemy_name_demon

enemy_sorcier:
	.db 22,140,18,10,128,120
	.dw enemy_name_sorcier
