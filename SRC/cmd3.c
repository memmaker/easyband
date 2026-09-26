/* File: cmd3.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"






/*
 * Display inventory
 */
void do_cmd_inven(void)
{
	/* RVIP: item menu (the stores keep the plain list) */
	if (!character_icky) { inven_screen(FALSE); return; }

	/* Hack -- Start in "inventory" mode */
	p_ptr->command_wrk = (USE_INVEN);

	/* Save screen */
	screen_save();

	/* Hack -- show empty slots */
	item_tester_full = TRUE;

	/* Display the inventory */
	show_inven();

	/* Hack -- hide empty slots */
	item_tester_full = FALSE;

	/* Prompt for a command */
	prt("(Inventory) Command: ", 0, 0);

	/* Hack -- Get a new command */
	p_ptr->command_new = inkey();

	/* Load screen */
	screen_load();


	/* Hack -- Process "Escape" */
	if (p_ptr->command_new == ESCAPE)
	{
		/* Reset stuff */
		p_ptr->command_new = 0;
	}

	/* Hack -- Process normal keys */
	else
	{
		/* Hack -- Use "display" mode */
		p_ptr->command_see = TRUE;
	}
}


/*
 * Display equipment
 */
void do_cmd_equip(void)
{
	/* RVIP: item menu (the stores keep the plain list) */
	if (!character_icky) { inven_screen(TRUE); return; }

	/* Hack -- Start in "equipment" mode */
	p_ptr->command_wrk = (USE_EQUIP);

	/* Save screen */
	screen_save();

	/* Hack -- show empty slots */
	item_tester_full = TRUE;

	/* Display the equipment */
	show_equip();

	/* Hack -- undo the hack above */
	item_tester_full = FALSE;

	/* Prompt for a command */
	prt("(Equipment) Command: ", 0, 0);

	/* Hack -- Get a new command */
	p_ptr->command_new = inkey();

	/* Load screen */
	screen_load();


	/* Hack -- Process "Escape" */
	if (p_ptr->command_new == ESCAPE)
	{
		/* Reset stuff */
		p_ptr->command_new = 0;
	}

	/* Hack -- Process normal keys */
	else
	{
		/* Enter "display" mode */
		p_ptr->command_see = TRUE;
	}
}


/*
 * The "wearable" tester
 */
static bool item_tester_hook_wear(object_type *o_ptr)
{
	/* Check for a usable slot */
	if (wield_slot(o_ptr) >= INVEN_WIELD) return (TRUE);

	/* Assume not wearable */
	return (FALSE);
}


/*
 * Wield or wear a single item from the pack or floor
 */
void do_cmd_wield(void)
{
	int item, slot;

	object_type *o_ptr;
	object_type *i_ptr;
        object_type *q_ptr;
	object_type object_type_body;

	cptr act;

	cptr q, s;

        u32b f1, f2, f3;

	char o_name[80];


	/* Restrict the choices */
	item_tester_hook = item_tester_hook_wear;

	/* Get an item */
	q = "Wear/Wield which item? ";
	s = "You have nothing you can wear or wield.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Check the slot */
	slot = wield_slot(o_ptr);

	/* Prevent wielding into a cursed slot */
	if (cursed_p(&inventory[slot]))
	{
		/* Describe it */
		object_desc(o_name, &inventory[slot], FALSE, 0);

		/* Message */
		msg_format("The %s you are %s appears to be cursed.",
		           o_name, describe_use(slot));

		/* Cancel the command */
		return;
	}


        /* Extract the flags */
        object_flags(o_ptr, &f1, &f2, &f3);

        /* Two handed weapons can't be wielded with a shield */

        if ((inventory[INVEN_ARM].k_idx != 0) && (f1 & TR1_MUST2H) && (p_ptr->pclass!=CLASS_WARRIOR))
        {
        object_desc(o_name, o_ptr, FALSE, 0);
        msg_format("You cannot wield your %s with a shield.", o_name);
        
        return;
}

        i_ptr = &inventory[INVEN_WIELD];

        /* Extract the flags */
        object_flags(i_ptr, &f1, &f2, &f3);

        /* Prevent shield from being put on if wielding 2H */

        if ((slot == INVEN_ARM) && (f1 & TR1_MUST2H) && (p_ptr->pclass!=CLASS_WARRIOR))
        {
          object_desc(o_name, o_ptr, FALSE, 0);
           msg_format("You cannot wield your %s with a two-handed weapon.", o_name);
          
           return;
        
        }

        if ((slot == INVEN_ARM) && (f1 & TR1_COULD2H))
        {
           if (!get_check("Are you sure you want to restrict your fighting? "))
            return;
        }


	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Get local object */
	q_ptr = &object_type_body;

	/* Obtain local object */
	object_copy(q_ptr, o_ptr);

	/* Modify quantity */
	q_ptr->number = 1;

	/* Decrease the item (from the pack) */
	if (item >= 0)
	{
		inven_item_increase(item, -1);
		inven_item_optimize(item);
	}

	/* Decrease the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_optimize(0 - item);
	}

	/* Get the wield slot */
	o_ptr = &inventory[slot];

	/* Take off existing item */
	if (o_ptr->k_idx)
	{
		/* Take off existing item */
		(void)inven_takeoff(slot, 255);
	}

	/* Wear the new stuff */
	object_copy(o_ptr, q_ptr);

	/* Increase the weight */
	p_ptr->total_weight += q_ptr->weight;

	/* Increment the equip counter by hand */
	p_ptr->equip_cnt++;

	/* Where is the item now */
	if (slot == INVEN_WIELD)
	{
		act = "You are wielding";
	}
	else if (slot == INVEN_BOW)
	{
		act = "You are shooting with";
	}
	else if (slot == INVEN_LITE)
	{
		act = "Your light source is";
	}
	else
	{
		act = "You are wearing";
	}

	/* Describe the result */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Message */
	msg_format("%s %s (%c).", act, o_name, index_to_label(slot));

	/* Cursed! */
	if (cursed_p(o_ptr))
	{
		/* Warn the player */
		msg_print("Oops! It feels deathly cold!");

		/* Remove special inscription, if any */
		if (o_ptr->discount >= INSCRIP_NULL) o_ptr->discount = 0;

		/* Sense the object if allowed */
		if (o_ptr->discount == 0) o_ptr->discount = INSCRIP_CURSED;

		/* The object has been "sensed" */
		o_ptr->ident |= (IDENT_SENSE);
	}

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);

	/* Recalculate mana */
	p_ptr->update |= (PU_MANA);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);
}



/*
 * Take off an item
 */
void do_cmd_takeoff(void)
{
	int item;

	object_type *o_ptr;

	cptr q, s;


	/* Get an item */
	q = "Take off which item? ";
	s = "You are not wearing anything to take off.";
	if (!get_item(&item, q, s, (USE_EQUIP))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Item is cursed */
	if (cursed_p(o_ptr))
	{
		/* Oops */
		msg_print("Hmmm, it seems to be cursed.");

		/* Nope */
		return;
	}


	/* Take a partial turn */
	p_ptr->energy_use = 50;

	/* Take off the item */
	(void)inven_takeoff(item, 255);
}


/*
 * Drop an item
 */
void do_cmd_drop(void)
{
	int item, amt;

	object_type *o_ptr;

	cptr q, s;


	/* Get an item */
	q = "Drop which item? ";
	s = "You have nothing to drop.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Get a quantity */
	amt = get_quantity(NULL, o_ptr->number);

	/* Allow user abort */
	if (amt <= 0) return;

	/* Hack -- Cannot remove cursed items */
	if ((item >= INVEN_WIELD) && cursed_p(o_ptr))
	{
		/* Oops */
		msg_print("Hmmm, it seems to be cursed.");

		/* Nope */
		return;
	}

	/* Take a partial turn */
	p_ptr->energy_use = 50;

	/* Drop (some of) the item */
	inven_drop(item, amt);
}



/*
 * Destroy an item
 */
void do_cmd_destroy(void)
{
	int item, amt;
	int old_number;

	object_type *o_ptr;

	char o_name[80];

	char out_val[160];

	cptr q, s;


	/* Get an item */
	q = "Destroy which item? ";
	s = "You have nothing to destroy.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Get a quantity */
	amt = get_quantity(NULL, o_ptr->number);

	/* Allow user abort */
	if (amt <= 0) return;

	/* Describe the object */
	old_number = o_ptr->number;
	o_ptr->number = amt;
	object_desc(o_name, o_ptr, TRUE, 3);
	o_ptr->number = old_number;

	/* Verify destruction */
	if (verify_destroy)
	{
		sprintf(out_val, "Really destroy %s? ", o_name);
		if (!get_check(out_val)) return;
	}

	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Artifacts cannot be destroyed */
	if (artifact_p(o_ptr))
	{
		/* Message */
		msg_format("You cannot destroy %s.", o_name);

		/* Don't mark id'ed objects */
		if (object_known_p(o_ptr)) return;

		/* It has already been sensed */
		if (o_ptr->ident & (IDENT_SENSE))
		{
			/* Already sensed objects always get improved feelings */
			if (cursed_p(o_ptr) || broken_p(o_ptr))
				o_ptr->discount = INSCRIP_TERRIBLE;
			else
				o_ptr->discount = INSCRIP_SPECIAL;
		}
		else
		{
			/* Mark the object as indestructible */
			o_ptr->discount = INSCRIP_INDESTRUCTIBLE;
		}

		/* Combine the pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);

		/* Done */
		return;
	}

	/* Message */
	msg_format("You destroy %s.", o_name);

	/* Eliminate the item (from the pack) */
	if (item >= 0)
	{
		inven_item_increase(item, -amt);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Eliminate the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -amt);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}
}


/*
 * Observe an item which has been *identify*-ed
 */
void do_cmd_observe(void)
{
	int item;

	object_type *o_ptr;

	char o_name[80];

	cptr q, s;


	/* Get an item */
	q = "Examine which item? ";
	s = "You have nothing to examine.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Require full knowledge */ 
	//if (!(o_ptr->ident & (IDENT_MENTAL))) hugly hack, think to remove .... mmm......... bhh
	
	//{
	//	msg_print("You have no special knowledge about that item.");
	//	return;
//	}


	/* Description */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Describe */
	msg_format("Examining %s...", o_name);

	/* Describe it fully */
	if (!identify_fully_aux(o_ptr)) msg_print("You see nothing special.");
}



/*
 * Remove the inscription from an object
 * XXX Mention item (when done)?
 */
void do_cmd_uninscribe(void)
{
	int item;

	object_type *o_ptr;

	cptr q, s;


	/* Get an item */
	q = "Un-inscribe which item? ";
	s = "You have nothing to un-inscribe.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Nothing to remove */
	if (!o_ptr->note)
	{
		msg_print("That item had no inscription to remove.");
		return;
	}

	/* Message */
	msg_print("Inscription removed.");

	/* Remove the incription */
	o_ptr->note = 0;

	/* Combine the pack */
	p_ptr->notice |= (PN_COMBINE);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);
}


/*
 * Inscribe an object with a comment
 */
void do_cmd_inscribe(void)
{
	int item;

	object_type *o_ptr;

	char o_name[80];

	char tmp[80];

	cptr q, s;


	/* Get an item */
	q = "Inscribe which item? ";
	s = "You have nothing to inscribe.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Describe the activity */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Message */
	msg_format("Inscribing %s.", o_name);
	msg_print(NULL);

	/* Start with nothing */
	strcpy(tmp, "");

	/* Use old inscription */
	if (o_ptr->note)
	{
		/* Start with the old inscription */
		strnfmt(tmp, 80, "%s", quark_str(o_ptr->note));
	}

	/* Get a new inscription (possibly empty) */
	if (get_string("Inscription: ", tmp, 80))
	{
		/* Save the inscription */
		o_ptr->note = quark_add(tmp);

		/* Combine the pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);
	}
}



/*
 * An "item_tester_hook" for refilling lanterns
 */
static bool item_tester_refill_lantern(object_type *o_ptr)
{
	/* Flasks of oil are okay */
	if (o_ptr->tval == TV_FLASK) return (TRUE);

	/* Non-empty lanterns are okay */
	if ((o_ptr->tval == TV_LITE) &&
	    (o_ptr->sval == SV_LITE_LANTERN) &&
	    (o_ptr->pval > 0))
	{
		return (TRUE);
	}

	/* Assume not okay */
	return (FALSE);
}


/*
 * Refill the players lamp (from the pack or floor)
 */
static void do_cmd_refill_lamp(void)
{
	int item;

	object_type *o_ptr;
	object_type *j_ptr;

	cptr q, s;


	/* Restrict the choices */
	item_tester_hook = item_tester_refill_lantern;

	/* Get an item */
	q = "Refill with which source of oil? ";
	s = "You have no sources of oil.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Take a partial turn */
	p_ptr->energy_use = 50;

	/* Get the lantern */
	j_ptr = &inventory[INVEN_LITE];

	/* Refuel */
	j_ptr->pval += o_ptr->pval;

	/* Message */
	msg_print("You fuel your lamp.");

	/* Comment */
	if (j_ptr->pval >= FUEL_LAMP)
	{
		j_ptr->pval = FUEL_LAMP;
		msg_print("Your lamp is full.");
	}

	/* Use fuel from a lantern */
	if (o_ptr->sval == SV_LITE_LANTERN)
	{
		/* No more fuel */
		o_ptr->pval = 0;

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN);
	}

	/* Decrease the item (from the pack) */
	else if (item >= 0)
	{
		inven_item_increase(item, -1);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Decrease the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP);
}



/*
 * An "item_tester_hook" for refilling torches
 */
static bool item_tester_refill_torch(object_type *o_ptr)
{
	/* Torches are okay */
	if ((o_ptr->tval == TV_LITE) &&
	    (o_ptr->sval == SV_LITE_TORCH)) return (TRUE);

	/* Assume not okay */
	return (FALSE);
}


/*
 * Refuel the players torch (from the pack or floor)
 */
static void do_cmd_refill_torch(void)
{
	int item;

	object_type *o_ptr;
	object_type *j_ptr;

	cptr q, s;


	/* Restrict the choices */
	item_tester_hook = item_tester_refill_torch;

	/* Get an item */
	q = "Refuel with which torch? ";
	s = "You have no extra torches.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Take a partial turn */
	p_ptr->energy_use = 50;

	/* Get the primary torch */
	j_ptr = &inventory[INVEN_LITE];

	/* Refuel */
	j_ptr->pval += o_ptr->pval + 5;

	/* Message */
	msg_print("You combine the torches.");

	/* Over-fuel message */
	if (j_ptr->pval >= FUEL_TORCH)
	{
		j_ptr->pval = FUEL_TORCH;
		msg_print("Your torch is fully fueled.");
	}

	/* Refuel message */
	else
	{
		msg_print("Your torch glows more brightly.");
	}

	/* Decrease the item (from the pack) */
	if (item >= 0)
	{
		inven_item_increase(item, -1);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Decrease the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP);
}




/*
 * Refill the players lamp, or restock his torches
 */
void do_cmd_refill(void)
{
	object_type *o_ptr;

	/* Get the light */
	o_ptr = &inventory[INVEN_LITE];

	/* It is nothing */
	if (o_ptr->tval != TV_LITE)
	{
		msg_print("You are not wielding a light.");
	}

	/* It's a lamp */
	else if (o_ptr->sval == SV_LITE_LANTERN)
	{
		do_cmd_refill_lamp();
	}

	/* It's a torch */
	else if (o_ptr->sval == SV_LITE_TORCH)
	{
		do_cmd_refill_torch();
	}

	/* No torch to refill */
	else
	{
		msg_print("Your light cannot be refilled.");
	}
}






/*
 * Target command
 */
void do_cmd_target(void)
{
	/* Target set */
	if (target_set_interactive(TARGET_KILL))
	{
		msg_print("Target Selected.");
	}

	/* Target aborted */
	else
	{
		msg_print("Target Aborted.");
	}
}



/*
 * Look command
 */
void do_cmd_look(void)
{
	/* Look around */
	if (target_set_interactive(TARGET_LOOK))
	{
		msg_print("Target Selected.");
	}
}



/*
 * Allow the player to examine other sectors on the map
 */
void do_cmd_locate(void)
{
	int dir, y1, x1, y2, x2;

	char tmp_val[80];

	char out_val[160];


	/* Start at current panel */
	y2 = y1 = p_ptr->wy;
	x2 = x1 = p_ptr->wx;

	/* Show panels until done */
	while (1)
	{
		/* Describe the location */
		if ((y2 == y1) && (x2 == x1))
		{
			tmp_val[0] = '\0';
		}
		else
		{
			sprintf(tmp_val, "%s%s of",
			        ((y2 < y1) ? " North" : (y2 > y1) ? " South" : ""),
			        ((x2 < x1) ? " West" : (x2 > x1) ? " East" : ""));
		}

		/* Prepare to ask which way to look */
		sprintf(out_val,
		        "Map sector [%d,%d], which is%s your sector.  Direction?",
		        (y2 / PANEL_HGT), (x2 / PANEL_WID), tmp_val);

		/* More detail */
		if (center_player)
		{
			sprintf(out_val,
		        	"Map sector [%d(%02d),%d(%02d)], which is%s your sector.  Direction?",
		        	(y2 / PANEL_HGT), (y2 % PANEL_HGT),
		        	(x2 / PANEL_WID), (x2 % PANEL_WID), tmp_val);
		}

		/* Assume no direction */
		dir = 0;

		/* Get a direction */
		while (!dir)
		{
			char command;

			/* Get a command (or Cancel) */
			if (!get_com(out_val, &command)) break;

			/* Extract direction */
			dir = target_dir(command);

			/* Error */
			if (!dir) bell("Illegal direction for locate!");
		}

		/* No direction */
		if (!dir) break;

		/* Apply the motion */
		y2 += (ddy[dir] * PANEL_HGT);
		x2 += (ddx[dir] * PANEL_WID);

		/* Verify the row */
		if (y2 < 0) y2 = 0;
		if (y2 > DUNGEON_HGT - SCREEN_HGT) y2 = DUNGEON_HGT - SCREEN_HGT;

		/* Verify the col */
		if (x2 < 0) x2 = 0;
		if (x2 > DUNGEON_WID - SCREEN_WID) x2 = DUNGEON_WID - SCREEN_WID;

		/* Handle "changes" */
		if ((p_ptr->wy != y2) || (p_ptr->wx != x2))
		{
			/* Update panel */
			p_ptr->wy = y2;
			p_ptr->wx = x2;

			/* Redraw map */
			p_ptr->redraw |= (PR_MAP);

			/* Window stuff */
			p_ptr->window |= (PW_OVERHEAD);

			/* Handle stuff */
			handle_stuff();
		}
	}

	/* Verify panel */
	p_ptr->update |= (PU_PANEL);

	/* Handle stuff */
	handle_stuff();
}






/*
 * The table of "symbol info" -- each entry is a string of the form
 * "X:desc" where "X" is the trigger, and "desc" is the "info".
 */
static cptr ident_info[] =
{
	" :A dark grid",
	"!:A potion (or oil)",
	"\":An amulet (or necklace)",
	"#:A wall (or secret door)",
	"$:Treasure (gold or gems)",
	"%:A vein (magma or quartz)",
	/* "&:unused", */
	"':An open door",
	"(:Soft armor",
	"):A shield",
	"*:A vein with treasure",
	"+:A closed door",
	",:Food (or mushroom patch)",
	"-:A wand (or rod)",
	".:Floor",
	"/:A polearm (Axe/Pike/etc)",
	/* "0:unused", */
	"1:Entrance to General Store",
	"2:Entrance to Armory",
	"3:Entrance to Weaponsmith",
	"4:Entrance to Temple",
	"5:Entrance to Alchemy shop",
	"6:Entrance to Magic store",
	"7:Entrance to Black Market",
	"8:Entrance to your home",
	/* "9:unused", */
	"::Rubble",
	";:A glyph of warding",
	"<:An up staircase",
	"=:A ring",
	">:A down staircase",
	"?:A scroll",
	"@:You",
	"A:Angel",
	"B:Bird",
	"C:Canine",
	"D:Ancient Dragon/Wyrm",
	"E:Elemental",
	"F:Dragon Fly",
	"G:Ghost",
	"H:Hybrid",
	"I:Insect",
	"J:Snake",
	"K:Killer Beetle",
	"L:Lich",
	"M:Multi-Headed Reptile",
	/* "N:unused", */
	"O:Ogre",
	"P:Giant Humanoid",
	"Q:Quylthulg (Pulsing Flesh Mound)",
	"R:Reptile/Amphibian",
	"S:Spider/Scorpion/Tick",
	"T:Troll",
	"U:Major Demon",
	"V:Vampire",
	"W:Wight/Wraith/etc",
	"X:Xorn/Xaren/etc",
	"Y:Yeti",
	"Z:Zephyr Hound",
	"[:Hard armor",
	"\\:A hafted weapon (mace/whip/etc)",
	"]:Misc. armor",
	"^:A trap",
	"_:A staff",
	/* "`:unused", */
	"a:Ant",
	"b:Bat",
	"c:Centipede",
	"d:Dragon",
	"e:Floating Eye",
	"f:Feline",
	"g:Golem",
	"h:Hobbit/Elf/Dwarf",
	"i:Icky Thing",
	"j:Jelly",
	"k:Kobold",
	"l:Louse",
	"m:Mold",
	"n:Naga",
	"o:Orc",
	"p:Person/Human",
	"q:Quadruped",
	"r:Rodent",
	"s:Skeleton",
	"t:Townsperson",
	"u:Minor Demon",
	"v:Vortex",
	"w:Worm/Worm-Mass",
	/* "x:unused", */
	"y:Yeek",
	"z:Zombie/Mummy",
	"{:A missile (arrow/bolt/shot)",
	"|:An edged weapon (sword/dagger/etc)",
	"}:A launcher (bow/crossbow/sling)",
	"~:A tool (or miscellaneous item)",
	NULL
};



/*
 * Sorting hook -- Comp function -- see below
 *
 * We use "u" to point to array of monster indexes,
 * and "v" to select the type of sorting to perform on "u".
 */
bool ang_sort_comp_hook(vptr u, vptr v, int a, int b)
{
	u16b *who = (u16b*)(u);

	u16b *why = (u16b*)(v);

	int w1 = who[a];
	int w2 = who[b];

	int z1, z2;


	/* Sort by player kills */
	if (*why >= 4)
	{
		/* Extract player kills */
		z1 = l_list[w1].r_pkills;
		z2 = l_list[w2].r_pkills;

		/* Compare player kills */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}


	/* Sort by total kills */
	if (*why >= 3)
	{
		/* Extract total kills */
		z1 = l_list[w1].r_tkills;
		z2 = l_list[w2].r_tkills;

		/* Compare total kills */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}


	/* Sort by monster level */
	if (*why >= 2)
	{
		/* Extract levels */
		z1 = r_info[w1].level;
		z2 = r_info[w2].level;

		/* Compare levels */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}


	/* Sort by monster experience */
	if (*why >= 1)
	{
		/* Extract experience */
		z1 = r_info[w1].mexp;
		z2 = r_info[w2].mexp;

		/* Compare experience */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}


	/* Compare indexes */
	return (w1 <= w2);
}


/*
 * Sorting hook -- Swap function -- see below
 *
 * We use "u" to point to array of monster indexes,
 * and "v" to select the type of sorting to perform.
 */
void ang_sort_swap_hook(vptr u, vptr v, int a, int b)
{
	u16b *who = (u16b*)(u);

	u16b holder;

	/* Swap */
	holder = who[a];
	who[a] = who[b];
	who[b] = holder;
}



/*
 * Hack -- Display the "name" and "attr/chars" of a monster race
 */
static void roff_top(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	byte a1, a2;
	char c1, c2;


	/* Get the chars */
	c1 = r_ptr->d_char;
	c2 = r_ptr->x_char;

	/* Get the attrs */
	a1 = r_ptr->d_attr;
	a2 = r_ptr->x_attr;


	/* Clear the top line */
	Term_erase(0, 0, 255);

	/* Reset the cursor */
	Term_gotoxy(0, 0);

	/* A title (use "The" for non-uniques) */
	if (!(r_ptr->flags1 & (RF1_UNIQUE)))
	{
		Term_addstr(-1, TERM_WHITE, "The ");
	}

	/* Dump the name */
	Term_addstr(-1, TERM_WHITE, (r_name + r_ptr->name));

	/* Append the "standard" attr/char info */
	Term_addstr(-1, TERM_WHITE, " ('");
	Term_addch(a1, c1);
	Term_addstr(-1, TERM_WHITE, "')");

	/* Append the "optional" attr/char info */
	Term_addstr(-1, TERM_WHITE, "/('");
	Term_addch(a2, c2);
	Term_addstr(-1, TERM_WHITE, "'):");
}


/*
 * Identify a character, allow recall of monsters
 *
 * Several "special" responses recall "mulitple" monsters:
 *   ^A (all monsters)
 *   ^U (all unique monsters)
 *   ^N (all non-unique monsters)
 *
 * The responses may be sorted in several ways, see below.
 *
 * Note that the player ghosts are ignored, since they do not exist.
 */
void do_cmd_query_symbol(void)
{
	int i, n, r_idx;
	char sym, query;
	char buf[128];

	bool all = FALSE;
	bool uniq = FALSE;
	bool norm = FALSE;

	bool recall = FALSE;

	u16b why = 0;
	u16b *who;


	/* Get a character, or abort */
	if (!get_com("Enter character to be identified: ", &sym)) return;

	/* Find that character info, and describe it */
	for (i = 0; ident_info[i]; ++i)
	{
		if (sym == ident_info[i][0]) break;
	}

	/* Describe */
	if (sym == KTRL('A'))
	{
		all = TRUE;
		strcpy(buf, "Full monster list.");
	}
	else if (sym == KTRL('U'))
	{
		all = uniq = TRUE;
		strcpy(buf, "Unique monster list.");
	}
	else if (sym == KTRL('N'))
	{
		all = norm = TRUE;
		strcpy(buf, "Non-unique monster list.");
	}
	else if (ident_info[i])
	{
		sprintf(buf, "%c - %s.", sym, ident_info[i] + 2);
	}
	else
	{
		sprintf(buf, "%c - %s.", sym, "Unknown Symbol");
	}

	/* Display the result */
	prt(buf, 0, 0);

	/* Allocate the "who" array */
	C_MAKE(who, z_info->r_max, u16b);

	/* Collect matching monsters */
	for (n = 0, i = 1; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];

		monster_lore *l_ptr = &l_list[i];

		/* Nothing to recall */
		if (!cheat_know && !l_ptr->r_sights) continue;

		/* Require non-unique monsters if needed */
		if (norm && (r_ptr->flags1 & (RF1_UNIQUE))) continue;

		/* Require unique monsters if needed */
		if (uniq && !(r_ptr->flags1 & (RF1_UNIQUE))) continue;

		/* Collect "appropriate" monsters */
		if (all || (r_ptr->d_char == sym)) who[n++] = i;
	}

	/* Nothing to recall */
	if (!n) return;


	/* Prompt */
	put_str("Recall details? (k/p/y/n): ", 0, 40);

	/* Query */
	query = inkey();

	/* Restore */
	prt(buf, 0, 0);


	/* Sort by kills (and level) */
	if (query == 'k')
	{
		why = 4;
		query = 'y';
	}

	/* Sort by level */
	if (query == 'p')
	{
		why = 2;
		query = 'y';
	}

	/* Catch "escape" */
	if (query != 'y') return;


	/* Sort if needed */
	if (why)
	{
		/* Select the sort method */
		ang_sort_comp = ang_sort_comp_hook;
		ang_sort_swap = ang_sort_swap_hook;

		/* Sort the array */
		ang_sort(who, &why, n);
	}


	/* Start at the end */
	i = n - 1;

	/* Scan the monster memory */
	while (1)
	{
		/* Extract a race */
		r_idx = who[i];

		/* Hack -- Auto-recall */
		monster_race_track(r_idx);

		/* Hack -- Handle stuff */
		handle_stuff();

		/* Hack -- Begin the prompt */
		roff_top(r_idx);

		/* Hack -- Complete the prompt */
		Term_addstr(-1, TERM_WHITE, " [(r)ecall, ESC]");

		/* Interact */
		while (1)
		{
			/* Recall */
			if (recall)
			{
				/* Save screen */
				screen_save();

				/* Recall on screen */
				screen_roff(who[i]);

				/* Hack -- Complete the prompt (again) */
				Term_addstr(-1, TERM_WHITE, " [(r)ecall, ESC]");
			}

			/* Command */
			query = inkey();

			/* Unrecall */
			if (recall)
			{
				/* Load screen */
				screen_load();
			}

			/* Normal commands */
			if (query != 'r') break;

			/* Toggle recall */
			recall = !recall;
		}

		/* Stop scanning */
		if (query == ESCAPE) break;

		/* Move to "prev" monster */
		if (query == '-')
		{
			if (++i == n)
			{
				i = 0;
				if (!expand_list) break;
			}
		}

		/* Move to "next" monster */
		else
		{
			if (i-- == 0)
			{
				i = n - 1;
				if (!expand_list) break;
			}
		}
	}


        return;
}



/*
 * RVIP item menus (the Zangband web port's inven_screen(), ported).
 *
 * 'i' / 'e' show the list with a cursor.  A letter runs the item's main
 * action, Shift+letter drops, Ctrl+letter observes; 2/8 or the arrows move
 * the cursor, Enter/Space/5 open the item's action menu, + - * run
 * main/drop/observe on the cursor item, 4/6 or '/' switch lists,
 * Escape/0/. close; any other key is a normal command (as before).
 *
 * An action runs through its own command (key queue + preselect): the key
 * goes into p_ptr->command_new (past the keymaps) and get_item() takes the
 * preselected item if that command accepts it, else it asks as usual.
 */
typedef struct inv_action inv_action;
struct inv_action
{
	char key;		/* underlying command */
	cptr name;
	byte where;		/* 1 pack, 2 equipment, 3 both */
	bool (*ok)(const object_type *o_ptr);
};

static bool inv_ok_any(const object_type *o_ptr) { (void)o_ptr; return (TRUE); }
static bool inv_ok_food(const object_type *o_ptr) { return (o_ptr->tval == TV_FOOD); }
static bool inv_ok_potion(const object_type *o_ptr) { return (o_ptr->tval == TV_POTION); }
static bool inv_ok_scroll(const object_type *o_ptr) { return (o_ptr->tval == TV_SCROLL); }
static bool inv_ok_wand(const object_type *o_ptr) { return (o_ptr->tval == TV_WAND); }
static bool inv_ok_staff(const object_type *o_ptr) { return (o_ptr->tval == TV_STAFF); }
static bool inv_ok_rod(const object_type *o_ptr) { return (o_ptr->tval == TV_ROD); }
static bool inv_ok_fuel(const object_type *o_ptr)
{
	return ((o_ptr->tval == TV_FLASK) || (o_ptr->tval == TV_LITE));
}
static bool inv_ok_wear(const object_type *o_ptr)
{
	return (wield_slot((object_type *)o_ptr) >= INVEN_WIELD);
}
static bool inv_ok_activate(const object_type *o_ptr)
{
	u32b f1, f2, f3;

	if (!object_known_p(o_ptr)) return (FALSE);
	object_flags((object_type *)o_ptr, &f1, &f2, &f3);
	return ((f3 & (TR3_ACTIVATE)) ? TRUE : FALSE);
}
static bool inv_ok_ammo(const object_type *o_ptr)
{
	return (p_ptr->ammo_tval && (o_ptr->tval == p_ptr->ammo_tval));
}
static bool inv_ok_book(const object_type *o_ptr)
{
	return (mp_ptr->spell_book && (o_ptr->tval == mp_ptr->spell_book));
}
static bool inv_ok_note(const object_type *o_ptr) { return (o_ptr->note ? TRUE : FALSE); }

/* In this order: the first that fits is the item's main action */
static const inv_action inv_act[] =
{
	{ 'E', "Eat", 1, inv_ok_food },
	{ 'q', "Quaff", 1, inv_ok_potion },
	{ 'r', "Read", 1, inv_ok_scroll },
	{ 'a', "Aim", 1, inv_ok_wand },
	{ 'u', "Use", 1, inv_ok_staff },
	{ 'z', "Zap", 1, inv_ok_rod },
	{ 'A', "Activate", 2, inv_ok_activate },
	{ 'f', "Fire", 1, inv_ok_ammo },
	{ 'm', "Cast from", 1, inv_ok_book },
	{ 'b', "Browse", 1, inv_ok_book },
	{ 'w', "Wear/Wield", 1, inv_ok_wear },
	{ 't', "Take off", 2, inv_ok_any },
	{ 'F', "Refuel with", 1, inv_ok_fuel },
	{ 'v', "Throw", 1, inv_ok_any },
	{ 'd', "Drop", 3, inv_ok_any },
	{ 'k', "Destroy", 1, inv_ok_any },
	{ 'I', "Observe", 3, inv_ok_any },
	{ '{', "Inscribe", 3, inv_ok_any },
	{ '}', "Uninscribe", 3, inv_ok_note },
	{ 0, NULL, 0, NULL }
};

/* Does action i fit the item in slot "item"? */
static bool inv_fits(int i, int item)
{
	object_type *o_ptr = &inventory[item];
	int where = (item >= INVEN_WIELD) ? 2 : 1;

	if (!o_ptr->k_idx) return (FALSE);
	if (!(inv_act[i].where & where)) return (FALSE);

	/* Priests pray, others cast */
	if ((inv_act[i].key == 'm') && (mp_ptr->spell_book == TV_PRAYER_BOOK))
		return (FALSE);
	if (inv_act[i].key == 'm') return (inv_ok_book(o_ptr));

	return ((*inv_act[i].ok)(o_ptr));
}

static char inv_key(int i)
{
	/* Prayers are 'p' */
	if ((inv_act[i].key == 'm') && (mp_ptr->spell_book == TV_PRAYER_BOOK)) return ('p');
	return (inv_act[i].key);
}

/* Queue action i on slot "item", reopen the list afterwards */
static void inv_run(int i, int item, bool equip)
{
	get_item_preselect = item;
	p_ptr->command_new = inv_key(i);
	command_new_raw = TRUE;
	inven_reopen = equip ? 'e' : 'i';
}

static int inv_find(char key)
{
	int i;

	for (i = 0; inv_act[i].key; i++) if (inv_act[i].key == key) return (i);
	return (-1);
}

/* Main action: the first that fits */
static int inv_main(int item)
{
	int i;

	for (i = 0; inv_act[i].key; i++) if (inv_fits(i, item)) return (i);
	return (-1);
}

/* The list is reopened after an action unless a monster is in view */
bool inven_may_reopen(void)
{
	int i;

	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		if (!m_ptr->r_idx || !m_ptr->ml) continue;
		if (r_info[m_ptr->r_idx].flags1 & (RF1_NEVER_MOVE)) continue;
		if (player_has_los_bold(m_ptr->fy, m_ptr->fx)) return (FALSE);
	}
	return (TRUE);
}

/* Action menu for one item, boxed over its line; returns the action or -1 */
static int inv_action_menu(int item, int row)
{
	int list[32], n = 0, i, cur = 0, w = 20;
	int x = show_list_col + 6, y = row;

	for (i = 0; inv_act[i].key; i++)
		if (inv_fits(i, item)) list[n++] = i;
	if (!n) return (-1);

	if (y + n + 2 > Term->hgt) y = Term->hgt - n - 2;
	if (x + w > Term->wid) x = Term->wid - w;

	while (1)
	{
		char ch, line[32];

		for (i = 0; i < w; i++) line[i] = (i == 0 || i == w - 1) ? '+' : '-';
		line[w] = 0;
		Term_putstr(x, y, w, TERM_L_BLUE, line);
		Term_putstr(x, y + n + 1, w, TERM_L_BLUE, line);
		for (i = 0; i < n; i++)
		{
			byte a = (i == cur) ? TERM_L_GREEN : TERM_WHITE;
			char kb[4];

			sprintf(kb, "%c", inv_key(list[i]));
			Term_putstr(x, y + 1 + i, w, TERM_L_BLUE, "|                  |");
			Term_putstr(x + 1, y + 1 + i, 1, a, (i == cur) ? ">" : " ");
			Term_putstr(x + 3, y + 1 + i, -1, TERM_L_BLUE, kb);
			Term_putstr(x + 6, y + 1 + i, -1, a, inv_act[list[i]].name);
		}
		Term_gotoxy(x + 1, y + 1 + cur);

		inkey_base = TRUE;
		ch = inkey();

		/* Arrow keys arrive as X11 keysym triggers (macros are off here) */
		if (ch == 31)
		{
			char buf[32];
			int k = 0;

			while (k < 31)
			{
				inkey_base = TRUE;
				ch = inkey();
				if ((ch == '\r') || !ch) break;
				buf[k++] = ch;
			}
			buf[k] = 0;
			ch = 0;
			if (strstr(buf, "FF52") || strstr(buf, "FFB8")) ch = '8';
			if (strstr(buf, "FF54") || strstr(buf, "FFB2")) ch = '2';
			if (strstr(buf, "FF51") || strstr(buf, "FFB4")) ch = '4';
			if (strstr(buf, "FF53") || strstr(buf, "FFB6")) ch = '6';
		}

		if ((ch == ESCAPE) || (ch == '4') || (ch == '0')) return (-1);
		if (ch == '8') { cur = (cur + n - 1) % n; continue; }
		if (ch == '2') { cur = (cur + 1) % n; continue; }
		if ((ch == '\r') || (ch == '\n') || (ch == ' ') || (ch == '5') || (ch == '6'))
			return (list[cur]);
		for (i = 0; i < n; i++)
			if (ch && (inv_key(list[i]) == ch)) return (list[i]);
	}
}

void inven_screen(bool equip)
{
	int cur = 0;

	screen_save();

	while (1)
	{
		int n, item, i;
		char ch;

		/* Rows of the list */
		item_tester_full = TRUE;
		if (equip)
		{
			show_equip();
			n = INVEN_TOTAL - INVEN_WIELD;
		}
		else
		{
			show_inven();
			for (n = 0, i = 0; i < INVEN_PACK; i++) if (inventory[i].k_idx) n = i + 1;
			if (!n) n = 1;
		}
		item_tester_full = FALSE;

		if (cur >= n) cur = n - 1;
		if (cur < 0) cur = 0;
		item = equip ? INVEN_WIELD + cur : cur;

		/* Cursor left of the list */
		Term_putstr(show_list_col >= 2 ? show_list_col - 2 : 0, cur + 1, 1, TERM_L_GREEN, ">");

		prt(equip ? "(Equipment) Letter: use, Shift: drop, Enter: menu, 4/6: pack, Esc" :
		            "(Inventory) Letter: use, Shift: drop, Enter: menu, 4/6: gear, Esc", 0, 0);

		inkey_base = TRUE;
		ch = inkey();

		/* Arrow keys: X11 keysym triggers (macros are off here) */
		if (ch == 31)
		{
			char buf[32];
			int k = 0;

			while (k < 31)
			{
				inkey_base = TRUE;
				ch = inkey();
				if ((ch == '\r') || !ch) break;
				buf[k++] = ch;
			}
			buf[k] = 0;
			ch = 0;
			if (strstr(buf, "FF52") || strstr(buf, "FFB8")) ch = '8';
			if (strstr(buf, "FF54") || strstr(buf, "FFB2")) ch = '2';
			if (strstr(buf, "FF51") || strstr(buf, "FFB4")) ch = '4';
			if (strstr(buf, "FF53") || strstr(buf, "FFB6")) ch = '6';
			if (strstr(buf, "FF8D")) ch = '\r';
			if (!ch) continue;
		}

		screen_load();
		screen_save();

		if ((ch == ESCAPE) || (ch == '0') || (ch == '.')) break;
		if (ch == '8') { cur = (cur + n - 1) % n; continue; }
		if (ch == '2') { cur = (cur + 1) % n; continue; }
		if ((ch == '4') || (ch == '6') || (ch == '/')) { equip = !equip; cur = 0; continue; }

		/* Action menu on the cursor item */
		if ((ch == '\r') || (ch == '\n') || (ch == ' ') || (ch == '5'))
		{
			int a;

			if (!inventory[item].k_idx) continue;

			/* Draw the list again under the box */
			item_tester_full = TRUE;
			if (equip) show_equip(); else show_inven();
			item_tester_full = FALSE;

			a = inv_action_menu(item, cur + 1);
			if (a < 0) continue;
			inv_run(a, item, equip);
			break;
		}

		/* Main / drop / observe on the cursor item */
		if ((ch == '+') || (ch == '-') || (ch == '*'))
		{
			int a = (ch == '+') ? inv_main(item) : inv_find((ch == '-') ? 'd' : 'I');

			if ((a < 0) || !inv_fits(a, item)) { bell("You cannot do that."); continue; }
			inv_run(a, item, equip);
			break;
		}

		/* Letter: main action; Shift: drop; Ctrl: observe */
		{
			int it = -1, a = -1;

			if ((ch >= 'a') && (ch <= 'z'))
			{
				it = equip ? label_to_equip(ch) : label_to_inven(ch);
				if (it >= 0) a = inv_main(it);
			}
			else if ((ch >= 'A') && (ch <= 'Z'))
			{
				it = equip ? label_to_equip(ch - 'A' + 'a') : label_to_inven(ch - 'A' + 'a');
				a = inv_find('d');
			}
			else if ((ch >= 1) && (ch <= 26) && (ch != '\r') && (ch != '\n') && (ch != '\t') && (ch != 8))
			{
				it = equip ? label_to_equip(ch - 1 + 'a') : label_to_inven(ch - 1 + 'a');
				a = inv_find('I');
			}

			if ((it >= 0) && (a >= 0) && inv_fits(a, it))
			{
				inv_run(a, it, equip);
				break;
			}

			/* Anything else: a normal command, as the original list did */
			if (it < 0)
			{
				p_ptr->command_new = ch;
				p_ptr->command_see = TRUE;
				p_ptr->command_wrk = equip ? (USE_EQUIP) : (USE_INVEN);
				break;
			}

			bell("You cannot do that.");
		}
	}

	screen_load();
}
