/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
SDName: Example_Gossip_Codebox
SD%Complete: 100
SDComment: Show a codebox in gossip option
SDCategory: Script Examples
EndScriptData */

#include "ScriptPCH.h"
#include <cstring>

enum eEnums
{
    SPELL_POLYMORPH         = 12826,
    SPELL_MARK_OF_THE_WILD  = 26990,

    SAY_NOT_INTERESTED      = -1999922,
    SAY_WRONG               = -1999923,
    SAY_CORRECT             = -1999924
};

#define GOSSIP_ITEM_1       "A quiz: what's your name?"
#define GOSSIP_ITEM_2       "I'm not interested"

class example_gossip_codebox : public CreatureScript
{
    public:

        example_gossip_codebox()
            : CreatureScript("example_gossip_codebox")
        {
        }

        bool OnGossipHello(Player* player, Creature* creature)
        {
            player->ADD_GOSSIP_ITEM_EXTENDED(0, GOSSIP_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1, "", 0, true);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

            player->PlayerTalkClass->SendGossipMenu(907, creature->GetGUID());

            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
        {
            player->PlayerTalkClass->ClearMenus();
            if (action == GOSSIP_ACTION_INFO_DEF+2)
            {
                DoScriptText(SAY_NOT_INTERESTED, creature);
                player->CLOSE_GOSSIP_MENU();
            }

            return true;
        }

        bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code)
        {
            player->PlayerTalkClass->ClearMenus();
            if (sender == GOSSIP_SENDER_MAIN)
            {
                switch (action)
                {
                case GOSSIP_ACTION_INFO_DEF+1:
                    if (std::strcmp(code, player->GetName()) != 0)
                    {
                        DoScriptText(SAY_WRONG, creature);
                        creature->CastSpell(player, SPELL_POLYMORPH, true);
                    }
                    else
                    {
                        DoScriptText(SAY_CORRECT, creature);
                        creature->CastSpell(player, SPELL_MARK_OF_THE_WILD, true);
                    }
                    player->CLOSE_GOSSIP_MENU();

                    return true;
                }
            }

            return false;
        }
};

void AddSC_example_gossip_codebox()
{
    new example_gossip_codebox();
}
