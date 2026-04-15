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
typing_result		equ typing_bonus_limit+1
typing_last_key		equ typing_result+1
typing_tick_div		equ typing_last_key+1
typing_word_x		equ typing_tick_div+1
last_player_damage	equ typing_word_x+1
last_enemy_damage	equ last_player_damage+1
level_up_flag		equ last_enemy_damage+1
last_player_bonus	equ level_up_flag+1
