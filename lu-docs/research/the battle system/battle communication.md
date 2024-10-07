
## `gBattleCommunication`

Defined in `battle.h`, this is a raw buffer of size `BATTLE_COMMUNICATION_ENTRIES_COUNT`[^BATTLE_COMMUNICATION_ENTRIES_COUNT]. The defined indices are as follows:

<table>
   <thead>
      <tr>
         <th>Index</th>
         <th>Index constant[^gBattleCommunication_Indices]</th>
         <th>Description</th>
      </tr>
   </thead>
   <tbody>
      <tr>
         <td>0</td>
         <td><code>MULTIUSE_STATE</code></td>
         <td>
            <p>Various uses:</p>
            <ul>
               <li>
                  <p>The <code>CB2_HandleStartBattle</code> function is a frame handler designed as 
                  a state machine, in order to stagger battle-related setup tasks across multiple 
                  frames and prevent the game from becoming unresponsive. Even in non-link battles, 
                  <code>gBattleCommunication[MULTIUSE_STATE]</code> is used as the state variable.</p>
               </li>
            </ul>
         </td>
      </tr>
      <tr>
         <td>1</td>
         <td><code>CURSOR_POSITION</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>1</td>
         <td><code>TASK_ID</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>1</td>
         <td><code>SPRITES_INIT_STATE1</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>2</td>
         <td><code>SPRITES_INIT_STATE2</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>3</td>
         <td><code>MOVE_EFFECT_BYTE</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>4</td>
         <td><code>ACTIONS_CONFIRMED_COUNT</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>5</td>
         <td><code>MULTISTRING_CHOOSER</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>6</td>
         <td><code>MISS_TYPE</code></td>
         <td>...</td>
      </tr>
      <tr>
         <td>7</td>
         <td><code>MSG_DISPLAY</code></td>
         <td>...</td>
      </tr>
   </tbody>
</table>


[^BATTLE_COMMUNICATION_ENTRIES_COUNT]: `BATTLE_COMMUNICATION_ENTRIES_COUNT` == 8. It's defined in `include/constants/battle_script_commands.h`.

[^gBattleCommunication_Indices]: The names for these indices are defined in `include/constants/battle_script_commands.h`.