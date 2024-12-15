
# Tasks

The mechanics and behavior of the Task system are well-documented, with a wiki article in the main `pokeemerald` repo. What's less well documented, in my opinion, are the *semantics* of the system.

We know that we can create tasks. We know that each task gets its own frame handler and a small buffer to store its state in. However, tasks can be blown away en masse via a call to `ResetTasks`, and this is done throughout the codebase. Tasks have no destructors, so a task that gets killed by code running elsewhere will fail to tear down its own state (including leaking any heap-allocated memory). It's important to make sure that your tasks tear themselves down instead, so... how do you do that? When does it make sense for tasks to be reset? How should we think and reason about the "lifetime" of a task?

It's possible that there *are* no semantics to tasks -- that Game Freak built a low-level primitive and then used it willy-nilly -- but in order to find out, we have to look at how tasks are used. So let's look.

## APIs and flexibility

The following APIs are worth knowing about for tasks, if only to understand the usage patterns involved.

* **`CreateTask(id)` and `DestroyTask(id)`:** Self-explanatory. One thing worth keeping in mind, when reading the research below, is that when a task destroys itself, its call stack isn't interrupted; and so a task can do things (e.g. spawn its own replacement) in the moments after it has destroyed itself.
* **`ResetTasks()`:** Destroys all extant tasks.
* **`FuncIsActiveTask(fp)`:** Check whether there exists a running task whose handler function is the function pointer `fp`.
* **`FindTaskIdByFunc(fp)`:** Search for any running task whose handler function is the function pointer `fp`; return the ID of the found task, if any, or `TASK_NONE` otherwise.
* **`SetTaskFuncWithFollowupFunc` and `SwitchTaskToFollowupFunc`:** These are convenience functions that co-opt some of a task's data buffer to store a function pointer. They exist to allow more generic swapping of a task's handler function: "Do X, and then do Y afterward," where "X" doesn't have to know or care what the follow-up function "Y" is.

## Behaviors seen

### Handler swapping

It's common for Game Freak to create a task which initially has one handler function, but which replaces its handler function during its operation. There are two patterns:

* A series of sequential operations which must be performed over time (e.g. the game startup cutscene). Each step in the sequence is given its own handler function, and upon completion of a given step, the task switches to the next step's handler. At the end of the sequence, the task will generally destroy itself, relying on CB2 to move onto whatever comes next.

* A single task used as the main loop for a menu or similar system, where, when the system is idle, the task's handler function waits for input. When input is received, the handler function dispatches to functions which react to those inputs by switching the task's handler. This has the effect of allowing the menu to respond to player inputs over time (e.g. by playing sound effects, animating the menu, et cetera) while blocking input until the menu has finished responding.

### Subordinate tasks

It's common for Game Freak to use separate tasks for minor effects, such as individual animations in the game's startup cutscene, or ancillary animations (e.g. scroll indicators bobbing up and down) within menus. It's less common for Game Freak to have one subordinate task spawn its own subordinate task.

#### Finite subordinates

In some cases, subordinate tasks are designed to run for a finite time and destroy themselves, with the timing arranged to ensure that they're dead by the time the primary task needs to move on; this is the case for the game's startup cutscene, where subordinate tasks used for graphics always destroy themselves, and always in time for their graphics to be unloaded and replaced.

#### Communication with subordinates

In other cases, subordinate tasks are puppeteered by the primary task, which remembers the IDs of the subordinate tasks in order to access the subordinate tasks' buffers and read or write data. Since this requires keeping track of the subordinate task's ID, a parent or ancestor task will generally use that ID to terminate the subordinate task via `DestroyTask` as well.

#### Self-terminating subordinates

In still other cases, a subordinate task nay check whether its parent task is still running, and destroy itself if not. One known example involves graphics-related tasks in the evolution cutscene, where the primary task may look up and terminate all subordinate tasks, or a nested-subordinate task may self-terminate if its parent has stopped running on its own.

This pattern is easier to use when dealing with tasks that never swap out their handlers, as you can then use `FuncIsActiveTask` and `FindTaskIdByFunc` instead of having to store task IDs. In essence, the creation of the subordinate task becomes fire-and-forget: the parent task doesn't have to keep track of the subordinate task and explicitly terminate it, because it can just terminate itself when it sees that its parent is gone. If the top-level task needs to instantly terminate all child and descendant tasks (i.e. when the top-level task itself is exiting), it can look them up by their functions (and this, too, happens in the evolution cutscene).

## Places where tasks are used

This list is not exhaustive

* Bag menu
  * If and only if the player is *not* in an active link with someone (`!MenuHelpers_IsLinkActive()`), tasks are reset on entry by a setup function that runs latently via CB2. A primary task is created to handle input, though creation of the task is abstracted (to account for the Wally Pokemon-catching cutscene, wherein Wally operates the bag rather than the player). The primary task will swap out its handler using follow-up functions, in order to respond to input.
* Battle engine
  * Tasks are used to send and receive data for battle controllers.
* Berry tag screen
  * **Summary:** A single task is used to power the menu, and the task changes its handler to perform menu operations.
  * This is the UI used to display information about berries.
  
    If and only if the player is *not* in an active link with someone (`!MenuHelpers_IsLinkActive()`), tasks are reset on entry by a setup function that runs latently via CB2. A task is created to serve as the menu's primary frame handler. When the menu is idle, the task handler is `Task_HandleInput`; the task will briefly replace its handler in response to input, in order to perform various operations latently while ignoring input until those operations are complete. The menu never creates any other tasks.
* Cable car cutscene
  * A self-terminating task is used as the entry point to the cutscene; this task sets CB2. The CB2 handler resets tasks and then creates a primary task to serve as the cutscene's frame handler, and a subordinate task to animate the background. When the cutscene ends, it resets tasks again, in order to terminate both the primary and subordinate tasks together.
* Dodrio berry picking minigame
  * **Summary:** Tasks are used first for latent setup functions, and then as the minigame's primary frame handler. An API to display high scores from overworld scripts is also exposed, and this relies on a task.
  * The minigame's entry point sets CB2 to a barebones handler, resets all tasks (and sprites), and then uses tasks for latent functions. These tasks don't generally replace their own callback pointers; rather, they destroy themselves and create replacement tasks (which has the benefit of zeroing out the replacement task's buffer, and the effect of palcing the new task at the end of the task queue). When setup is complete, a task is spawned to serve as the minigame's core frame handler: this task uses a state variable to index into a function table, in order to decide what to do on each frame. One function in this table is responsible for exiting the minigame: at completion, it has the task destroy itself, and then sets CB2 to the "exit callback" that was provided at the minigame's entry point.
  * Separately from the above, the minigame exposes an API for displaying its stored high scores. This API relies on a task (which destroys itself on exit), and is designed to be invoked from scripts. It is expected that the calling script will run the `waitstate` opcode after invoking this API, as the high score display resumes all waiting scripts on exit.
* Evolution cutscene
  * A task is used to start the cutscene by fading to black; this task destroys itself and invokes a setup function that resets tasks. Thereafter, a primary task acts as the frame handler for the overall animation; this primary task spawns a subordinate task to handle animating the background graphics. All tasks used are responsible for destroying themselves on completion, with the primary task using CB2 to advance to whatever comes after an evolution attempt.
  
    The `FindTaskIdByFunc` function is used in some cases to find and destroy any extant tasks related to the evolution cutscene graphics. Additionally, the `Task_UpdateBgPalette` subordinate task can spawn its own subordinate, and the subordinate uses `FuncIsActiveTask` to destroy itself when its parent is no longer running.
* Game startup intro (`intro.c`)
  * This is the intro seen when you start up the game: a copyright notice, animations leading into the game's logo, and scenes of the playable characters and legendaries.
    
    Tasks are reset while prepping the copyright notice for display, by a setup function that runs latently via CB2. The copyright screen is then displayed using a task. Once this task runs to completion, it replaces its own callback with a handler for the next phase of display (fading in the game's startup animation). Certain phases of the intro create secondary tasks to control individual animations (e.g. sparkles, fading of the game's logo, scrolling of parallax backgrounds), with these tasks destroying themselves on completion. When we finally reach the end of the intro cutscene (`Task_EndIntroMovie`), the primary task destroys itself, and the game uses CB2 to proceed to the title screen.
* Overworld
  * The overworld resets tasks when changing the current map (in `CB2_DoChangeMap`, defined, confusingly, in `fldeff_flash.c` despite its sole caller being `CB2_LoadMap` in `overworld.c`).
  * Certain tasks are set up (via `SetUpFieldTasks()`) every time you return to the field (outside of multiplayer link situations):
    * Muddly slope behaviors
    * Per-tile effects (e.g. cracked floors and ice)
    * `Task_RunTimeBasedEvents`, which calls into the system for updating certain events per-day and per-minute, and also handles ambient Pokemon cries in the environment.
  * Tasks are used for all "warp" functions (i.e. `Task_WarpAndLoadMap`), and for many (all?) warp-related animations.
  * Tasks are used for Dig, Escape Rope, and Sweet Scent.
  * Tasks are used to display "field message boxes," and to hide Pokenav call message boxes once they've finished printing (see `field_message_box.c`).
  * Tasks are used to display map name popup boxes.
  * Taking a seat in the Cable Club, for Record Mixing, involves a task, and is triggered by scripts.
  * Tasks are used to animate players in the Union Room.
  * The Battle Dome uses a task (`CreateTask_PlayMapChosenOrBattleBGM` defined in `pokemon.c`) to play battle background music.
  * Tasks power the `waitweather` script command: the task polls for when the weather has finished changing, and then resumes the script.
    * What happens if a script uses `waitweather` and then a map change happens, killing the task that would've woken the script back up?
  * Tasks are used to freeze the player when she stops moving, as part of the `lock` and `lockall` script commands. The tasks wait for the player to stop moving, freeze her, and destroy themselves. These script commands also set up a "native script" (read: C code running in a script context) which waits for the tasks to no longer be running, and then stops the player's movement.
  * Tasks power scripted fanfares, scripted movement, and scripted menus e.g. Yes/No.
  * Tasks are used for cutscenes:
    * Using any field move (e.g. HMs)
    * Descending Mirage Tower
    * Pushing boulders
    * The collapse of Mirage Tower
    * The truck intro
    * The S.S. Tidal's porthole
    * Sealed Chamber screen shake (i.e. the Braille rooms)
  * Tasks are used for various field effects
    * Escalator tile animations
    * Poison
    * Secret Base
      * PC power on
      * Balloon popping
* Pokenav
  * **Summary:** Tasks are used as frame handlers, with an additional mechanism built on top to allow for synchronous execution within tasks (as opposed to latent execution, the default) and the coordination of a superior task (for the Pokenav's core infrastructure) and a subordinate task (for the active Pokenav submenu).
  * The Pokenav has two entry points: a normal one and a tutorial-specific one. Both entry points heap-allocate Pokenav data (and abort if the allocation fails) before then resetting tasks and sprites, and creating a core Pokenav task. The core task spawns subordinate "loop tasks," using a system built on top of the Task engine which automates some of the work needed to create a state machine: loop task handlers are run at least once per frame and return a status code indicating whether to increment the state variable, and whether to continue latently (i.e. run on the next frame), continue synchronously (i.e. run within a `while` loop), or exit (which destroys the task). The vast majority of processes occurring within the Pokenav use loop tasks, both for "backend" operations like loading data to display, and for "frontend" operations like basic UI interactions.
    
    The core task manages the overall Pokenav flow. It waits for the active submenu's loop task to cease to be active, and then switches to a new submenu, returns to the main Pokenav menu, or exits the Pokenav, as appropriate.
    
    The `PokenavCallbacks` struct is used to statically define all callback functions for a Pokenav submenu, including the function which spawns the submenu's loop task, and a function to check whether the submenu's loop task is active. Each submenu has a `PokenavCallbacks` instance.
* Reading mail
  * Tasks are reset on entry using a setup function that runs latently via CB2. Tasks are never created; instead, CB2 is used as the primary frame handler.
* Shop menu
  * Tasks are reset on entry using a setup function that runs latently via CB2. A primary task is created to serve as the menu's frame handler. Large amounts of the menu's functionality are delegated to a set of "list menu" helper functions used by several menus. When the player exits the menu, the primary task sets its handler to an "exit" function, which fades out the screen and then destroys the task.
    
    The menu also resets all sprites. It assumes responsibility for looping over all object events (actors) on the current map, setting their sprites back up, and drawing them.
* Union Room
  * Link battles (`union_room_battle.c`)
    * Tasks are reset on entry using a setup function that runs lately via CB2. The code here then yields control to the battle engine, and is returned to post-battle by way of `gMain.savedCallback`. Some of the code that we return to is shared between the Union Room and the Cable Club, and both appear to use tasks to some degree.

### Menus that only reset tasks conditionally

#### If no multiplayer link is active

* Bag menu (both normal and Battle Pyramid)
* Berry tag screen (accessed via the bag menu)
* Party menu

#### Other conditions

* PokeBlock case: only if not in a battle

## Conclusion

Tasks will typically be reset whenever a menu is opened, though menus that can be opened during a multiplayer link will refrain from doing so. Generally, tasks should be thought of as being scoped to menus/screens.

A basic menu that has a submenu should free any heap allocations when opening the submenu, given that the submenu will probably reset tasks. If a small amount of state needs to persist through the submenu, that state can be stored statically or globally. For example, the Bag menu stores the last-used bag pocket as a global variable, so that it can be remembered when you exit and reopen the bag, but also when you open and back out of a submenu. The Bag menu also heap-allocates much of the data it displays, and frees that data before opening a submenu and before exiting.

Some menus are more complex. For example, the Pokenav consists of a top-level menu with multiple submenus, and the top-level menu performs heap allocations. In this case, Game Freak built an additional system on top of tasks to coordinate the top-level menu and its active submenu, but &mdash; more importantly &mdash; they refrained from having the submenus reset tasks. This means that the Pokenav's top-level systems remain active &mdash; and their heap allocations reachable &mdash; through the opening and closing of submenus. It *is* worth noting that the Pokenav's submenus have a lot in common with one another and with the main menu, which means that it's unlikely that they'll exhaust the task pool; a "busier" menu or submenu may want to reset tasks.

On the overworld, tasks are scoped to the current map but should be considered ephemeral, since opening any menu (including menus that are overlaid on the overworld, such as the shop menu) may reset tasks. Stateful tasks should only be used in situations where player controls are locked and map transitions are impossible, such as overworld cutscenes, or UIs that capture or lock interaction (e.g. field message boxes).

On the overworld, tasks can be used as persistent frame handlers, to poll for specific events and react to them, if those tasks can cope with being spontaneously killed and restarted (with their state zeroed out); you'll need to update `SetUpFieldTasks` to (re)start those tasks. There *are* a limited number of task slots, though, so you would benefit from using one of the existing tasks/mechanisms  (e.g. per-step callbacks or time-based events) whenever that's feasible.