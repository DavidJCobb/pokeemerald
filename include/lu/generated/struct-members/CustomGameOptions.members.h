// generated

   bool8 start_with_running_shoes;
   bool8 can_run_indoors;
   bool8 can_bike_indoors;
   u16 scale_wild_encounter_rate;
   bool8 enable_catch_exp;
   u16 catch_rate_scale; // applies to final computed catch rate
   u8 catch_rate_increase_base; // percentage, where 100% = guaranteed catch
   u8 battle_item_usage; // Enabled, No Backfield, Disabled
   u16 scale_battle_damage_dealt_by_player;
   u16 scale_battle_damage_dealt_by_enemy;
   u16 scale_battle_damage_dealt_by_ally;
   u16 scale_battle_accuracy_player;
   u16 scale_battle_accuracy_enemy;
   u16 scale_battle_accuracy_ally;
   u16 scale_exp_gains_normal;
   u16 scale_exp_gains_traded;
   u16 scale_player_money_gain_on_victory; // Does not scale Pay Day gains, which are applied at the end of a victory and lost if you lose the battle.
   bool8 modern_calc_player_money_loss_on_defeat; // enabled: FR/LG and Gen IV+ calc; disabled: half your cash
   u8 overworld_poison_interval; // step count; 0 to disable
   u16 overworld_poison_damage; // damage taken per poison field effect
   bool8 no_physical_special_split; // TODO: implement the physical/special split first lol
   bool8 never_white_out_with_partner; // effect: when battling alongside Steven, you don't lose until your Pokemon AND HIS all faint
   bool8 no_inherently_typeless_moves; // effect: Beat Up, Future Sight, and Doom Desire are typed
   bool8 wonder_guard_blocks_typeless; // effect: Wonder Guard blocks typeless moves except Struggle
