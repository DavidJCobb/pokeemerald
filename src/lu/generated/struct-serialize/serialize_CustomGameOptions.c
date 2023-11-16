#include "lu/generated/struct-serialize/serialize_CustomGameOptions.h"

#include "global.h"
#include "lu/custom_game_options.h" // struct definition

#include "lu/bitstreams.h"

void lu_BitstreamRead_CustomGameOptions(struct lu_BitstreamState* state, struct CustomGameOptions* v) {
   v->start_with_running_shoes = lu_BitstreamRead_bool(state);
   v->can_run_indoors = lu_BitstreamRead_bool(state);
   v->can_bike_indoors = lu_BitstreamRead_bool(state);
   v->scale_wild_encounter_rate = lu_BitstreamRead_u16(state, 13);
   v->enable_catch_exp = lu_BitstreamRead_bool(state);
   v->catch_rate_scale = lu_BitstreamRead_u16(state, 13);
   v->catch_rate_increase_base = lu_BitstreamRead_u8(state, 7);
   v->battle_item_usage = lu_BitstreamRead_u8(state, 3);
   v->scale_battle_damage_dealt_by_player = lu_BitstreamRead_u16(state, 13);
   v->scale_battle_damage_dealt_by_enemy = lu_BitstreamRead_u16(state, 13);
   v->scale_battle_damage_dealt_by_ally = lu_BitstreamRead_u16(state, 13);
   v->scale_battle_accuracy_player = lu_BitstreamRead_u16(state, 13);
   v->scale_battle_accuracy_enemy = lu_BitstreamRead_u16(state, 13);
   v->scale_battle_accuracy_ally = lu_BitstreamRead_u16(state, 13);
   v->scale_exp_gains_normal = lu_BitstreamRead_u16(state, 13);
   v->scale_exp_gains_traded = lu_BitstreamRead_u16(state, 13);
   v->scale_player_money_gain_on_victory = lu_BitstreamRead_u16(state, 13);
   v->modern_calc_player_money_loss_on_defeat = lu_BitstreamRead_bool(state);
   v->overworld_poison_interval = lu_BitstreamRead_u8(state, 6);
   v->overworld_poison_damage = lu_BitstreamRead_u16(state, 11) + 1;
   v->no_physical_special_split = lu_BitstreamRead_bool(state);
   v->never_white_out_with_partner = lu_BitstreamRead_bool(state);
   v->no_inherently_typeless_moves = lu_BitstreamRead_bool(state);
   v->wonder_guard_blocks_typeless = lu_BitstreamRead_bool(state);
}

void lu_BitstreamWrite_CustomGameOptions(struct lu_BitstreamState* state, const struct CustomGameOptions* v) {
   lu_BitstreamWrite_bool(state, v->start_with_running_shoes);
   lu_BitstreamWrite_bool(state, v->can_run_indoors);
   lu_BitstreamWrite_bool(state, v->can_bike_indoors);
   lu_BitstreamWrite_u16(state, v->scale_wild_encounter_rate, 13);
   lu_BitstreamWrite_bool(state, v->enable_catch_exp);
   lu_BitstreamWrite_u16(state, v->catch_rate_scale, 13);
   lu_BitstreamWrite_u8(state, v->catch_rate_increase_base, 7);
   lu_BitstreamWrite_u8(state, v->battle_item_usage, 3);
   lu_BitstreamWrite_u16(state, v->scale_battle_damage_dealt_by_player, 13);
   lu_BitstreamWrite_u16(state, v->scale_battle_damage_dealt_by_enemy, 13);
   lu_BitstreamWrite_u16(state, v->scale_battle_damage_dealt_by_ally, 13);
   lu_BitstreamWrite_u16(state, v->scale_battle_accuracy_player, 13);
   lu_BitstreamWrite_u16(state, v->scale_battle_accuracy_enemy, 13);
   lu_BitstreamWrite_u16(state, v->scale_battle_accuracy_ally, 13);
   lu_BitstreamWrite_u16(state, v->scale_exp_gains_normal, 13);
   lu_BitstreamWrite_u16(state, v->scale_exp_gains_traded, 13);
   lu_BitstreamWrite_u16(state, v->scale_player_money_gain_on_victory, 13);
   lu_BitstreamWrite_bool(state, v->modern_calc_player_money_loss_on_defeat);
   lu_BitstreamWrite_u8(state, v->overworld_poison_interval, 6);
   lu_BitstreamWrite_u16(state, v->overworld_poison_damage - 1, 11);
   lu_BitstreamWrite_bool(state, v->no_physical_special_split);
   lu_BitstreamWrite_bool(state, v->never_white_out_with_partner);
   lu_BitstreamWrite_bool(state, v->no_inherently_typeless_moves);
   lu_BitstreamWrite_bool(state, v->wonder_guard_blocks_typeless);
}
